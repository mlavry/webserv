/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cnamoune <cnamoune@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/31 14:32:21 by cnamoune          #+#    #+#             */
/*   Updated: 2026/06/02 16:30:08 by cnamoune         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CgiHandler.hpp"
#include <unistd.h>
#include <fcntl.h>

// CgiHandler::CgiHandler() : cgi_pid(-1), stats(CGI_INIT), bytes_sended(0), start_time(0), envp(NULL)
// {
//     pipe_out = -1;
//     pipe_in = -1;
// }

// CgiHandler::~CgiHandler()
// {
// 	if (this->envp)
// 	{
// 		delete[]	envp;
// 		envp = NULL;
// 	}
// }

// void	CgiHandler::create_env_variable(const Request& request, const ServerConfig *config)
// {
// 	if (request.method == "GET" || request.method == "POST")
// 		env_variable.push_back("REQUEST_METHOD=" + request.method);
// 	env_variable.push_back("QUERY_STRING=" + request.query);
// 	env_variable.push_back("PATH_INFO=" + request.path);
// 	std::string	script_name = request.path.substr(request.path.find_last_of("/") + 1, request.path.length());
// 	env_variable.push_back(("SCRIPT_FILENAME=" + config->root + script_name));
// 	if (request.method == "POST")
// 	{
// 		for (std::map<std::string, std::string>::const_iterator	it = request.header.begin(); it != request.header.end(); ++it)
// 		{
// 			if (it->first == "Content-Type" && !it->second.empty())
// 				env_variable.push_back("CONTENT_TYPE=" + it->second);
// 			if (it->first == "Content-Length" && !it->second.empty())
// 				env_variable.push_back("CONTENT_LENGTH=" + it->second);
// 		}
// 	}
// }

// void	CgiHandler::handle_script(const Request& request, const ServerConfig *config, std::string cgi_extention)
// {   
//     bytes_sended = 0;
// 	create_env_variable(request, config);
//     stats = CGI_INIT;

// 	int	fd_in[2];
// 	int	fd_out[2];

// 	if (pipe(fd_in) < 0 || pipe(fd_out))
// 	{
// 		stats = CGI_ERROR;
// 		return ;
// 	}

// 	cgi_pid = fork();

// 	if(cgi_pid < 0)
// 	{
// 		stats = CGI_ERROR;
// 		return ;
// 	}

// 	if (cgi_pid == 0)
// 	{
// 		lauch_script(request, config, cgi_extention);
// 	}
// 	else
// 	{
// 		close(fd_in[0]);
// 		close(fd_out[1]);

// 		this->pipe_out = fd_out[0];
// 		this->pipe_in = fd_in[1];

// 		fcntl(pipe_out, F_SETFL, O_NONBLOCK);
// 		fcntl(pipe_in, F_SETFL, O_NONBLOCK);

//         if (request.method == "POST")
//             stats = CGI_WRITING;
//         else
//             stats = CGI_READING;
// 	}
// }
