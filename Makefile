# --- MAKEFILE COLORS ---
C_GREEN  = \033[0;32m
C_YELLOW = \033[0;33m
C_BLUE   = \033[0;34m
C_RED    = \033[0;31m
C_RESET  = \033[0m
C_PURPLE = \033[1;35m

SHELL := /bin/zsh

NAME = webserv
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -I . -g

SRC = main.cpp \
      server/Server_core.cpp server/Server_read.cpp server/Server_route.cpp \
      server/Server_static.cpp server/Server_modify.cpp server/Server_cgi.cpp server/Server_response.cpp \
      struct/location.cpp \
      parce/file/error.cpp \
      parce/file/listen.cpp \
      parce/file/location.cpp \
      parce/file/parce.cpp \
      parce/file/return.cpp \
      parce/file/server.cpp \
      parce/request/cookies.cpp \
      parce/request/headers.cpp \
      parce/request/method.cpp \
      parce/cgi/cgi.cpp \
      utils/numbers_handle.cpp \
      \
      utils/split.cpp \
      utils/trim.cpp  \
	  utils/join.cpp

OBJ = $(SRC:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJ)
	@echo "$(C_BLUE) 󰌷 Linking $(NAME)...$(C_RESET)"
	@$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)
	@echo "$(C_GREEN) 󰙴 Done compiling!$(C_RESET)"

%.o: %.cpp webserv.h server/server.hpp
	@echo "$(C_YELLOW) 󰔚 Compiling $<...$(C_RESET)"
	@$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@echo "$(C_RED) 󰮘 Removing object files...$(C_RESET)"
	@rm -fv $(OBJ)

fclean: clean
	@echo "$(C_RED)  Removing executable $(NAME)...$(C_RESET)"
	@rm -fv $(NAME)

re: fclean all

run : re all 
	@echo "$(C_PURPLE) Runing the excutable $(NAME)...$(C_RESET)"
	@./$(NAME)

.PHONY: all clean fclean re
