/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mlavry <mlavry@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/21 13:07:54 by mlavry            #+#    #+#             */
/*   Updated: 2026/05/05 13:02:25 by mlavry           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

#include "Request.hpp"

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
		int fd;
		Request		request;
		ClientRequest parser;
		std::string response;
		size_t bytesSent;

		clock_t startTime;
		std::string ip;
};

#endif