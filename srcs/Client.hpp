/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mlavry <mlavry@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/21 13:07:54 by mlavry            #+#    #+#             */
/*   Updated: 2026/06/03 12:15:21 by mlavry           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

#include "Request.hpp"
#include "Response.hpp"
// #include "CgiHandler.hpp"
#include <string>
#include <cstddef>
#include <ctime>

class Client
{
	public:
		//----------- Construct ----------
		Client();
		Client(int fd);

		//----------- Variable publique ----------
		int 			fd;
		Request			request;
		ClientRequest	parser;
		HttpResponse	response_builder;
		std::string 	response;
		size_t			bytesSent;
		bool 			isKeepAlive;
		
		int 			listenFd;

		bool 			hasStartTime;
		time_t 			startTime;
		time_t 			lastActivity;

		int				statusCode;
		std::string 	ip;

		// CgiHandler		cgi;
};

#endif