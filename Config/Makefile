# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: qliso <qliso@student.42.fr>                +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/06/03 13:23:11 by mafritz           #+#    #+#              #
#    Updated: 2025/06/03 20:06:12 by qliso            ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME        := webserv
CXX         := c++
CXXFLAGS    := -Wall -Wextra -Werror -Wno-unused -std=c++98
SRCS        := 0_Utils.cpp 1_Lexing.cpp 2_Parsing.cpp 3_Build.cpp \
              4_ListeningSocket.cpp 5_Server.cpp 6_ClientConnection.cpp \
			  7_HttpRequest.cpp \
              Console.cpp main.cpp
OBJS        := $(SRCS:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re