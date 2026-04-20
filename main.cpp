/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mlavry <mlavry@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/14 17:15:28 by mlavry            #+#    #+#             */
/*   Updated: 2026/04/20 13:11:33 by mlavry           ###   ########.fr       */
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
}

//AF_INET IPv4
//SOCK_STREAM connexion TCP

//pour listen on deinit le nombre de connection pouvant attendre
//	avec une macro qui donne le maximum pour l'os 