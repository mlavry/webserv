/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mlavry <mlavry@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/21 13:07:46 by mlavry            #+#    #+#             */
/*   Updated: 2026/05/14 14:10:06 by mlavry           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

Client::Client() : fd(-1), bytesSent(0), hasStartTime(false),
	startTime(0), lastActivity(std::time(NULL))
{

}

Client::Client(int _fd) : fd(_fd), bytesSent(0), hasStartTime(false),
	startTime(0), lastActivity(std::time(NULL))
{
	
}
