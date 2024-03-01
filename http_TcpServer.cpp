/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   http_TcpServer.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: almelo <almelo@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/24 15:31:39 by almelo            #+#    #+#             */
/*   Updated: 2024/03/01 16:45:13 by almelo           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "http_TcpServer.h"
#include "http_Message.h"

#include <arpa/inet.h>
#include <sys/socket.h>

#include <cstddef>

#include <fstream>
#include <iostream>
#include <sstream>

#include <string>
#include <map>

#include <unistd.h>
#include <stdlib.h>

#include <fcntl.h>

namespace
{
	//int const BUFFER_SIZE = 4096; // 4KB
	int const BUFFER_SIZE = 256; // 1KB

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
			_socketAddr(),
			_socketAddrLen(sizeof(_socketAddr)),
			_serverMessage()
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
		// create the "server" socket that's going
		// to listen to connections
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

		// build log message
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

		while (true)
		{
			
			_acceptConnection(_newSocket);

			std::stringstream	requestData;
			_readRequestData(requestData);

			struct Request* request = _parseRequest(requestData);

			// to do:
			// create a Response struct/class
			// to manage headers and body separately 
			_serverMessage = _buildResponse(*request);
			delete request;

			_sendResponse();
			close(_newSocket);
		}
	}

	void	TcpServer::_readRequestData(std::stringstream& requestData)
	{
		// set the socket to non-blocking
		//fcntl(_newSocket, F_SETFL, O_NONBLOCK);

		char	buffer[BUFFER_SIZE] = {0};
		int		bytesReceived;
		while (true)
		{
			// the read call will block by default
			// waiting for data to be available for reading
			bytesReceived = read(_newSocket, buffer, BUFFER_SIZE - 1);

			// is something wrong with the reading process?
			if (bytesReceived == -1)
			{
				close(this->_newSocket);
				exitWithError("Failed to read data from client");
			}

			// did the client close the connection?
			if (bytesReceived == 0)
			{
				break ;
			}

			buffer[bytesReceived] = '\0';
			requestData << buffer;

			// temporary solution while still using blocking sockets
			if (requestData.str().find("\r\n\r\n") != std::string::npos)
			{
				break ;
			}
		}
	}

	struct Request*	TcpServer::_parseRequest(std::stringstream& requestData)
	{
		// extract start-line(request-line) data
		struct Request* request = new Request();
		requestData >> request->method;
		requestData >> request->targetURI;
		requestData >> request->httpVersion;

		return request;
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

	std::string TcpServer::_buildResponse(struct Request& request)
    {
		std::string const INDEX = "index.html";

		//std::string const ROOT = "/home/algacyr/Desktop/oracle-next-education/oneCrypt";
		std::string const ROOT = "/home/algacyr/Desktop/forum-mariana";

		if (request.targetURI== "/") {
			request.targetURI.append(INDEX);
		}

		struct Response* response = new Response();

		// to do:
		// create a function/struct with status code-message table
		response->statusCode = "200";
		response->statusMessage = "OK";

		// to do:
		// if there is no index.html, create directory listing for /
		std::string targetPath = ROOT + request.targetURI;
		std::ifstream	ifs(targetPath.c_str(), std::ifstream::binary);
		if (!ifs.is_open())
		{
			response->statusCode = "404";
			response->statusMessage = "File not found";
		}

		// message constant characters
		std::string const SP = " ";
		std::string const CRLF = "\r\n";
		
		std::ostringstream bodyBuf;
		bodyBuf << ifs.rdbuf();
		ifs.close();

		response->body = bodyBuf.str();

		// to do:
		// move this MIME setup to another place
		std::map<std::string, std::string> extToMIME;
		extToMIME["html"] = "text/html";
		extToMIME["css"] = "text/css";
		extToMIME["js"] = "text/javascript";

		extToMIME["svg"] = "image/svg+xml";
		extToMIME["jpg"] = "image/jpeg";

		// extract file extension
		std::size_t const extPos = request.targetURI.find_last_of(".");
		std::string ext = request.targetURI.substr(extPos+1);

		// default MIME type
		std::string MIMEType = "application/octet-stream";

		std::map<std::string, std::string>::iterator const MIMETypeIt = extToMIME.find(ext);
		if (MIMETypeIt != extToMIME.end())
		{
			MIMEType = MIMETypeIt->second;
		}

		// build message header
		std::ostringstream ossMessageHeader;
		ossMessageHeader
			<< "HTTP/1.1" << SP
			<< response->statusCode << SP
			<< response->statusMessage << CRLF // status line end
			<< "Content-Type:" << SP << MIMEType << CRLF
			<< "Content-Length:" << SP << response->body.size() << CRLF
			<< CRLF;

		return ossMessageHeader.str() + response->body;
    }

	void	TcpServer::_sendResponse(void)
	{
		// to do:
		// create a loop to guarantee that all data
		// is delivered to client
		
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
