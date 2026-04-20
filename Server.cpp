/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mlavry <mlavry@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/17 13:22:39 by mlavry            #+#    #+#             */
/*   Updated: 2026/04/20 15:40:03 by mlavry           ###   ########.fr       */
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

void Server::run()
{
	pollfd server_poll_fd;
	server_poll_fd.fd = _serverFd;
	server_poll_fd.events = POLLIN;
	_fds.push_back(server_poll_fd);

	while (true)
	{
		int ret = poll(&_fds[0], _fds.size(), -1);
		if (ret < 0)
		{
			std::cerr << "Erreur poll" << std::endl;
			break;
		}
		for (int i = 0; i < (int)_fds.size(); i++)
		{
			if (!(_fds[i].revents & POLLIN))
				continue;

			if (_fds[i].fd == _serverFd)
			{
				int client_fd = accept(_serverFd, NULL, NULL);
				if (client_fd < 0)
					continue;

				int flags = fcntl(client_fd, F_GETFL, 0);
				if (flags < 0 || fcntl(client_fd, F_SETFL, flags | O_NONBLOCK)
					< 0)
				{
					close(client_fd);
					continue;
				}
				
				pollfd client_poll_fd;
				client_poll_fd.fd = client_fd;
				client_poll_fd.events = POLLIN;
				_fds.push_back(client_poll_fd);
				std::cout << "Client ajouté dans poll" << std::endl;
			}
			else
			{
				char buffer[1024];
				int bytes = recv(_fds[i].fd, buffer, sizeof(buffer) - 1, 0);
				if (bytes > 0)
				{
					buffer[bytes] = '\0';
					std::cout << buffer << std::endl;

					const char* response =
						"HTTP/1.1 200 OK\r\n"
						"Content-Length: 5\r\n"
						"Content-Type: text/plain\r\n"
						"Connection: close\r\n"
						"\r\n"
						"Hello";
				
					send(_fds[i].fd, response, strlen(response), 0);
				}
				close(_fds[i].fd);
				_fds.erase(_fds.begin() + i);
				i--;
			}
		}
	}
}
