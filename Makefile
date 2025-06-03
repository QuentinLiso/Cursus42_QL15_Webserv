# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: qliso <qliso@student.42.fr>                +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/05/07 11:54:51 by mafritz           #+#    #+#              #
#    Updated: 2025/06/03 23:15:15 by qliso            ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

#VARIABLES
NAME		=		webserv

SRCS_FILES	=		$(addsuffix .cpp,	\
					0_Utils \
					1_Lexing \
					2_Parsing \
					3_Build \
					4_ListeningSocket \
					5_Server \
					6_ClientConnection \
					7_HttpRequest \
					Console \
					main	\
					)
SRCS_DIR	=		./srcs/
SRCS		=		$(addprefix $(SRCS_DIR), $(SRCS_FILES))


HEADERS		=		$(addsuffix .hpp,	\
					0_Utils \
					1_Lexing \
					2_Parsing \
					3_Build \
					4_ListeningSocket \
					5_Server \
					6_ClientConnection \
					7_HttpRequest \
					Console \
					Colors \
					Includes \
					) \
					$(addsuffix .tpp, \
					0_Utils \
					)

OBJS_PATH	=		objs/
OBJS		=		$(SRCS_FILES:%.cpp=$(OBJS_PATH)%.o)
DEPS		=		$(OBJS:.o=.d)

CC			=		c++
CPPFLAGS	=		-Wall -Wextra -Werror -std=c++98 -MMD -MP -I.
RM			=		rm -rf


#RULES
all				:	$(OBJS_PATH) $(NAME)

$(NAME)			:	$(OBJS)
					$(CC) $(CPPFLAGS) $(OBJS) -o $(NAME)

$(OBJS_PATH)%.o	:	$(SRCS_DIR)%.cpp | $(OBJS_PATH)
					$(CC) $(CPPFLAGS) -c $< -o $@

$(OBJS_PATH)	:
					@mkdir -p $(OBJS_PATH)

clean			:
					$(RM) $(OBJS_PATH)
					@echo "\033[1;31mObject files deleted !\033[0m"

fclean			:	clean		
					$(RM) $(NAME)
					@echo "\033[1;31m$(NAME) deleted !\033[0m"

re				:	fclean all

valgrind		:	all
					valgrind --leak-check=full --trace-children=yes --track-fds=yes ./webserv Configs/lol3.conf


-include	$(DEPS)

.PHONY			:	all clean fclean re