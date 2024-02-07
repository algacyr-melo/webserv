/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   http_TcpServer.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: almelo <almelo@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/24 15:31:39 by almelo            #+#    #+#             */
/*   Updated: 2024/02/06 23:45:03 by almelo           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "http_TcpServer.hpp"
#include <arpa/inet.h>
#include <iostream>
#include <sstream>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>

#include <fcntl.h>

namespace
{
	int const BUFFER_SIZE = 4096; // 4KB
	//int const BUFFER_SIZE = 4;

	void log(std::string const& message)
	{
		std::cout << message << std::endl;
	}

	void exitWithError(std::string const& errorMessage)
	{
		log("ERROR: " + errorMessage);
		exit(1);
	}
}

namespace http
{
	TcpServer::TcpServer(std::string ipAddr, int port)
		:
			_ipAddr(ipAddr),
			_port(port),
			_socket(),
			_newSocket(),
			//_incomingMessage(),
			_socketAddr(),
			_socketAddrLen(sizeof(_socketAddr)),
			_serverMessage(_buildResponse())
	{
		_socketAddr.sin_family = AF_INET;
		_socketAddr.sin_port = htons(_port);
		_socketAddr.sin_addr.s_addr = inet_addr(_ipAddr.c_str());

		_startServer();
	}
	
	TcpServer::~TcpServer(void)
	{
		_closeServer();
	}

	int	TcpServer::_startServer(void)
	{
		// create the "server" socket that's going to listen to connections
		_socket = socket(AF_INET, SOCK_STREAM, 0);
		if (_socket < 0)
		{
			exitWithError("Cannot create socket");
			return 1;
		}
		// bind socket to the server address
		if (bind(_socket, (sockaddr*)&_socketAddr, _socketAddrLen) < 0)
		{
			exitWithError("Cannot connect socket to address");
			return 1;
		}
		return 0;
	}

	void	TcpServer::startListen(void)
	{
		if (listen(_socket, 20) < 0)
		{
			exitWithError("Socket listen failed");
		}

		std::ostringstream	oss;
		oss << "Serving HTTP on "
			<< inet_ntoa(_socketAddr.sin_addr)
			<< " port " << ntohs(_socketAddr.sin_port)
			<< " (http://"
			<< inet_ntoa(_socketAddr.sin_addr)
			<< ":"
			<< ntohs(_socketAddr.sin_port)
			<< ") ...";
		log(oss.str());

		int bytesReceived;
		while (true)
		{
			_acceptConnection(_newSocket);

			char buffer[BUFFER_SIZE] = {0};
			std::ostringstream request;
			while (true)
			{
				bytesReceived = read(_newSocket, buffer, BUFFER_SIZE - 1);
				buffer[bytesReceived] = '\0';
				request << buffer;

				// break if find the end of request header or
				// connection was closed by client or
				// some error ocurred while reading
				std::size_t reqHeaderEnd = request.str().find("\r\n\r\n");
				if (reqHeaderEnd != std::string::npos ||
					bytesReceived <= 0)
				{
					break ;
				}
			}
			if (bytesReceived == -1)
			{
				close(this->_socket);
				close(this->_newSocket);
				exitWithError("Failed to read bytes from client");
			}
			this->_request = request.str();
			request.clear();

			//std::ostringstream oss;
			//oss << "---- Received request from client ----";
			//log(oss.str());

			// to do: parse the request

			_sendResponse();
			close(_newSocket);
		}
	}

	void	TcpServer::_acceptConnection(int& _newSocket)
	{
		_newSocket = accept(
			_socket,
			(sockaddr*)&_socketAddr,
			&_socketAddrLen
		);

		if (_newSocket < 0)
		{
			std::ostringstream oss;
			oss << "Server failed to accept conection from ADDRESS: "
				<< inet_ntoa(_socketAddr.sin_addr) << "; PORT: "
				<< ntohs(_socketAddr.sin_port);
			exitWithError(oss.str());
		}
	}

	std::string TcpServer::_buildResponse(void)
    {
		std::string htmlFile = "<!DOCTYPE html><html lang=\"en\"><body><h1> HOME </h1><p> Hello from your Server :) </p></body></html>";
		std::ostringstream oss;
		oss << "HTTP/1.1 200 OK\r\n"
			<< "Content-Type: text/html\r\n"
			<< "Content-Length: " << htmlFile.size() << "\r\n"
			<< "\r\n"
			<< htmlFile;

		return oss.str();
    }

	void	TcpServer::_sendResponse(void)
	{
		std::string::size_type	bytesSent;

		bytesSent = write(
			_newSocket,
			_serverMessage.c_str(),
			_serverMessage.size()
		);

		if (bytesSent < _serverMessage.size())
		{
			log("Error sending response to client");
		}
	}

	void	TcpServer::_closeServer(void)
	{
		close(_socket);
		close(_newSocket);
		exit(0);
	}
} // namespace http

