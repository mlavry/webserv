/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mlavry <mlavry@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/17 13:22:25 by mlavry            #+#    #+#             */
/*   Updated: 2026/05/20 17:20:11 by mlavry           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

#include "Client.hpp"

#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>
#include <map>
#include <string>

struct TimeoutConfig
{
	int headerTimeout;
	int bodyTimeout;
	int sendTimeout;

	TimeoutConfig();
};

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
		
		//------------ Variable (timeout config) ----------
		TimeoutConfig _config;
		
		//------------ Forme canonique ----------
		Server(const Server& other);

		//------------ Methode server principale ----------
		void initPoll();
		bool checkPoll();
		bool handleEvents();
		void acceptClient();
		bool handleClient(int i);
		bool sendResponse(int i);
		bool setSocketOption(int fd, int option);
		void removeClient(int i);
		void checkTimeouts();
		
		//------------ Methode logger ----------
		std::string methodColor(const std::string& method) const;
		void printLog(const Client& client) const;
		std::string getTime() const;
		std::string ipToString(unsigned int ip) const;
		std::string statusColor(int status) const;
		std::string truncString(const std::string& str, size_t max) const;
		double getResponseTime(const Client& client) const;

		//------------ Methode Timeout ----------
		int getTimeoutClient(const Client& client, short events) const;
		std::string buildTimeoutResponse() const;
		bool handleTimeout(int i);
};

#endif