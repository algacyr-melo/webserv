NAME		= webserv 

SRC			= main.cpp \
			  http_TcpServer.cpp

OBJ			= $(SRC:.cpp=.o)

CXX			= c++

CXXFLAGS	= -Wall -Wextra -Werror -std=c++98

$(NAME)		: $(OBJ) http_TcpServer.h http_Message.h
	$(CXX) $(OBJ) -o $(NAME)

all			: $(NAME)

.cpp.o		:
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean		:
	$(RM) $(OBJ)

fclean		: clean
	$(RM) $(NAME)

re			: fclean all

.PHONY		: all clean fclean re
