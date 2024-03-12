/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   http_Message.h                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: almelo <almelo@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/29 16:20:26 by almelo            #+#    #+#             */
/*   Updated: 2024/03/11 20:29:44 by almelo           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTP__MESSAGE_H
# define HTTP__MESSAGE_H

# include <string>
# include <map>

namespace http
{
	struct Message
	{
		protected:
			Message() {};

		public:
			std::string httpVersion;

			std::string header;
			std::string body;

			std::map<std::string, std::string>	fieldLines;
	};

	struct Request : Message
	{
		std::string method;
		std::string targetURI;
	};

	struct Response : Message
	{
		std::string	statusCode;
		std::string	statusMessage;
	};
}
#endif
