/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cnamoune <cnamoune@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/29 17:40:46 by mlavry            #+#    #+#             */
/*   Updated: 2026/06/08 14:30:41 by cnamoune         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"
#include "CgiHandler.hpp"
#include "Client.hpp"
#include <sys/stat.h>
#include <iostream>
#include <algorithm>

HttpResponse::HttpResponse()
{
    status_code = 200;
    bytes_sent = 0;
    state = RESPONSE_BUILDING;
    server_config = NULL;
	location_config = NULL;
};

HttpResponse::~HttpResponse()
{
    
};

bool	HttpResponse::build_error_from_error_page(int error_code, const ServerConfig *config, const Request& request)
{
		std::string active_root = config->root;
        if (!active_root.empty() && active_root[active_root.length() - 1] != '/')
            active_root += "/";
            
        std::string full_error_path = active_root + config->errorPages.at(error_code);

        std::ifstream file(full_error_path.c_str(), std::ios::binary);
        if (file.is_open())
        {
            body.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
            file.close();

            assemble_response(request);
            return (true);
        }

	return (false);
}

void	HttpResponse::generate_error(int error_code, const ServerConfig *config, const Request& request)
{
	std::stringstream	error_message;
	
	state = RESPONSE_BUILDING;
	server_config = config;
	status_code = error_code;
	headers["Content-Type"] = "text/html";
	
	if (config && config->errorPages.find(error_code) != config->errorPages.end())
	{
        if (build_error_from_error_page(error_code, config, request))
            return ;
	}

	error_message   << "<html><body><h1>Error " << error_code << std::endl
                    << " " << get_status_message(error_code)
                    << "</h1></body></html>";
	std::string	str = error_message.str();
	body.assign(str.begin(), str.end());
	

	assemble_response(request);
}

void	HttpResponse::assemble_response(const Request& request)
{
	std::stringstream	header_steam;

	header_steam << "HTTP/1.1 " << status_code << " " << get_status_message(status_code)
				<< "\r\n";
	if (!body.empty())
	{
		std::stringstream	content_size;
		content_size << body.size();
		headers["Content-Length"] = content_size.str();
	}
	else
		headers["Content-Length"] = "0";

	if (request.header.find("Connection") == headers.end() || request.header.at("Connection") != "close")
		headers["Connection"] = "keep_alive";
    else
    {
        headers["Connection"] = "close";
    }

	std::map<std::string, std::string>::const_iterator	it;
	for (it = headers.begin(); it != headers.end(); ++it)
		header_steam << it->first << ": " << it->second << "\r\n";
	header_steam << "\r\n";
	
	std::string	header_str = header_steam.str();
	raw_response.clear();
	raw_response.insert(raw_response.end(), header_str.begin(), header_str.end());
	if (request.method != "HEAD")
		raw_response.insert(raw_response.end(), body.begin(), body.end());
	
	bytes_sent = 0;
	state = RESPONSE_READY_TO_SEND;
}

void HttpResponse::build_autoindex(const std::string& directory_path, const Request& request)
{
    std::stringstream html;

    html << "<html>\n<head><title>Index of " << directory_path << "</title></head>\n<body>\n";
    html << "<h1>Index of " << directory_path << "</h1>\n<hr>\n<pre>\n";
    
    DIR* directory = opendir(directory_path.c_str());
    if (!directory)
    {
        generate_error(500, server_config, request);
        return ;
    }

    struct dirent* ent;
    while ((ent = readdir(directory)) != NULL)
    {
        std::string file_name = ent->d_name;
        html << "<a href=\"" << file_name << "\">" << file_name << "</a>\n";
    }

    closedir(directory);

    html << "</pre>\n<hr>\n";

    if (this->location_config && this->location_config->uploadEnabled)
    {
        html << "<h3>Upload to this directory:</h3>\n";
        html << "<form action=\"\" method=\"POST\" enctype=\"multipart/form-data\">\n";
        html << "  <input type=\"file\" name=\"file\" required>\n";
        html << "  <input type=\"submit\" value=\"Upload File\" style=\"margin-top: 10px; padding: 5px 10px;\">\n";
        html << "</form>\n<hr>\n";
    }

    html << "</body>\n</html>\n";

    std::string succes_upload_body = html.str();
    body.assign(succes_upload_body.begin(), succes_upload_body.end());

    headers["Content-Type"] = "text/html";
    status_code = 200;
    
    assemble_response(request);
}

bool HttpResponse::extract_multipart_file(const std::vector<char>& body, std::string& out_filename, size_t& file_start, size_t& file_end)
{
    const char* filename_tag = "filename=\"";
    const char* boundary_tag = "\r\n--";
	std::string	delimiter = "\r\n\r\n";

    std::vector<char>::const_iterator it = std::search(body.begin(), body.end(), filename_tag, filename_tag + 10);
    if (it != body.end())
    {
        std::vector<char>::const_iterator name_start = it + 10;
        std::vector<char>::const_iterator name_end = std::find(name_start, body.end(), '\"');
        if (name_end != body.end())
            out_filename.assign(name_start, name_end);
    }
    else
		return (false);

    it = std::search(body.begin(), body.end(), delimiter.begin(), delimiter.end());
    if (it != body.end())
    {
        file_start = std::distance(body.begin(), it) + 4;

        std::vector<char>::const_reverse_iterator rit = std::search(body.rbegin(), body.rend(), boundary_tag, boundary_tag + 4);

        if (rit != body.rend())
        {
            size_t file_limits = std::distance(body.begin(), rit.base()) - 4; 
            if (file_limits > file_start)
                file_end = file_limits - file_start;
            else
                file_end = body.size() - file_start;
        }
        else
            file_end = body.size() - file_start;

        return (true);
    }
    return (false);
}

void HttpResponse::handle_post(const Request& request)
{
    if (!this->location_config || !this->location_config->uploadEnabled)
        return generate_error(403, server_config, request);

    std::string upload_directory_path = this->location_config->uploadStore;

    if (upload_directory_path[upload_directory_path.length() - 1] != '/')
        upload_directory_path += "/";

    std::string filename;
    size_t file_start = 0;
    size_t file_length = request.body.size();

    std::string content_type;
    if (request.header.find("Content-Type") != request.header.end())
        content_type = request.header.at("Content-Type");

    if (content_type.find("multipart/form-data") != std::string::npos)
    {
        std::cout << "There is a multipart" << std::endl;
        if (!extract_multipart_file(request.body, filename, file_start, file_length))
            return generate_error(400, server_config, request);
    }
    else
    {
        size_t last_slash = request.path.find_last_of('/');
        if (last_slash != std::string::npos && last_slash < request.path.length() - 1)
            filename = request.path.substr(last_slash + 1);
    }

    std::string		final_output_path = upload_directory_path + filename;
    std::ofstream	output_file(final_output_path.c_str(), std::ios::binary);
    
    if (!output_file.is_open())
        return generate_error(500, server_config, request);

    output_file.write(&request.body[file_start], file_length);
    output_file.close();
	
	this->status_code = 201;
	this->headers["Content-Type"] = "text/html";
	
	std::stringstream str;
    str << "<html><body><h1>201 Created</h1><p>File " << filename << " uploaded successfully!</p></body></html>";
    std::string succes_upload_body = str.str();
    
    this->body.assign(succes_upload_body.begin(), succes_upload_body.end());
    assemble_response(request);
}

void	HttpResponse::handle_get(const Request& request)
{
    if (this->target_path_info == PATH_IS_DIRECTORY)
    {
        build_autoindex(target_file_path, request);
        return ;
    }

    std::ifstream file(target_file_path.c_str(), std::ios::binary);

    body.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    file.close();
    headers["Content-Type"] = get_mime_type(target_file_path);

    assemble_response(request);
}

void	HttpResponse::handle_delete(const Request& request)
{
	if (this->target_path_info == PATH_IS_DIRECTORY)
    {
        generate_error(403, server_config, request);
        return ;
    }

    if (std::remove(target_file_path.c_str()) != 0)
    {
        generate_error(403, server_config, request); 
        return ;
    }
    this->status_code = 204;
    this->body.clear();

    assemble_response(request);
}

void	HttpResponse::handle_head(const Request& request)
{
    if (this->target_path_info == PATH_IS_DIRECTORY)
    {
        headers["Content-Type"] = "text/html";
        this->status_code = 200;

        assemble_response(request);
        return;
    }

    struct stat buffer;
    if (stat(target_file_path.c_str(), &buffer) != 0)
    {
        generate_error(500, server_config, request); 
        return ;
    }

    headers["Content-Type"] = get_mime_type(target_file_path);

    std::stringstream size_stream;
    size_stream << buffer.st_size;
    headers["Content-Length"] = size_stream.str();

    this->body.clear();
    assemble_response(request);
}

void HttpResponse::handle_cgi(const Request& request, const ServerConfig *config, Client& client,
                                std::string cgi_extention, const std::string& target_path_file, PathInfo path_info)
{
    this->status_code = client.cgi.handle_script(request, cgi_extention, executable, target_path_file, path_info);
                
    if (client.cgi.stats == CGI_ERROR)
        generate_error(this->status_code, config, request);
    return ;
}

void    HttpResponse::parse_cgi_output(const std::vector<char>& cgi_outpout, const Request& request)
{
    std::string delimiter = "\r\n\r\n";

    std::vector<char>::const_iterator it = cgi_outpout.begin();
    std::vector<char>::const_iterator end = cgi_outpout.end();
    std::vector<char>::const_iterator header_end = std::search(it, end, delimiter.begin(), delimiter.end());

    if (header_end != end)
    {
        std::string         header_str(it, header_end);
        std::istringstream  header_stream(header_str);
        std::string         line;

        while (std::getline(header_stream, line))
        {
            size_t colon_pos = line.find(':');
            if (colon_pos != std::string::npos)
            {
                std::string key = line.substr(0, colon_pos);
                std::string value = line.substr(colon_pos + 1);
                value.erase(0, value.find_first_not_of(" \t\r\n"));
                value.erase(value.find_last_not_of(" \t\r\n") + 1);

                if (key == "Status" || key == "status")
                    this->status_code = std::atoi(value.c_str());
                else
                    headers[key] = value;
            }
        }
        body.assign(header_end + 4, end);
    }
    else
    {
        // std::cerr << "ERROR HERE" << std::endl;
        generate_error(500, server_config, request);
        return ;
    }
    assemble_response(request);
}

void    HttpResponse::generate(const Request& request, const ServerConfig* config, Client& client)
{
	reset();

    this->server_config = config;

    if (!this->server_config)
        return (generate_error(500, server_config, request));

    state = RESPONSE_BUILDING;

	translate_path(request);
	
	if (status_code != 200)
	{
		if (this->status_code >= 300 && this->status_code < 400)
        {
            this->headers["Location"] = this->location_config->redirectTarget;
            this->body.clear();
            assemble_response(request);
        }
        else
            generate_error(this->status_code, config, request);
        return ;
	}

	if (this->location_config && this->location_config->hasMethods)
	{
		if (this->location_config->methods.count(request.method) == 0)
		return (generate_error(405, config, request));
	}

    std::string cgi_extention = is_cgi_requested(this->target_file_path);
    if (cgi_extention != "NO")
    {
        handle_cgi(request, config, client, cgi_extention, this->target_file_path, this->target_path_info);
    }
	else if (request.method == "GET")
        handle_get(request);
    else if (request.method == "POST")
        handle_post(request);
    else if (request.method == "DELETE")
        handle_delete(request);
    else if (request.method == "HEAD")
        handle_head(request);
}

std::string	HttpResponse::is_cgi_requested(const std::string& target_path)
{
    size_t extention_pos = target_path.find_last_of(".");
    
    if (extention_pos != std::string::npos)
    {
        std::string cgi_extention = target_path.substr(extention_pos, target_path.length());
        for (std::map<std::string, std::string>::const_iterator it = location_config->cgi.begin();
                 it != location_config->cgi.end(); ++it)
        {
            if (it->first == cgi_extention && !it->second.empty())
            {
                this->executable = it->second;
				return (cgi_extention);
            }
        }
    }
    return ("NO");  
}

std::string	HttpResponse::get_status_message(int code) const
{
	switch (code)
	{
		case 200:	return "OK";
		case 201:	return "Created";
		case 204:	return "No Content";
		case 400:	return "Bad Request";
		case 401:	return "Unauthorized";
		case 403:	return "Forbidden";
		case 404:	return "Not Found";
		case 405:	return "Method Not Allowed";
		case 408:	return "Request Timeout";
		case 413:	return "Payload Too Large";
		case 500:	return "Internal Server Error";
		case 501:	return "Not Implemented";
		case 502:	return "Bad Gateway";
		case 503:	return "Service Unavailable";
		case 505:	return "HTTP Version Not Supported";
		default:	return "Error";
	}
}

std::string	HttpResponse::get_mime_type(const std::string& file_path)
{
    size_t dot_pos = file_path.find_last_of('.');
    if (dot_pos == std::string::npos)
        return "application/octet-stream";

    std::string ext = file_path.substr(dot_pos + 1);
    if (ext == "html" || ext == "htm")
        return "text/html";
    if (ext == "css")
        return "text/css";
    if (ext == "js")
        return "application/javascript";
    if (ext == "json")
        return "application/json";
    if (ext == "txt")
        return "text/plain";
    if (ext == "jpg" || ext == "jpeg")
        return "image/jpeg";
    if (ext == "png")
        return "image/png";
    if (ext == "gif")
        return "image/gif";
    if (ext == "pdf")
        return "application/pdf";
    if (ext == "py")
        return ("cgi");
    if (ext == "sh")
        return ("cgi");
    return "application/octet-stream";
}

void    HttpResponse::add_bytes_sent(size_t bytes)
{
    bytes_sent += bytes;
    if (bytes_sent >= raw_response.size())
        state = RESPONSE_COMPLETE;
    else
        state = RESPONSE_SENDING;
}

int             HttpResponse::get_http_status() const
{
    return (status_code);
}

ResponseState    HttpResponse::get_state() const
{
    return (state);
}

size_t	HttpResponse::get_remaining_bytes() const
{
	return (raw_response.size() - bytes_sent);
}

const char*	HttpResponse::get_response_data() const
{
	return (raw_response.data() + bytes_sent);
}

std::string HttpResponse::select_active_index() const
{
    std::string active_index = "index.html";
    if (this->location_config && !this->location_config->index.empty())
        active_index = this->location_config->index;
    else if (this->server_config && !this->server_config->index.empty())
        active_index = this->server_config->index;
    return (active_index);
}

void HttpResponse::handle_directory_path(std::string& full_path)
{
    if (full_path.empty() || full_path[full_path.length() - 1] != '/')
        full_path += '/';

    std::string active_index = select_active_index();
    std::string index_path = full_path + active_index;

    PathInfo index_info = get_path_info(index_path);
    if (index_info == PATH_IS_FILE)
    {
        this->target_file_path = index_path;
        this->target_path_info = PATH_IS_FILE;
        return;
    }
    if (index_info == PATH_PERMISSION_DENIED)
    {
        this->status_code = 403;
        return;
    }

    bool autoindex_on = false;
    if (this->location_config && this->location_config->hasAutoindex)
        autoindex_on = this->location_config->autoindex;

    if (autoindex_on)
        this->target_file_path = full_path;
    else
        this->status_code = 403;
}

PathInfo HttpResponse::get_path_info(const std::string& path) const
{
    struct stat buffer;

    // std::cerr << path << std::endl;
    if (stat(path.c_str(), &buffer) != 0)
    {
        if (errno == EACCES)
		{
        	return (PATH_PERMISSION_DENIED);
    	}
    	return (PATH_DOES_NOT_EXIST);
    }
    if (S_ISDIR(buffer.st_mode))
        return (PATH_IS_DIRECTORY);

    return (PATH_IS_FILE);
}

void	HttpResponse::build_final_path(std::string& full_path)
{
	this->target_path_info = get_path_info(full_path);

	if (target_path_info == PATH_DOES_NOT_EXIST)
	{
		// std::cerr << "CGI DONT EXIST" << std::endl;
        this->status_code = 404;
	}
	else if (target_path_info == PATH_PERMISSION_DENIED)
	{
		this->status_code = 403;
	}
	else if (target_path_info == PATH_IS_FILE)
	{
		this->target_file_path = full_path;
	}
	else if (target_path_info == PATH_IS_DIRECTORY)
	{
        handle_directory_path(full_path);
	}
}

void HttpResponse::translate_path(const Request& request)
{
    this->location_config = NULL;
    size_t longest_match = 0;

    for (size_t i = 0; i < server_config->locations.size(); ++i)
    {
        std::string loc_path = server_config->locations[i].path;
        
        if (request.path.find(loc_path) == 0)
        {
            if (loc_path == "/" || request.path.length() == loc_path.length() || request.path[loc_path.length()] == '/')
            {
                if (loc_path.length() > longest_match)
                {
                    longest_match = loc_path.length();
                    this->location_config = &server_config->locations[i];
                }
            }
        }
    }

    if (this->location_config && this->location_config->hasRedirect)
    {
        this->status_code = this->location_config->redirectCode;
        return ;
    }
    
    std::string active_root = server_config->root;
    
    if (this->location_config && !this->location_config->root.empty())
        active_root = this->location_config->root;
    
    if (!active_root.empty() && active_root[active_root.length() - 1] == '/')
        active_root.erase(active_root.length() - 1);
    
    std::string full_path = active_root + request.path;
    
    build_final_path(full_path);
}

void HttpResponse::reset()
{
    status_code = 200;
    bytes_sent = 0;
    state = RESPONSE_BUILDING;
    location_config = NULL;
    target_file_path.clear();
    body.clear();
    raw_response.clear();
    headers.clear();
}