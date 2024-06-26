/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   http_TcpServer.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: almelo <almelo@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/24 15:31:39 by almelo            #+#    #+#             */
/*   Updated: 2024/05/27 14:23:50 by almelo           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "http_TcpServer.h"
#include "http_Message.h"

#include <arpa/inet.h>
#include <cstring>
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

#include <dirent.h>
#include <sys/stat.h>

namespace
{
    // message constant characters
    std::string const SP = " ";
    std::string const CRLF = "\r\n";

    // temporary constants
    std::string const INDEX = "index.html";
    //std::string const ROOT = "/home/algacyr/Desktop/42/projects/webserv";
    std::string const ROOT = "/home/algacyr/Desktop";
    int const BUFFER_SIZE = 4096; // 4KB
    //int const BUFFER_SIZE = 256; // test

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
    TcpServer::TcpServer(std::string ipAddr, int port) :
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

    int TcpServer::_startServer(void)
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

    void TcpServer::startListen(void)
    {
	if (listen(_socket, 20) < 0)
	{
	    exitWithError("Socket listen failed");
	}

	// Build log message
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

	    // TODO: Log every req delivered

	    std::stringstream reqData;
	    _readRequestData(reqData);

	    struct Request* req = _parseRequest(reqData);
	    struct Response* res = _handleRequest(*req);

	    _sendResponse(*res);

	    close(_newSocket);
	    delete req;
	    delete res;
	}
    }

    void TcpServer::_readRequestData(std::stringstream& reqData)
    {
	// Set the socket to non-blocking
	//fcntl(_newSocket, F_SETFL, O_NONBLOCK);
	char buffer[BUFFER_SIZE] = {0};
	int bytesReceived;
	while (true)
	{
	    // NOTE: The read call will block by default
	    // waiting for data to be available for reading
	    bytesReceived = read(_newSocket, buffer, BUFFER_SIZE - 1);
	    // Is something wrong with the reading process?
	    // Did the client close the connection? <CTRL-S-R>
	    if (bytesReceived <= 0)
	    {
		close(this->_newSocket);
		break ;
	    }
	    buffer[bytesReceived] = '\0';
	    reqData << buffer;
	    // Temporary solution while still using blocking sockets
	    if (reqData.str().find("\r\n\r\n") != std::string::npos)
	    {
		break ;
	    }
	}
    }

    struct Request* TcpServer::_parseRequest(std::stringstream& reqData)
    {
	// Extract start-line(req-line) data
	struct Request* req = new Request();
	reqData >> req->method;
	reqData >> req->targetURI;
	reqData >> req->httpVersion;

	std::string line;
	while (std::getline(reqData, line))
	{
	    // Remove delimiter (:) and space (SP)
	    std::size_t del = line.find_first_of(":");
	    if (del != std::string::npos)
	    {
		std::string name = line.substr(0, del);
		std::string value = line.substr(del+2);
		req->fieldLines[name] = value;
	    }
        }
	return req;
    }

    void TcpServer::_acceptConnection(int& _newSocket)
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

    struct Response* TcpServer::_handleRequest(struct Request& req)
    {
	struct Response* res = new Response();

	DIR* dir;
	struct dirent* entry;

	//std::string origin = "/home/algacyr/Desktop";
	std::string origin = ROOT;
	std::string path = origin + req.targetURI;

	// could it be opened as a directory?
	// check if there is an index file
	if ((dir = opendir(path.c_str())) != NULL)
	{
	    while ((entry = readdir(dir)) != NULL)
	    {
	        // is it the index.html file?
		if (entry->d_name == INDEX)
		{
		    std::string const entryPath = path+"/"+entry->d_name;

		    std::ifstream ifs(entryPath.c_str(), std::ifstream::binary);
		    if (!ifs.is_open())
		    {
		        log("Error opening the file");
		    }

		    // read target file data
		    std::ostringstream bodyBuf;
		    bodyBuf << ifs.rdbuf();
		    ifs.close();

		    // build body message
		    res->body = bodyBuf.str();

		    // set header data
		    res->statusCode = "200";
		    res->statusMessage = "OK";

		    // to do: pass the ultimate path for getMIMEType
		    // instead of the entire req
		    res->fieldLines["ContentType"] = _getMIMEType(INDEX);
		    res->fieldLines["ContentLength"] = res->body.size();

		    // build header message
		    res->header = _buildMessageHeader(*res);

		    closedir(dir);
		    return res;
		} // END found index.html file
	    } // END check if there is an index file
	    closedir(dir);

	    // index.html not found?
	    // go for directory listing
	    std::ostringstream ossMessageBody;

	    ossMessageBody <<
		"<html>"
		"<head>"
		"<title>Directory Listing</title>"
		"</head>"
		"<body>"
		"<h1>Directory listing for "
		<< req.targetURI + "</h1>"
		"<ul>";

	    if ((dir = opendir(path.c_str())) != NULL)
	    {
		while ((entry = readdir(dir)) != NULL)
		{
		    // skip . and .. dirs
		    if (strcmp(entry->d_name, ".") == 0 ||
		        strcmp(entry->d_name, "..") == 0)
		    {
		        continue ;
		    }

		    std::string internalPath = path+entry->d_name;
		    struct stat fileInfo;
		    if (stat(internalPath.c_str(), &fileInfo) == -1)
		    {
		        log("Error accessing file information");
		    }

		    ossMessageBody
		        << "<li>"
		        << "<a href=\""
		        << req.targetURI;
		    ossMessageBody
		        << entry->d_name;
		    if (S_ISDIR(fileInfo.st_mode))
		    {
			ossMessageBody << "/";
		    }
		    ossMessageBody
			<< "\">"
			<< entry->d_name
			<< "</a>"
			<< "</li>";
	        }
	        closedir(dir);
	    }
	    ossMessageBody << "</ul></body></html>";
	    // END building directory list

	    // set body data
	    res->body = ossMessageBody.str();

	    // set header data
	    res->statusCode = "200";
	    res->statusMessage = "OK";

	    res->fieldLines["ContentType"] = "text/html";
	    res->fieldLines["ContentLength"] = res->body.size();

	    // build header message
	    res->header = _buildMessageHeader(*res);
	    return res;
	}
	closedir(dir);

	// after trying to open it as a directory
	// try it as a regular file
	std::ifstream ifs(path.c_str(), std::ifstream::binary);

	if (!ifs.is_open())
	{
	    std::ifstream ifs("404.html"); // default 404 page

	    // read target file data
	    std::ostringstream bodyBuf;
	    bodyBuf << ifs.rdbuf();
	    ifs.close();

	    // set body data 
	    res->body = bodyBuf.str();

	    // set header data
	    res->statusCode = "404";
	    res->statusMessage = "File not found";
	    res->fieldLines["ContentType"] = "text/html";
	    res->fieldLines["ContentLength"] = res->body.size();

	    // build header message
	    res->header = _buildMessageHeader(*res);

	    return res;
	}

	// read target file data
	std::ostringstream bodyBuf;
	bodyBuf << ifs.rdbuf();
	ifs.close();

	// set body data 
	res->body = bodyBuf.str();

	// set header data
	res->statusCode = "200";
	res->statusMessage = "OK";
	res->fieldLines["ContentType"] = _getMIMEType(req.targetURI);
	res->fieldLines["ContentLength"] = res->body.size();

	// build header message
	res->header = _buildMessageHeader(*res);

	return res;
    }

    std::string TcpServer::_buildMessageHeader(struct Response& res)
    {
	std::ostringstream ossMessageHeader;
	// status-line
	ossMessageHeader
	    << "HTTP/1.1" << SP
	    << res.statusCode << SP
	    << res.statusMessage << CRLF
	    // header
	    << "Content-Type:" << SP << res.fieldLines["ContentType"] << CRLF
	    << "Content-Length:" << SP << res.body.size() << CRLF
	    << CRLF;
	return ossMessageHeader.str();
    }

    void TcpServer::_sendResponse(struct Response& res)
    {
	// TODO: Create a loop to guarantee that all data
	// is delivered to the client

	std::string::size_type bytesSent;
        // send status line + header first
        bytesSent = write(
	    _newSocket,
	    res.header.c_str(),
	    res.header.size()
        );

        if (bytesSent < res.header.size())
	{
	    log("Error sending res to client");
	}
	// then send body data
        bytesSent = write(
	    _newSocket,
	    res.body.c_str(),
	    res.body.size()
	);
	if (bytesSent < res.body.size())
	{
	    log("Error sending res to client");
	}
    }

    std::string TcpServer::_getMIMEType(std::string fileName)
    {
	// NOTE: The client(browser) needs this info in order to render
	// the content properly
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

	// Extract file extension
	std::size_t const extPos = fileName.find_last_of(".");
	std::string ext = fileName.substr(extPos+1);

	// Default MIME type
	//std::string MIMEType = "application/octet-stream";
	std::string MIMEType = "text/html";

	std::map<std::string, std::string>::iterator const MIMETypeIt = extToMIME.find(ext);
	if (MIMETypeIt != extToMIME.end())
	{
	    MIMEType = MIMETypeIt->second;
	}
	return MIMEType;
    }

    void TcpServer::_closeServer(void)
    {
	close(_socket);
	close(_newSocket);
	exit(0);
    }
} // namespace http
