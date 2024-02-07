/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   http_TcpServer.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: almelo <almelo@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/24 15:31:30 by almelo            #+#    #+#             */
/*   Updated: 2024/02/06 03:12:07 by almelo           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTP__TCP_SERVER_HPP
# define HTTP__TCP_SERVER_HPP

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
			int			_socket;

			int		_newSocket;
			//long	_incomingMessage;

			struct sockaddr_in	_socketAddr;
			unsigned int		_socketAddrLen;

			std::string	_serverMessage;
			std::string	_request;

			int			_startServer(void);
			void		_acceptConnection(int& newSocket);
			std::string	_buildResponse(void);
			void		_sendResponse(void);
			void		_closeServer(void);
	};
} // namespace http
#endif
