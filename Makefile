# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::   #
#    Makefile                                           :+:      :+:    :+:   #
#                                                     +:+ +:+         +:+     #
#    By: jacob <jacob@student.42.fr>                +#+  +:+       +#+        #
#                                                 +#+#+#+#+#+   +#+           #
#    Created: 2025/06/12 00:00:00 by jacob             #+#    #+#             #
#    Updated: 2025/06/12 00:00:00 by jacob            ###   ########.fr       #
#                                                                              #
# **************************************************************************** #

# Program name
NAME		= webserv

# Directories
SRCDIR		= srcs
INCDIR		= includes
OBJDIR		= objs

# Source files
MAIN_SRCS	= $(SRCDIR)/main/main.cpp
SERVER_SRCS	= $(SRCDIR)/server/Server.cpp
PARSING_SRCS= $(SRCDIR)/parsing/Parser.cpp

# All source files
SRCS		= $(MAIN_SRCS) $(SERVER_SRCS) $(PARSING_SRCS)

# Object files
OBJS		= $(SRCS:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)

# Compiler and flags
CXX			= c++
CXXFLAGS	= -Wall -Wextra -Werror -std=c++98
INCLUDES	= -I$(INCDIR)

# Colors for pretty output
RED			= \033[0;31m
GREEN		= \033[0;32m
YELLOW		= \033[0;33m
BLUE		= \033[0;34m
PURPLE		= \033[0;35m
CYAN		= \033[0;36m
WHITE		= \033[0;37m
RESET		= \033[0m

# Rules
all: $(NAME)

$(NAME): $(OBJS)
	@echo "$(CYAN)Linking $(NAME)...$(RESET)"
	@$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)
	@echo "$(GREEN)✅ $(NAME) compiled successfully!$(RESET)"

# Create object files and directories
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(dir $@)
	@echo "$(YELLOW)Compiling $<...$(RESET)"
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Create obj directory
$(OBJDIR):
	@mkdir -p $(OBJDIR)

# Clean object files
clean:
	@echo "$(RED)Cleaning object files...$(RESET)"
	@rm -rf $(OBJDIR)
	@echo "$(RED)🗑️  Object files cleaned!$(RESET)"

# Clean everything
fclean: clean
	@echo "$(RED)Cleaning $(NAME)...$(RESET)"
	@rm -f $(NAME)
	@echo "$(RED)🗑️  $(NAME) cleaned!$(RESET)"

# Rebuild everything
re: fclean all

# Run the server
run: $(NAME)
	@echo "$(PURPLE)🚀 Starting WebServ on port 8080...$(RESET)"
	@./$(NAME)

# Run with custom port
run8081: $(NAME)
	@echo "$(PURPLE)🚀 Starting WebServ on port 8081...$(RESET)"
	@./$(NAME) 8081

# Test with curl
test: $(NAME)
	@echo "$(BLUE)🧪 Testing server (make sure it's running)...$(RESET)"
	@curl -s http://localhost:8080 || echo "$(RED)❌ Server not responding$(RESET)"

# Debug compilation (with debug symbols)
debug: CXXFLAGS += -g -DDEBUG
debug: $(NAME)
	@echo "$(CYAN)🐛 Debug version compiled!$(RESET)"

# Show help
help:
	@echo "$(WHITE)Available targets:$(RESET)"
	@echo "  $(GREEN)all$(RESET)     - Compile the project"
	@echo "  $(GREEN)clean$(RESET)   - Remove object files"
	@echo "  $(GREEN)fclean$(RESET)  - Remove object files and binary"
	@echo "  $(GREEN)re$(RESET)      - Rebuild everything"
	@echo "  $(GREEN)run$(RESET)     - Compile and run on port 8080"
	@echo "  $(GREEN)run8081$(RESET) - Compile and run on port 8081"
	@echo "  $(GREEN)test$(RESET)    - Test the server with curl"
	@echo "  $(GREEN)debug$(RESET)   - Compile with debug symbols"
	@echo "  $(GREEN)help$(RESET)    - Show this help"

# Declare phony targets
.PHONY: all clean fclean re run run8081 test debug help