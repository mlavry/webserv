/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mlavry <mlavry@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/14 17:15:28 by mlavry            #+#    #+#             */
/*   Updated: 2026/04/16 12:47:19 by mlavry           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>

int main(void)
{
	int server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd < 0)
	{
		std::cerr << "Erreur socket" << std::endl;
		return (1);
	}
	sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(8080);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	
	if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
	{
		std::cerr << "Erreur bind" << std::endl; // fermer fd ?
		return (1);
	}

	if (listen(server_fd, SOMAXCONN) < 0)
	{
		std::cerr << "Erreur listen" << std::endl;
		return (1);
	}
	
	fcntl(server_fd, F_SETFL, O_NONBLOCK);

	pollfd fds[1];
	fds[0].fd = server_fd;
	fds[0].events = POLLIN;

	while (true)
	{
		int ret = poll(fds, 1, -1);
		if (ret < 0)
		{
			std::cerr << "Erreur poll" << std::endl;
			break;
		}
		if (fds[0].revents & POLLIN)
		{
			int client_fd = accept(server_fd, NULL, NULL);
			if (client_fd < 0)
				continue;
			std::cout << "Client connecté" << std::endl;
			close(client_fd);
		}
	}
}

//AF_INET IPv4
//SOCK_STREAM connexion TCP

//pour listen on deinit le nombre de connection pouvant attendre
//	avec une macro qui donne le maximum pour l'os 