/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mlavry <mlavry@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/14 17:15:28 by mlavry            #+#    #+#             */
/*   Updated: 2026/04/18 02:06:25 by mlavry           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

int main(void)
{
	Server server;
	if (!server.initServer())
		return (1);
	server.run();
	return (0);
	/*int server_fd = socket(AF_INET, SOCK_STREAM, 0);
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



	
	std::vector<pollfd> fds;
	pollfd server_poll_fd;
	server_poll_fd.fd = server_fd;
	server_poll_fd.events = POLLIN;
	fds.push_back(server_poll_fd);

	while (true)
	{
		int ret = poll(&fds[0], 1, -1);
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
			fcntl(client_fd, F_SETFL, O_NONBLOCK);
			pollfd client_poll_fd;
			client_poll_fd.fd = client_fd;
			client_poll_fd.events = POLLIN;
			fds.push_back(client_poll_fd);
			std::cout << "Client ajouté dans poll" << std::endl;
			
			char buffer[1024];
			int bytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
			if (bytes > 0)
			{
				buffer[bytes] = '\0';
				std::cout << buffer << std::endl;

				const char* response = 
					"HTTP/1.1 200 OK\r\n"
					"\r\n"
					"Hello";
				
				send(client_fd, response, strlen(response), 0);
			}
			close(client_fd);
		}
	}*/
}

//AF_INET IPv4
//SOCK_STREAM connexion TCP

//pour listen on deinit le nombre de connection pouvant attendre
//	avec une macro qui donne le maximum pour l'os 