/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mlavry <mlavry@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/17 13:22:25 by mlavry            #+#    #+#             */
/*   Updated: 2026/06/03 12:39:03 by mlavry           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

#include "Client.hpp"
#include "Response.hpp"
#include "../config_loader/Config.hpp"

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
		void setConfig(std::vector<ServerConfig> const &list);

	private:
		//------------ Forme canonique ----------
		Server(const Server& other);
	
		//------------ Variable ----------
		std::vector<pollfd> _fds;
		std::map<int, Client> _clients;
		std::map<int, std::vector<const ServerConfig*> > _listenFdToServers;
		std::map<std::string, int> _listenKeyToFd;

		//------------ Variable (config) ----------
		TimeoutConfig _timeoutConfig;
		std::vector<ServerConfig> _configs;

		//------------ Methode server principale ----------
		void initPoll(int fd);
		bool checkPoll();
		bool handleEvents();
		void acceptClient(int listenFd);
		bool handleClient(int i);
		bool sendResponse(int i);
		bool setSocketOption(int fd, int option);
		void removeClient(int i);
		void checkTimeouts();
		void closeAllFds();
		std::string makeListenKey(const std::string& host, int port);
		void resetClientForNextRequet(Client& client);

		const ServerConfig* getMatchedServer(const std::string& host_header, int listenFd) const;

		//------------ Methode logger ----------
		std::string methodColor(const std::string& method) const;
		void printRequestHeader(const Request& request) const;
		void printResponseHeader(const HttpResponse& response) const;
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

		//------------ CGI ----------
		
		std::map<int, int> client_pipe;
};

#endif