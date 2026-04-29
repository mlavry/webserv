/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cnamoune <cnamoune@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/20 14:43:26 by cnamoune          #+#    #+#             */
/*   Updated: 2026/04/29 21:26:27 by cnamoune         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"
#include <cstdlib>
#include <cerrno>
#include <sstream>
#include <iostream>
#include <limits.h>

ClientRequest::ClientRequest()
{
	status = READING_REQUEST;
	content_length = 0;
	body_bytes_readed = 0;
	file_fd = -1;
	http_error_code = 0;
}

ClientRequest::~ClientRequest()
{
	
}

void ClientRequest::parse_request_line(Request& request)
{
    size_t		request_length;
	std::string	line;

	request_length = data.find("\r\n");
	line = data.substr(0, request_length);
	
	// we extracted the request and we now erase it so it will start from the header (+ 2 for the \r\n);
	data.erase(0, request_length + 2);
	std::istringstream	iss(line);

	iss >> request.method >> request.path >> request.http_version;
	if (request.method.empty() || request.path.empty() || request.http_version.empty())
	{
		status = ERROR;
		http_error_code = 400;
		return ;
	}
	if (request.method != "GET" || request.method != "HEAD"
		|| request.method != "POST" || request.method != "DELETE")
	{
		status = ERROR;
		http_error_code = 501;
		return ;
	}
}

void			ClientRequest::parse_body(const char *body, size_t body_size, Request& request)
{
	/*
	if (!is_available_method_with_path(request.method, request.path)) 
	{
		status = ERROR;
		set_error_code(405);
	}
	*/

	size_t bytes_to_read = body_size;
    
    if (body_bytes_readed + body_size > content_length)
	{	
        bytes_to_read = content_length - body_bytes_readed;
    }

	request.body.insert(request.body.end(), body, body + bytes_to_read);

	body_bytes_readed += bytes_to_read;
	if (body_bytes_readed >= content_length)
		status = COMPLETE;
}

RequestState	ClientRequest::parse_chunk(const char *buffer, size_t bytes_read, Request& request)
{
	if (status == READING_BODY && request.method == "POST")
	{
		parse_body(buffer, bytes_read, request);
		return (status);
	}
	// else if (status == READING_CHUNKED && request.method == "POST")
	// {
	// 	parse_chunked_body(buffer, bytes_read, request);
	// 	return (status);
	// }
	
	if (status == READING_HEADER && !request.method.empty())
	{
		parse_headers(request);
		return(status);
	}
	if (status == READING_REQUEST)
	{
		parse_request_line(request);
		return (status);
	}

	if (status == COMPLETE)
		return (status);
	if (status == ERROR)
		return (status);
	return (TIME_OUT);
}

void				ClientRequest::set_error_code(int code_error)
{
	http_error_code = code_error;
}

int					ClientRequest::get_error_code() const
{
	return (http_error_code);
}

RequestState	ClientRequest::get_status() const
{
	return (status);
}

int	ClientRequest::content_size(const std::string& str)
{
	char	*end;
	long	l;

	if (str.length() > 10 || (str.length() == 10 && str > "2147483647"))
	{
		status = ERROR;
		set_error_code(413);
		return (-1);
	}

	l = strtol(str.c_str(), &end, 10);
	return (static_cast<int>(l));
}

int ClientRequest::is_valid_size(const std::string& str, int code)
{
	if (str.empty())
	{
		status = ERROR;
		set_error_code(400);
		return (-1);
	}
	if (code == 0)
	{
		for (size_t i = 0; i < str.size(); ++i)
		{
			if (!std::isdigit(static_cast<unsigned char>(str[i])))
			{
				status = ERROR;
				set_error_code(400);
				return (-1);
			}
		}
	}
	else if (code == 1)
	{
		if (str != "chunked")
		{
			status = ERROR;
			set_error_code(501);
			return (-1);
		}
		return (1);
	}
	return (content_size(str));
}

void ClientRequest::parse_headers(Request& request)
{
	size_t	end_of_line;
	status = READING_HEADER;
	int		as_content_size = 0;

	while ((end_of_line = data.find("\r\n")) != std::string::npos)
	{
		std::string	line = data.substr(0, end_of_line);
		data.erase(0, end_of_line + 2);

		if (line.empty())
		{
			if (request.header.find("Content-Length") != request.header.end())
			{
				int i = is_valid_size(request.header["Content-Length"], 0);
				if (i < 0)
					return ;
				status = READING_BODY;
				content_length = i;
				as_content_size++;
			}
			else if (request.header.find("Transfer-Encoding") != request.header.end())
			{
				if (is_valid_size(request.header["Transfer-Encoding"], 1) < 0)
					return ;
				status = READING_CHUNKED;
				as_content_size++;
			}
			else
				status = COMPLETE;
			return ;
		}
		size_t	key_pos = line.find(':');
		if (key_pos != line.npos)
		{
			std::string key = line.substr(0, key_pos);
			std::string value = line.substr(key_pos + 1);

			while (!value.empty() && (value[0] == ' ' || value[0] == '\t'))
				value.erase(0, 1);

			request.header[key] = value;
		}
		else
		{
			status = ERROR;
			set_error_code(400);
			return ;
		}
	}
	// this is requiered to check if header have both Content-Length and Transfer-Encoding in the same header
	if (as_content_size > 1)
	{
		status = ERROR;
		set_error_code(400);
		return ;
	}
}