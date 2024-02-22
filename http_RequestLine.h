/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   http_RequestData.h                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: almelo <almelo@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/21 18:45:40 by almelo            #+#    #+#             */
/*   Updated: 2024/02/21 18:56:58 by almelo           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTP__REQUEST_LINE_H
# define HTTP__REQUEST_LINE_H

# include <string>

namespace http {
	struct	RequestLine
	{
		std::string method;
		std::string target;
		std::string httpVersion;
	};
}
#endif
