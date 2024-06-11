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

	    void startListen(void);

	private:
	    std::string _ipAddr;
	    int _port;
	    int _socket;
	    int _newSocket;

	    // socket API variables
	    struct sockaddr_in _socketAddr;
	    unsigned int _socketAddrLen;

	    // req parser stuff
	    std::string _pathName;
	    int _startServer(void);
	    void _acceptConnection(int&);
	    void _closeServer(void);

	    // request handlers
	    void _readRequestData(std::stringstream&);
	    struct Request* _parseRequest(std::stringstream&);
	    struct Response* _handleRequest(struct Request&);
	    std::string _buildMessageHeader(struct Response&);
	    void _sendResponse(struct Response&);
	    std::string _getMIMEType(std::string);
    };
} // namespace http
#endif
