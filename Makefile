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
		srcs/server/DeleteHandler.cpp \
		srcs/server/GetHandler.cpp \
		srcs/server/PostHandler.cpp \
		srcs/server/CgiHandler.cpp \
		srcs/server/HandleErrors.cpp \
		srcs/server/ServerMonitor.cpp \
		srcs/utils/str_utils.cpp \
		srcs/http/HttpRequest.cpp \
		srcs/http/HttpResponse.cpp \
		srcs/http/Client.cpp

OBJ = ${SRC:.cpp=.o}
CC = c++ 
CFLAGS = -Wall -Wextra -Werror -fsanitize=address -g -std=c++98
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