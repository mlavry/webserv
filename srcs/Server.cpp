/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mlavry <mlavry@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/17 13:22:39 by mlavry            #+#    #+#             */
/*   Updated: 2026/05/28 14:15:05 by mlavry           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#define RESET   "\033[0m"
#define BOLD    "\033[1m"
#define DIM     "\033[2m"

#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define WHITE   "\033[37m"

#define HEADER_TIMEOUT 30
#define BODY_TIMEOUT 30
#define SEND_TIMEOUT 30

#include "Server.hpp"

#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <arpa/inet.h>
#include <cerrno>
#include <csignal>
#include <ctime>
#include <sstream>
#include <iomanip>

extern volatile sig_atomic_t g_running;

Server::Server()
{
}
		
Server::~Server()
{
	closeAllFds();
}

TimeoutConfig::TimeoutConfig()
{
	headerTimeout = HEADER_TIMEOUT;
	bodyTimeout = BODY_TIMEOUT;
	sendTimeout = SEND_TIMEOUT;
}

void Server::setConfig(std::vector<ServerConfig> const &list)
{
	_configs = list;
}

bool Server::setSocketOption(int fd, int option)
{
	int opt = 1;
	if (setsockopt(fd, SOL_SOCKET, option, &opt, sizeof(opt)) < 0)
		return (false);
	return (true);
}

void Server::closeAllFds()
{
	for (size_t i = 0; i < _fds.size(); i++)
	{
		if (_fds[i].fd >= 0)
			close (_fds[i].fd);
	}
	
	_fds.clear();
	_clients.clear();
	_listenFdToServers.clear();
}

std::string Server::getTime() const
{
	std::time_t now;
	char buffer[64];
	
	now = std::time(NULL);
	std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", 
		std::localtime(&now));
	return (std::string(buffer));
}

double Server::getResponseTime(const Client& client) const
{
	if (!client.hasStartTime)
		return (0.0);
	
	return (std::difftime(std::time(NULL), client.startTime));
}

std::string Server::methodColor(const std::string& method) const
{
	if (method == "GET")
		return (GREEN);
	else if (method == "POST")
		return (YELLOW);
	else if (method == "DELETE")
		return (RED);
	else if (method == "HEAD")
		return (CYAN);
	return (WHITE);
}

std::string Server::statusColor(int status) const
{
	if (status >= 200 && status < 300)
		return (GREEN);
	if (status >= 300 && status < 400)
		return (CYAN);
	if (status >= 400 && status < 500)
		return (YELLOW);
	if (status >= 500)
		return (RED);
	return (WHITE);
}

std::string Server::truncString(const std::string& str, size_t max) const
{
	if (str.size() <= max)
		return (str);
	if (max <= 3)
		return (str.substr(0, max));
	return (str.substr(0, max - 3) + "...");
}

void Server::printLog(const Client& client) const
{
	double response_time = getResponseTime(client);
	std::string method = truncString(client.request.method, 6);
	std::string path = truncString(client.request.path, 16);

	std::cout
		<< BOLD << BLUE << "● " << RESET
		<< DIM << getTime() << RESET
		<< " │ "
		<< CYAN << std::left << std::setw(15)
		<< client.ip << RESET
		<< " │ "
		<< DIM << "fd:" << std::left << std::setw(4)
		<< client.fd << RESET
		<< " │ "
		<< methodColor(client.request.method)
		<< BOLD << std::left << std::setw(6)
		<< method << RESET
		<< " "
		<< std::left << std::setw(24)
		<< path
		<< " │ "
		<< statusColor(client.statusCode)
		<< BOLD << std::setw(3)
		<< client.statusCode << RESET
		<< " │ "
		<< std::right << std::setw(5) << GREEN
		<< client.response.size() << "B" << RESET
		<< " │ "
		<< std::fixed << std::setprecision(2)
		<< response_time << "s"
		<< std::endl;
}

std::string Server::makeListenKey(const std::string& host, int port)
{
	std::ostringstream oss;

	oss << host << ":" << port;
	return (oss.str());
}

bool Server::initServer()
{
	for (size_t i = 0; i < _configs.size(); i++)
	{
		for (size_t j = 0; j < _configs[i].listens.size(); j++)
		{
			std::string host = _configs[i].listens[j].host;
			int port = _configs[i].listens[j].port;
			std::string key = makeListenKey(host, port);

			if (_listenKeyToFd.find(key) != _listenKeyToFd.end())
			{
				int existingFd = _listenKeyToFd[key];

				_listenFdToServers[existingFd].push_back(&_configs[i]);
				continue;
			}
			
			//AF_INET IPv4
			//SOCK_STREAM connexion TCP
			int serverFd = socket(AF_INET, SOCK_STREAM, 0);
			if (serverFd < 0)
			{
				std::cerr << "Erreur socket" << std::endl;
				return (false);
			}
			
			// On permet au serveur de se relancer si le port est en cours de fermeture
			if (!setSocketOption(serverFd, SO_REUSEADDR))
			{
				std::cerr << "Erreur setsockopt" << std::endl;
				close(serverFd);
				serverFd = -1;
				return (false);
			}

			sockaddr_in address;
			memset(&address, 0, sizeof(address));
			
			address.sin_family = AF_INET;
			address.sin_port = htons(_configs[i].listens[j].port);
			//_serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");//htonl(INADDR_ANY);
			address.sin_addr.s_addr = inet_addr(_configs[i].listens[j].host.c_str());
			
			// On attache le socket a une adresse 
			std::cout << "Trying bind: "
          		<< _configs[i].listens[j].host << ":"
          		<< _configs[i].listens[j].port
          		<< std::endl;
			if (bind(serverFd, (struct sockaddr *)&address, 
				sizeof(address)) < 0)
			{
				std::cerr << "Erreur bind on "
          			<< _configs[i].listens[j].host << ":"
          			<< _configs[i].listens[j].port
          			<< " -> " << strerror(errno) << std::endl;
				close(serverFd);
				serverFd = -1;
				return (false);
			}
			
			// Le socket passe en mode serveur (écoute) et accepete des connexions
			if (listen(serverFd, SOMAXCONN) < 0)
			{
				std::cerr << "Erreur listen" << std::endl;
				close(serverFd);
				serverFd = -1;
				return (false);
			}
	
			// On récupére les flags et on passe et on ajoute le non bloquant
			int flags = fcntl(serverFd, F_GETFL, 0);
			if (flags < 0 || fcntl(serverFd, F_SETFL, flags | O_NONBLOCK) < 0)
			{
				std::cerr << "Erreur fcntl" << std::endl;
				close(serverFd);
				serverFd = -1;
				return (false);
			}
			_listenKeyToFd[key] = serverFd;
			_listenFdToServers[serverFd].push_back(&_configs[i]);
			initPoll(serverFd);
		}
	}
	return (true);
}

void Server::removeClient(int i)
{
	close(_fds[i].fd);
	_clients.erase(_fds[i].fd);
	_fds.erase(_fds.begin() + i);
}

void Server::initPoll(int fd)
{
	pollfd server_poll_fd;
	server_poll_fd.fd = fd;
	server_poll_fd.events = POLLIN;
	server_poll_fd.revents = 0;
	_fds.push_back(server_poll_fd);
}

bool Server::checkPoll()
{
	if (_fds.empty())
		return (false);

	int ret = poll(&_fds[0], _fds.size(), 1000); // 1000 (every second server check activity)
	if (ret < 0)
	{
		if (errno == EINTR) // Need utility check (remettre errno a 0 apres ?)
		{
			std::cout << "\n" GREEN "Arret du serveur..." RESET << std::endl;
			return (true);
		} // if block no util comment
		std::cerr << "Erreur poll" << std::endl;
		return (false);
	}
	return (true);
}

std::string Server::ipToString(unsigned int ip) const
{
	std::ostringstream oss;
	
	ip = ntohl(ip); // Network to host long 

	oss << ((ip >> 24) & 0xFF) << "."
		<< ((ip >> 16) & 0xFF) << "."
		<< ((ip >> 8) & 0xFF) << "."
		<< (ip & 0xFF);
	
	return (oss.str());
}

int Server::getTimeoutClient(const Client& client, short events) const
{
	RequestState state = client.parser.get_status();

	if (events & POLLOUT)
		return (_timeoutConfig.sendTimeout);
		
	if (state == READING_HEADER || state == READING_REQUEST)
		return (_timeoutConfig.headerTimeout);
	
	if (state == READING_BODY || state == READING_CHUNKED)
		return (_timeoutConfig.bodyTimeout);
	
	return (_timeoutConfig.headerTimeout);
}

std::string Server::buildTimeoutResponse() const
{
	return (
		"HTTP/1.1 408 Request Timeout\r\n"
		"Content-Length: 15\r\n"
		"Content-Type: text/plain\r\n"
		"Connection: close\r\n"
		"\r\n"
		"Request Timeout"
	);
}

bool Server::handleTimeout(int i)
{
	Client &client = _clients[_fds[i].fd];

	if (_fds[i].events & POLLOUT)
	{
		removeClient(i);
		return (true);
	}
	
	client.statusCode = 408;
	client.response = buildTimeoutResponse();
	client.bytesSent = 0;
	
	if (!client.hasStartTime)
	{
		client.startTime = std::time(NULL);
		client.hasStartTime = true;
	}
	
	printLog(client);

	_fds[i].events = POLLOUT;
	client.lastActivity = std::time(NULL); // evite de timeout avant reponse
	
	return (false);
}

void Server::checkTimeouts()
{
	time_t now = std::time(NULL);
	
	for (int i = 0; i < (int)_fds.size(); i++)
	{
		int fd = _fds[i].fd;
		
		if (_listenFdToServers.find(fd) != _listenFdToServers.end())
			continue;

		if (_clients.find(fd) == _clients.end())
			continue;
		
		int timeout = getTimeoutClient(_clients[fd], _fds[i].events);
		
		if (now - _clients[fd].lastActivity >= timeout)
		{
			if (handleTimeout(i))
				i--;
		}
	}
}

void Server::acceptClient(int listenFd)
{
	sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);
	
	int client_fd = accept(listenFd, (struct sockaddr *)&client_addr,
		&client_len);
	if (client_fd < 0)
		return;
	std::cout << "Client accepted on listenFd: " << listenFd << std::endl;

	int flags = fcntl(client_fd, F_GETFL, 0);
	if (flags < 0 || fcntl(client_fd, F_SETFL, flags | O_NONBLOCK)
		< 0)
	{
		close(client_fd);
		return;
	}

	pollfd client_poll_fd;
	client_poll_fd.fd = client_fd;
	client_poll_fd.events = POLLIN;
	client_poll_fd.revents = 0;
	_fds.push_back(client_poll_fd);

	_clients[client_fd] = Client(client_fd);
	_clients[client_fd].listenFd = listenFd;
	_clients[client_fd].ip = ipToString(client_addr.sin_addr.s_addr);
	std::cout << "Client ajouté dans poll" << std::endl;
}

bool Server::handleClient(int i)
{
	Client &client = _clients[_fds[i].fd];
	RequestState state;
	char buffer[1024];

	int bytes = recv(_fds[i].fd, buffer, sizeof(buffer), 0);

	if (bytes < 0)
	{
		std::cout << "recv < 0 bytes" << std::endl;
		removeClient(i);
		return (true);
	}
	else if (bytes > 0)
	{
		//std::cout << "data received" << std::endl;
		client.lastActivity = std::time(NULL);
		if (!client.hasStartTime)
		{
			client.startTime = std::time(NULL);
			client.hasStartTime = true;
		}
		client.parser.parse_chunk(buffer, bytes, client.request);
	}
	//std::cout << client.parser.get_error_code() << std::endl;
	//std::cout << "status: " << state << std::endl;

	state = client.parser.get_status();

	//std::cout << "state: " << state
	//	<< " method: [" << client.request.method << "]"
	//	<< " path: [" << client.request.path << "]"
	//	<< " error: " << client.parser.get_error_code()
	//	<< std::endl;
	
	if (bytes == 0)
	{
		std::cout << "recv = 0 bytes" << std::endl;
		if (state != COMPLETE)
		{
			std::cout << "status != COMPLETE" << std::endl;
			removeClient(i);
			return (true);
		}
	}	
	
	if (state == ERROR)
	{
		client.response = "HTTP/1.1 400 Bad Request\r\n"
			"Content-Length: 11\r\n"
			"Connection: close\r\n"
			"\r\n"
			"Bad Request";
		client.statusCode = client.parser.get_error_code();
		client.bytesSent = 0;
		printLog(client);
		_fds[i].events = POLLOUT;
		return (false);
	}
	
	if (state != COMPLETE)
		return (false);

	client.bytesSent = 0;
	client.response =
		"HTTP/1.1 200 OK\r\n"
		"Content-Length: 5\r\n"
		"Content-Type: text/plain\r\n"
		"Connection: close\r\n"
		"\r\n"
		"Hello";
	if (client.parser.get_error_code() == 0)
		client.statusCode = 200;
	client.request.print_body();
	printLog(client);
	_fds[i].events = POLLOUT;
	return (false);
}

bool Server::sendResponse(int i)
{
	Client &client = _clients[_fds[i].fd];
	
	size_t remaining = client.response.size() - client.bytesSent;
	
	int ret = send(_fds[i].fd, client.response.c_str() + client.bytesSent, 
		remaining, 0);
	if (ret <= 0)
	{
		removeClient(i);
		return (true);
	}
	
	client.bytesSent += ret;
	client.lastActivity = std::time(NULL);
	
	if (client.bytesSent >= client.response.size())
	{
		removeClient(i);
		return (true);
	}

	return (false);
}

bool Server::handleEvents()
{
	for (int i = 0; i < (int)_fds.size(); i++)
	{
		if (_listenFdToServers.find(_fds[i].fd) != _listenFdToServers.end())
		{
			if (_fds[i].revents & (POLLERR | POLLHUP | POLLNVAL))
			{
				std::cerr << "Erreur sur le socket serveur" << std::endl;
				return (false);
			}
			if (_fds[i].revents & POLLIN)
				acceptClient(_fds[i].fd);
		}
		else
		{
			bool removed = false;
			if (_fds[i].revents & (POLLERR | POLLNVAL))
			{
				//std::cout << "Caught POLLHUP or other" << std::endl;
				removeClient(i);
				removed = true;
			}
			if (!removed && (_fds[i].revents & POLLIN))
				removed = handleClient(i);
			if (!removed && (_fds[i].revents & POLLOUT))
				removed = sendResponse(i);
			if (!removed && (_fds[i].revents & POLLHUP)
				&& !(_fds[i].events & POLLOUT))
			{
				removeClient(i);
				removed = true;
			}
			if (removed)
				i--;
		}
	}
	return (true);
}

void Server::run()
{
	std::cout << GREEN "Lancement du serveur..." RESET << std::endl;
	while (g_running)
	{
		if (!checkPoll())
			break;
		if (!handleEvents())
			break;
		checkTimeouts();
	}
	closeAllFds();
}
