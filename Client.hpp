/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mlavry <mlavry@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/21 13:07:54 by mlavry            #+#    #+#             */
/*   Updated: 2026/04/27 13:45:57 by mlavry           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

#include <string>
#include <cstddef>

class Client
{
	public:
		//----------- Construct ----------
		Client();
		Client(int fd);

		//----------- Variable publique ----------
		int fd;
		std::string request;
		std::string response;
		size_t bytesSent;
};

#endif