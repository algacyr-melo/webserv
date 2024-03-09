/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   http_TcpServer.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: almelo <almelo@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/24 15:31:39 by almelo            #+#    #+#             */
/*   Updated: 2024/03/08 23:35:34 by almelo           ###   ########.fr       */
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
	// message constant characters
	std::string const SP = " ";
	std::string const CRLF = "\r\n";

	//int const BUFFER_SIZE = 4096; // 4KB
	int const BUFFER_SIZE = 256; // test

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
			_socketAddrLen(sizeof(_socketAddr))
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

			// to do: log every request delivered

			std::stringstream	requestData;
			_readRequestData(requestData);

			struct Request*		request = _parseRequest(requestData);
			struct Response*	response = _buildResponse(*request);

			_sendResponse(*response);

			close(_newSocket);
			delete request;
			delete response;
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
			// did the client close the connection? e.g ctrl+shift+R
			if (bytesReceived <= 0)
			{
				close(this->_newSocket);
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

		std::string	line;
		while (std::getline(requestData, line))
		{
			std::size_t	del = line.find_first_of(":");

			if (del != std::string::npos)
			{
				std::string	name = line.substr(0, del);
				std::string	value = line.substr(del+2); // skip del and SP

				request->fieldLines[name] = value;
			}
		}
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

	struct Response*	TcpServer::_buildResponse(struct Request& request)
    {
		// to do:
		// check request method and call the correspondent
		// handle function (handleGetRequest)
		std::string const INDEX = "index.html";

		std::string const ROOT = "/home/algacyr/Desktop/42/projects/webserv";
		//std::string const ROOT = "/home/algacyr/Desktop";

		// test solution for query params and default URI
		if (request.targetURI== "/" ||
			request.targetURI.find("?") != std::string::npos) {
				request.targetURI = "/index.html";
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

		// read response data
		std::ostringstream bodyBuf;
		bodyBuf << ifs.rdbuf();
		ifs.close();

		response->body = bodyBuf.str();

		std::string MIMEType = _getMIMEType(request);

		// build message header
		std::ostringstream ossMessageHeader;
		ossMessageHeader
			<< "HTTP/1.1" << SP
			<< response->statusCode << SP
			<< response->statusMessage << CRLF // status line end
			
			<< "Content-Type:" << SP << MIMEType << CRLF
			<< "Content-Length:" << SP << response->body.size() << CRLF
			<< CRLF;

		response->headers = ossMessageHeader.str();

		return response;
    }

	void	TcpServer::_sendResponse(struct Response& response)
	{
		// to do:
		// create a loop to guarantee that all data
		// is delivered to client
		
		std::string::size_type	bytesSent;

		// send status line + headers first
		bytesSent = write(
			_newSocket,
			response.headers.c_str(),
			response.headers.size()
		);

		if (bytesSent < response.headers.size())
		{
			log("Error sending response to client");
		}

		// then send body data
		bytesSent = write(
			_newSocket,
			response.body.c_str(),
			response.body.size()
		);

		if (bytesSent < response.body.size())
		{
			log("Error sending response to client");
		}
	}

	std::string	TcpServer::_getMIMEType(struct Request& request)
	{
		std::map<std::string, std::string> extToMIME;
		extToMIME["html"] = "text/html";
		extToMIME["css"] = "text/css";
		extToMIME["js"] = "text/javascript";

		extToMIME["svg"] = "image/svg+xml";
		extToMIME["jpg"] = "image/jpeg";
		extToMIME["ico"] = "image/vdn.microsoft.icon";

		extToMIME["mp3"] = "audio/mpeg";

		extToMIME["mp4"] = "video/mp4";

		extToMIME["pdf"] = "application/pdf";

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
		return MIMEType;
	}

	void	TcpServer::_closeServer(void)
	{
		close(_socket);
		close(_newSocket);
		exit(0);
	}
} // namespace http
