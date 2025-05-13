# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: qliso <qliso@student.42.fr>                +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/05/07 11:54:51 by mafritz           #+#    #+#              #
#    Updated: 2025/05/13 10:48:52 by qliso            ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = webserv
#SRC = main.cpp MyClientConnection.cpp MyServer.cpp MySocket.cpp 
SRC = main.cpp
OBJ_DIR = obj
OBJ = $(SRC:%.cpp=$(OBJ_DIR)/%.o)
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -Wno-unused #-std=c++98

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)

$(OBJ_DIR)/%.o: %.cpp
	mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean
	$(MAKE) all


.PHONY: all clean fclean re
