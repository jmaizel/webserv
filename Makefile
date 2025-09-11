# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: hsorel <marvin@42.fr>                      +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/09/09 10:59:29 by hsorel            #+#    #+#              #
#    Updated: 2025/09/09 10:59:31 by hsorel           ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = webserv

SRC =	srcs/main.cpp \
		srcs/config/Config.cpp \
		srcs/server/Server.cpp \
		srcs/server/ServerMonitor.cpp \
		srcs/utils/str_utils.cpp

OBJ = ${SRC:.cpp=.o}
CC = c++ -fsanitize=address -g
CFLAGS = #-Wall -Wextra -Werror -std=c++98
CXX = $(CC) $(CFLAGS)

all : ${NAME}

${NAME} : ${OBJ}
	${CC} ${CFLAGS} ${OBJ} -o ${NAME}

clean:
	rm -rf $(OBJ)

fclean: clean
	rm -rf $(NAME)

re: fclean all

.PHONY : all clean fclean re