/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mlavry <mlavry@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/21 13:07:54 by mlavry            #+#    #+#             */
/*   Updated: 2026/04/21 16:28:39 by mlavry           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

#include <string>

class Client
{
	public:
		//----------- Construct ----------
		Client();
		Client(int fd);

		//----------- Variable publique ----------
		int _fd;
		std::string _request;
};

#endif