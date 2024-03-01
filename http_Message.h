/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   http_Message.h                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: almelo <almelo@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/29 16:20:26 by almelo            #+#    #+#             */
/*   Updated: 2024/03/01 00:56:05 by almelo           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTP__MESSAGE_H
# define HTTP__MESSAGE_H

# include <string>

namespace http
{
	struct Message
	{
		protected:
			Message() {};

		public:
			std::string httpVersion;
			std::string body;
		// to do: maybe a map for the headers?
	};

	struct Request : Message
	{
		std::string method;
		std::string targetURI;
	};

	struct Response : Message
	{
		unsigned short	statusCode;
		std::string		statusMessage;
	};
}
#endif
