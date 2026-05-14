/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cnamoune <cnamoune@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/20 14:43:22 by cnamoune          #+#    #+#             */
		/*   Updated: 2026/05/12 19:08:20 by cnamoune         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_HPP
# define REQUEST_HPP

#include <string>
#include <map>
#include <vector>

enum    RequestState
{
    READING_REQUEST,
    READING_HEADER,
    READING_BODY,
    READING_CHUNKED,
    COMPLETE,
	TIME_OUT,
    ERROR
};

struct Request
{
    std::string                         method;
    std::string                         path;
    std::string                         http_version;

    std::map<std::string, std::string>  header;

	bool								is_a_file();
	void								print_body() const;
	std::string							file_path;
	std::vector<char>	                body;
	void			clear();
};

class ClientRequest
{
    private:
		RequestState	status;							// current reading request status;

		std::string		data;
		size_t			content_length;
		size_t			body_bytes_readed;

		int				file_fd;
		std::string		file_name;

		void			parse_request_line(Request& request);
		void			parse_headers(Request& request);
		void			parse_body(Request& request);
		void			parse_chunked_body(Request& request);
		long			get_chunk_size(const std::string& hex_size);
		std::string		extract_chunk_size();
		int				http_error_code;
		int				is_valid_size(const std::string& str, int code);
		int				content_size(const std::string& str);
		int				is_crlf_correct(int code);
		size_t			current_data_size;
		size_t			current_data_size_readed;
		bool			reading_data_chunked;
		bool			crlf_received;
		
    public:
        ClientRequest();
        ~ClientRequest();

		void			parse_chunk(const char *buffer, size_t bytes_read, Request& request);
		RequestState	get_status() const;

		int				get_error_code() const;
		void			set_error_code(int code_error);
		// void			clear();
};

#endif