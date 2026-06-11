/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestCookies.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mlavry <mlavry@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/11 13:14:35 by mlavry            #+#    #+#             */
/*   Updated: 2026/06/11 13:15:48 by mlavry           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUESTCOOKIES_HPP
# define REQUESTCOOKIES_HPP

#include "Request.hpp"
#include <string>

void parse_cookies(Request& request, const std::string& cookie_header);

#endif