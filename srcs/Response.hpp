/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cnamoune <cnamoune@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/29 17:40:29 by mlavry            #+#    #+#             */
/*   Updated: 2026/06/07 18:25:11 by cnamoune         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPRESPONSE_HPP
# define HTTPRESPONSE_HPP

#include <sstream>
#include <errno.h>
#include <dirent.h>
#include "Request.hpp"


class   Client;

enum	ResponseState
{
    RESPONSE_BUILDING,
    RESPONSE_READY_TO_SEND,
    RESPONSE_SENDING,       
    RESPONSE_COMPLETE,
    RESPONSE_SENDED    
};

enum	PathInfo
{
	PATH_IS_FILE,
	PATH_IS_DIRECTORY,
	PATH_DOES_NOT_EXIST,
    PATH_PERMISSION_DENIED
};

class HttpResponse
{
    private:
        
        void                                reset();
        ResponseState                       state;
        PathInfo							get_path_info(const std::string& path) const;
		PathInfo                            target_path_info;
        std::vector<char>                   raw_response;
        size_t                              bytes_sent; 
    
        int                                 status_code;
        std::map<std::string, std::string>  headers;
        std::vector<char>                   body;
        std::string                         executable;
        const ServerConfig*                 server_config;
        const LocationConfig*               location_config;
        std::string                         target_file_path;
        std::string     is_cgi_requested(const std::string& target_path);
        void            handle_cgi(const Request& request, const ServerConfig *config,
                                    Client& client, std::string cgi_extention,
                                    const std::string& target_file_path, PathInfo path_info);
        void            handle_get(const Request& request);
        void            handle_post(const Request& request);
        void            handle_delete(const Request& request);
        void            handle_head(const Request& request);
        
        bool            build_error_from_error_page(int error_code, const ServerConfig *config, const Request& request);
        void            build_autoindex(const std::string& directory_path, const Request& request);
        void            build_final_path(std::string& full_path);
        void            handle_directory_path(std::string& full_path);
        std::string     select_active_index() const;
        
        void            assemble_response(const Request& request);
        std::string     get_mime_type(const std::string& file_path);
        std::string     get_status_message(int code) const;
    
        void            translate_path(const Request& request);
        bool            extract_multipart_file(const std::vector<char>& body, std::string& out_filename,
                                            size_t& out_start, size_t& out_length);

    public:
    
        HttpResponse();
        ~HttpResponse();
        
        void            parse_cgi_output(const std::vector<char>& cgi_outpout, const Request& request);
        
        void            generate(const Request& request, const ServerConfig* config, Client& client);
        
        void            generate_error(int error_code, const ServerConfig* config, const Request& request);
    
        ResponseState   get_state() const;
        
        int             get_http_status() const;
        
        const char*     get_response_data() const;
        
        size_t          get_remaining_bytes() const;
        
        void            add_bytes_sent(size_t bytes); 
};

#endif