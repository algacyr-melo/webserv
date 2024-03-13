/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   http_TcpServer.h                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: almelo <almelo@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/24 15:31:30 by almelo            #+#    #+#             */
/*   Updated: 2024/03/12 23:20:16 by almelo           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTP__TCP_SERVER_H
# define HTTP__TCP_SERVER_H

# include <sstream>
# include <sys/socket.h>
# include <arpa/inet.h>
# include <string>

namespace http
{
	class TcpServer
	{
		public:
			TcpServer(std::string ipAddr, int port);
			~TcpServer(void);

			void	startListen(void);

		private:
			std::string	_ipAddr;
			int			_port;

			int	_socket;
			int	_newSocket;

			// socket API variables
			struct sockaddr_in	_socketAddr;
			unsigned int		_socketAddrLen;

			std::string _pathName; // req parser stuff

			int			_startServer(void);
			void		_acceptConnection(int&);
			void		_closeServer(void);

			// request handlers
			void	_readRequestData(std::stringstream&);

			struct Request*		_parseRequest(std::stringstream&);
			struct Response*	_handleRequest(struct Request&);

			std::string	_buildMessageHeader(struct Response&);

			void	_sendResponse(struct Response&);

			std::string	_getMIMEType(std::string);
	};
} // namespace http
#endif
