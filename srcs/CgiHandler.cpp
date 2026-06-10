/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cnamoune <cnamoune@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/31 14:32:21 by cnamoune          #+#    #+#             */
/*   Updated: 2026/06/10 01:40:10 by cnamoune         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CgiHandler.hpp"
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>

CgiHandler::CgiHandler() : cgi_pid(-1), stats(STAND_BY), envp(NULL), bytes_sended(0)
{
    pipe_out = -1;
    pipe_in = -1;
	http_error_code = 500;
}

CgiHandler::~CgiHandler()
{
	if (this->envp)
	{
		delete[] envp;
		envp = NULL;
	}
}

void	CgiHandler::create_env_variable(const Request& request, const std::string& target_file_path)
{
    env_variable.clear();

    if (request.method == "GET" || request.method == "POST")
        env_variable.push_back("REQUEST_METHOD=" + request.method);

    if (request.method == "GET")
        env_variable.push_back("QUERY_STRING=" + request.query);
    else
        env_variable.push_back("QUERY_STRING="); // Good practice to always initialize it

    env_variable.push_back("PATH_INFO=" + request.path);
    env_variable.push_back("SCRIPT_FILENAME=" + target_file_path);
    env_variable.push_back("SERVER_PROTOCOL=" + request.http_version);

    if (request.method == "POST")
    {
        std::stringstream ss;
        ss << request.body.size();
        env_variable.push_back("CONTENT_LENGTH=" + ss.str());
    }

    // Convert all request headers into HTTP_ CGI environment variables
    for (std::map<std::string, std::string>::const_iterator it = request.header.begin();
         it != request.header.end(); ++it)
    {
        std::string key = it->first;

        // Content-Type is a special CGI variable that doesn't get the HTTP_ prefix
        if (key == "Content-Type")
        {
            env_variable.push_back("CONTENT_TYPE=" + it->second);
            continue;
        }
        
        // Content-Length is handled above based on actual body size, ignore the client's header
        if (key == "Content-Length" || key == "Transfer-Encoding")
        {
            continue;
        }

        // Format the key: prefix with HTTP_, uppercase letters, '-' becomes '_'
        std::string cgi_key = "HTTP_";
        for (size_t i = 0; i < key.length(); ++i)
        {
            if (key[i] == '-')
                cgi_key += '_';
            else
                cgi_key += std::toupper(key[i]);
        }

        env_variable.push_back(cgi_key + "=" + it->second);
    }
}

void	CgiHandler::print_env_variable() const
{
	for (std::vector<std::string>::const_iterator it = env_variable.begin();
		 it != env_variable.end(); ++it)
		std::cout << *it << std::endl;
}

bool	CgiHandler::cgi_exist(const std::string& cgi_extention, const std::string& target_file_path, PathInfo path_info)
{
	// if (path_info != PATH_IS_FILE)
    //     return (false);
	(void)path_info;
    if (target_file_path.empty())
        return (false);

    if (target_file_path.length() < cgi_extention.length())
        return (false);

    if (target_file_path.compare(
            target_file_path.length() - cgi_extention.length(),
            cgi_extention.length(),
            cgi_extention) != 0)
    {
        return (false);
    }
    // if (access(target_file_path.c_str(), R_OK) != 0)
	// {
	// 	this->http_error_code = 403;
	// 	return (false);
	// }
    return (true);
}

bool	CgiHandler::prepare_script(const std::string& cgi_extention,
									const std::string& target_file_path, PathInfo path_info)
{
    if (!cgi_exist(cgi_extention, target_file_path, path_info))
    {
        this->stats = CGI_ERROR;
        return (false);
    }
	return (true);
}

int	CgiHandler::handle_script(const Request& request, const std::string& cgi_extention, const std::string& executable,
									const std::string& target_file_path, PathInfo path_info)
{   
    stats = CGI_INIT;
	bytes_sended = 0;

	this->response_buffer.clear();
	// std::cerr << "CGI LAUCHED" << std:: endl;
	create_env_variable(request, target_file_path);
	print_env_variable();
	if (!prepare_script(cgi_extention, target_file_path, path_info))
		return (this->http_error_code);

	int	fd_in[2];
	int	fd_out[2];

	if (pipe(fd_in) < 0 || pipe(fd_out) < 0)
	{
		stats = CGI_ERROR;
		// std::cerr << "ERROR HERE FROM PIPE" << std::endl;
		this->http_error_code = 500;
		return (this->http_error_code);
	}
	this->stats = CGI_INIT;
	cgi_pid = fork();
	// std::cerr << "FORKING" << std::endl;
	if(cgi_pid < 0)
	{
		close(fd_in[0]);
		close(fd_in[1]);
		close(fd_out[0]);
		close(fd_out[1]);
		// std::cerr << "ERROR HERE FROM FORK" << std::endl;
		this->http_error_code = 500;
		stats = CGI_ERROR;
		return (this->http_error_code);
	}

	if (cgi_pid == 0)
	{
		dup2(fd_in[0], STDIN_FILENO);
		dup2(fd_out[1], STDOUT_FILENO);

		close(fd_in[0]);
		close(fd_in[1]);
		close(fd_out[0]);
		close(fd_out[1]);

		this->envp = new char*[this->env_variable.size() + 1];
		for (size_t i = 0; i < this->env_variable.size(); ++i)
			this->envp[i] = const_cast<char*>(this->env_variable[i].c_str());
		this->envp[this->env_variable.size()] = NULL;
		
		char	*argv[3];
		argv[0] = const_cast<char*>(executable.c_str());
		argv[1] = const_cast<char*>(target_file_path.c_str());
		argv[2] = NULL;
		


        std::cerr << "Executable: [" << argv[0] << "]" << std::endl;
        std::cerr << "Target File: [" << argv[1] << "]" << std::endl;

        execve(argv[0], argv, this->envp);

        // If we reach this line, execve FAILED! Let's print exactly why:
		std::cerr << "EXECVE FAILED FATALLY: " << std::strerror(errno) << std::endl;
        std::cerr << "--------------------" << std::endl;
        
        std::exit(1);
	}
	else
	{
		close(fd_in[0]);
		close(fd_out[1]);

		this->pipe_out = fd_out[0];
		this->pipe_in = fd_in[1];

		fcntl(pipe_out, F_SETFL, O_NONBLOCK);
		fcntl(pipe_in, F_SETFL, O_NONBLOCK);

		if (request.method == "POST")
            this->stats = CGI_WRITING;
        else
            this->stats = CGI_READING;

        return (200);
	}
	return (200);
}
