/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mlavry <mlavry@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/17 13:22:39 by mlavry            #+#    #+#             */
/*   Updated: 2026/04/23 11:31:49 by mlavry           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <arpa/inet.h>

Server::Server() : _serverFd(-1)
{
	memset(&_serverAddress, 0, sizeof(_serverAddress));
}
		
Server::~Server()
{
	if (_serverFd >= 0)
		close(_serverFd);
}

bool Server::initServer()
{
	_serverFd = socket(AF_INET, SOCK_STREAM, 0);
	if (_serverFd < 0)
	{
		std::cerr << "Erreur socket" << std::endl;
		return (false);
	}

	_serverAddress.sin_family = AF_INET;
	_serverAddress.sin_port = htons(8080);
	_serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);//inet_addr("0.0.0.0");

	// On attache le socket a une adresse 
	if (bind(_serverFd, (struct sockaddr *)&_serverAddress, 
		sizeof(_serverAddress)) < 0)
	{
		std::cerr << "Erreur bind" << std::endl;
		close(_serverFd);
		_serverFd = -1;
		return (false);
	}

	// Le socket passe en mode serveur (écoute) et accepete des connexions
	if (listen(_serverFd, SOMAXCONN) < 0)
	{
		std::cerr << "Erreur listen" << std::endl;
		close(_serverFd);
		_serverFd = -1;
		return (false);
	}
	
	// On récupére les flags et on passe et on ajoute le non bloquant
	int flags = fcntl(_serverFd, F_GETFL, 0);
	if (flags < 0 || fcntl(_serverFd, F_SETFL, flags | O_NONBLOCK) < 0)
	{
		std::cerr << "Erreur fcntl" << std::endl;
		close(_serverFd);
		_serverFd = -1;
		return (false);
	}

	return (true);
}

void Server::initPoll()
{
	pollfd server_poll_fd;
	server_poll_fd.fd = _serverFd;
	server_poll_fd.events = POLLIN;
	_fds.push_back(server_poll_fd);
}

bool Server::checkPoll()
{
	int ret = poll(&_fds[0], _fds.size(), -1);
	if (ret < 0)
	{
		std::cerr << "Erreur poll" << std::endl;
		return (false);
	}
	return (true);
}

void Server::acceptClient()
{
	int client_fd = accept(_serverFd, NULL, NULL);
	if (client_fd < 0)
		return;

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
	_fds.push_back(client_poll_fd);

	_clients[client_fd] = Client(client_fd);
	std::cout << "Client ajouté dans poll" << std::endl;
}

bool Server::handleClient(int i)
{
	Client &client = _clients[_fds[i].fd];
	char buffer[1024];
	int bytes = recv(_fds[i].fd, buffer, sizeof(buffer) - 1, 0);
	if (bytes <= 0)
	{
		close(_fds[i].fd);
		_clients.erase(_fds[i].fd);
		_fds.erase(_fds.begin() + i);
		return (true);
	}
	buffer[bytes] = '\0';
	client.request += buffer;
	std::cout << client.request << std::endl;

	client.response =
		"HTTP/1.1 200 OK\r\n"
		"Content-Length: 5\r\n"
		"Content-Type: text/plain\r\n"
		"Connection: close\r\n"
		"\r\n"
		"Hello";
	_fds[i].events = POLLOUT;
	return (false);
}

void Server::sendResponse(int i)
{
	Client &client = _clients[_fds[i].fd];
	
	send(_fds[i].fd, client.response.c_str(), client.response.size(), 0);
	
	close(_fds[i].fd);
	_clients.erase(_fds[i].fd);
	_fds.erase(_fds.begin() + i);
}

void Server::handleEvents()
{
	for (int i = 0; i < (int)_fds.size(); i++)
	{
		if (_fds[i].fd == _serverFd)
		{
			if (_fds[i].revents & POLLIN)
				acceptClient();
		}
		else
		{
			bool removed = false;
			if (_fds[i].revents & POLLIN)
				removed = handleClient(i);
			if (!removed && (_fds[i].revents & POLLOUT))
			{
				sendResponse(i);
				removed = true;
			}
			if (removed)
				i--;
		}
	}
}

void Server::run()
{
	initPoll();
	while (true)
	{
		if (!checkPoll())
			break;
		handleEvents();
	}
}
