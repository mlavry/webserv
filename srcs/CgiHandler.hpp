/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cnamoune <cnamoune@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/30 18:27:50 by cnamoune          #+#    #+#             */
/*   Updated: 2026/06/02 16:32:12 by cnamoune         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGIHANDLER_HPP
# define CGIHANDLER_HPP

#include "Response.hpp"

enum	CGI_STATE
{
	CGI_INIT,
	CGI_WRITING,
	CGI_READING,
	CGI_FINISHED,
	CGI_ERROR
};

class   CgiHandler
{
	public :
	
		CgiHandler();
		~CgiHandler();

		void	handle_script(const Request& request, const ServerConfig *config, std::string cgi_extention);
		void	create_env_variable(const Request& request, const ServerConfig *config);
	
		pid_t						cgi_pid;
		int							pipe_in;
		int							pipe_out;
		
		CGI_STATE					stats;
		
		std::vector<std::string>	env_variable;
		char						**envp;
		std::vector<char>			body_to_sends;
		std::vector<char>			response_buffer;

		size_t						bytes_sended;
					
		time_t						start_time;
};

#endif