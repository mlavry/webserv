/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mlavry <mlavry@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/17 13:22:25 by mlavry            #+#    #+#             */
/*   Updated: 2026/04/23 11:08:31 by mlavry           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>
#include <map>
#include "Client.hpp"

class Server
{
	public:
		//----------- Construct ----------
		Server();
		~Server();
		
		//----------- Methode publique ----------
		bool initServer();
		void run();
	private:
		//------------ Variable ----------
		int _serverFd;
		sockaddr_in _serverAddress;
		std::vector<pollfd> _fds;
		std::map<int, Client> _clients;
		
		//------------ Forme canonique ----------
		Server(const Server& other);
		Server& operator=(const Server& other);

		//------------ Methode privée ----------
		void initPoll();
		bool checkPoll();
		void handleEvents();
		void acceptClient();
		bool handleClient(int i);
		void sendResponse(int i);
};

#endif