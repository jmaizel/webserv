# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: jmaizel <jmaizel@student.42.fr>            +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/06/19 00:00:00 by jmaizel           #+#    #+#              #
#    Updated: 2025/06/26 14:39:17 by jmaizel          ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

# Nom de l'exécutable
NAME = webserv

# Compilateur et flags (respecter la Norme 42)
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98

# Dossiers
SRCDIR = srcs
OBJDIR = objs
INCDIR = includes

# Fichiers sources organisés par dossier
MAIN_SRCS = main/main.cpp

SERVER_SRCS = server/Server.cpp \
              server/ServerNetwork.cpp \
              server/ServerHttp.cpp \
              server/ServerFiles.cpp \
              server/ServerConfigMethods.cpp \
              server/ServerHttpPost.cpp

CONFIG_SRCS = parsing/Parser.cpp

HTTP_SRCS = http/HttpRequestLine.cpp \
            http/HttpHeaders.cpp \
            http/HttpParser.cpp

# Tous les fichiers sources
SRCS = $(MAIN_SRCS) $(SERVER_SRCS) $(CONFIG_SRCS) $(HTTP_SRCS)

# Ajouter le préfixe du dossier source
SRCS_FULL = $(addprefix $(SRCDIR)/, $(SRCS))

# Fichiers objets (même structure dans objs/)
OBJS = $(SRCS_FULL:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)

# Flags d'inclusion
INCLUDES = -I$(INCDIR)

# Couleurs pour l'affichage
GREEN = \033[0;32m
RED = \033[0;31m
YELLOW = \033[0;33m
BLUE = \033[0;34m
NC = \033[0m # No Color

# Règle principale
all: $(NAME)

# Création de l'exécutable
$(NAME): $(OBJS)
	@echo "$(YELLOW)Linking $(NAME)...$(NC)"
	@$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)
	@echo "$(GREEN)✓ $(NAME) created successfully!$(NC)"
	@echo "$(BLUE)Usage: ./$(NAME) config.conf$(NC)"

# Règle pour compiler les fichiers .cpp en .o
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(dir $@)
	@echo "$(YELLOW)Compiling $<...$(NC)"
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@
	@echo "$(GREEN)✓ $@ created$(NC)"

# Nettoyage des fichiers objets
clean:
	@echo "$(RED)Cleaning object files...$(NC)"
	@rm -rf $(OBJDIR)
	@echo "$(GREEN)✓ Object files cleaned$(NC)"

# Nettoyage complet
fclean: clean
	@echo "$(RED)Cleaning executable...$(NC)"
	@rm -f $(NAME)
	@rm -f test_http test_headers
	@echo "$(GREEN)✓ Executable cleaned$(NC)"

# Recompilation complète
re: fclean all

# Test du parsing des headers HTTP
test_headers: $(OBJDIR)/http/HttpHeaders.o
	@echo "$(YELLOW)Creating test for HTTP headers...$(NC)"
	@echo '#include "HttpRequest.hpp"' > test_headers.cpp
	@echo '#include <iostream>' >> test_headers.cpp
	@echo 'bool ft_parse_headers(const std::string& headers_block, HttpRequest& request);' >> test_headers.cpp
	@echo 'int main() {' >> test_headers.cpp
	@echo '    std::string test = "Host: localhost:8080\\r\\nContent-Type: text/html\\r\\n";' >> test_headers.cpp
	@echo '    HttpRequest req;' >> test_headers.cpp
	@echo '    if (ft_parse_headers(test, req)) {' >> test_headers.cpp
	@echo '        std::cout << "✓ Headers parsed: " << req.headers.size() << " headers" << std::endl;' >> test_headers.cpp
	@echo '        std::map<std::string, std::string>::iterator it;' >> test_headers.cpp
	@echo '        for (it = req.headers.begin(); it != req.headers.end(); ++it)' >> test_headers.cpp
	@echo '            std::cout << "  " << it->first << ": " << it->second << std::endl;' >> test_headers.cpp
	@echo '    } else std::cout << "✗ Parse failed: " << req.error_message << std::endl;' >> test_headers.cpp
	@echo '    return 0;' >> test_headers.cpp
	@echo '}' >> test_headers.cpp
	@$(CXX) $(CXXFLAGS) $(INCLUDES) test_headers.cpp $(OBJDIR)/http/HttpHeaders.o -o test_headers
	@rm test_headers.cpp
	@echo "$(GREEN)✓ test_headers created! Run with: ./test_headers$(NC)"

# Test du parsing complet HTTP
test_http: $(OBJDIR)/http/HttpRequestLine.o $(OBJDIR)/http/HttpHeaders.o $(OBJDIR)/http/HttpParser.o
	@echo "$(YELLOW)Creating test for complete HTTP parsing...$(NC)"
	@echo '#include "HttpRequest.hpp"' > test_http.cpp
	@echo '#include <iostream>' >> test_http.cpp
	@echo 'HttpRequest ft_parse_http_request(const std::string& raw_data);' >> test_http.cpp
	@echo 'int main() {' >> test_http.cpp
	@echo '    std::string test = "GET /index.html HTTP/1.1\\r\\nHost: localhost:8080\\r\\nUser-Agent: curl/7.68.0\\r\\n\\r\\n";' >> test_http.cpp
	@echo '    HttpRequest req = ft_parse_http_request(test);' >> test_http.cpp
	@echo '    if (req.is_valid) {' >> test_http.cpp
	@echo '        std::cout << "✓ Complete parsing OK!" << std::endl;' >> test_http.cpp
	@echo '        std::cout << "Method: " << req.method << std::endl;' >> test_http.cpp
	@echo '        std::cout << "URI: " << req.uri << std::endl;' >> test_http.cpp
	@echo '        std::cout << "Headers: " << req.headers.size() << std::endl;' >> test_http.cpp
	@echo '    } else std::cout << "✗ Parse failed: " << req.error_message << std::endl;' >> test_http.cpp
	@echo '    return 0;' >> test_http.cpp
	@echo '}' >> test_http.cpp
	@$(CXX) $(CXXFLAGS) $(INCLUDES) test_http.cpp $(OBJDIR)/http/HttpRequestLine.o $(OBJDIR)/http/HttpHeaders.o $(OBJDIR)/http/HttpParser.o -o test_http
	@rm test_http.cpp
	@echo "$(GREEN)✓ test_http created! Run with: ./test_http$(NC)"

# Afficher les fichiers qui seront compilés
show:
	@echo "$(YELLOW)Source files:$(NC)"
	@echo "$(SRCS_FULL)" | tr ' ' '\n'
	@echo "$(YELLOW)Object files:$(NC)"
	@echo "$(OBJS)" | tr ' ' '\n"
	@echo "$(YELLOW)Include directories:$(NC)"
	@echo "$(INCDIR)"

# Test rapide de compilation
check: all
	@echo "$(BLUE)Testing compilation...$(NC)"
	@echo "$(GREEN)✓ All files compiled successfully!$(NC)"
	@echo "$(BLUE)Ready to run: ./$(NAME) config.conf$(NC)"

# Règles .PHONY
.PHONY: all clean fclean re test_http test_headers show check

# Dépendances des headers pour recompilation automatique
$(OBJDIR)/main/main.o: $(INCDIR)/Server.hpp $(INCDIR)/ServerConfig.hpp
$(OBJDIR)/server/Server.o: $(INCDIR)/Server.hpp
$(OBJDIR)/server/ServerNetwork.o: $(INCDIR)/Server.hpp $(INCDIR)/HttpRequest.hpp
$(OBJDIR)/server/ServerHttp.o: $(INCDIR)/Server.hpp
$(OBJDIR)/server/ServerFiles.o: $(INCDIR)/Server.hpp
$(OBJDIR)/parsing/Parser.o: $(INCDIR)/ServerConfig.hpp
$(OBJDIR)/http/HttpRequestLine.o: $(INCDIR)/HttpRequest.hpp
$(OBJDIR)/http/HttpHeaders.o: $(INCDIR)/HttpRequest.hpp
$(OBJDIR)/http/HttpParser.o: $(INCDIR)/HttpRequest.hpp