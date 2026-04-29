/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mlavry <mlavry@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/14 17:15:28 by mlavry            #+#    #+#             */
/*   Updated: 2026/04/28 18:37:01 by mlavry           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include <csignal>

volatile sig_atomic_t g_running = 1;
//volatile dit au compilateur
//“cette variable peut changer à tout moment en dehors du flux normal du code”

void handle_signal(int signal)
{
	(void)signal;
	g_running = 0;	
}

int main(void)
{
	signal(SIGINT, handle_signal);
	signal(SIGTERM, handle_signal);
	
	Server server;
	if (!server.initServer())
		return (1);
	server.run();
	return (0);
}
