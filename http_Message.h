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

	    std::map<std::string, std::string> fieldLines;
    };

    struct Request : Message
    {
	std::string method;
	std::string targetURI;
    };

    struct Response : Message
    {
	std::string statusCode;
	std::string statusMessage;
    };
}
#endif
