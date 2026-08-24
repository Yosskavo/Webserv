# --- MAKEFILE COLORS ---
C_GREEN  = \033[0;32m
C_YELLOW = \033[0;33m
C_BLUE   = \033[0;34m
C_RED    = \033[0;31m
C_RESET  = \033[0m
C_PURPLE = \033[1;35m

SHELL := /bin/zsh

NAME = webserv
CC = c++
CFLAGS = -g3 -Wall -Wextra -Werror -std=c++98 -I .

SRC = main.cpp \
      server.cpp \
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
      utils/print.cpp \
      utils/split.cpp \
      utils/trim.cpp  \
	  utils/join.cpp

OBJ = $(SRC:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJ)
	@echo "$(C_BLUE) 󰌷 Linking $(NAME)...$(C_RESET)"
	@$(CC) $(CFLAGS) $(OBJ) -o $(NAME)
	@echo "$(C_GREEN) 󰙴 Done compiling!$(C_RESET)"

%.o: %.cpp webserv.h server.hpp
	@echo "$(C_YELLOW) 󰔚 Compiling $<...$(C_RESET)"
	@$(CC) $(CFLAGS) -c $< -o $@

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
