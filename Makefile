############## VARIABLES ##############

NAME		=	webserv

SRCS 		=	srcs/main.cpp\
				srcs/Server.cpp\
				srcs/ServerSocket.cpp

INCS		= 	$(SRCS:.cpp:.hpp)

CC			=	clang++

FLAGS 		=	-fsanitize=address -g3 -Wall -Wextra -std=c++98

############## RECIPES ##############

all:		$(NAME)

# compile program without unit tests
$(NAME):		Makefile $(SRCS) $(INCS)
				@$(CC) $(FLAGS) $(SRCS) -o $(NAME)
				@echo "execute ./$(NAME) to run webserver"

clean:
				@rm -rf $(NAME).o
				@rm -rf $(NAME).dSYM
				@echo "make clean done..."

fclean:			clean
				@rm -rf $(NAME)
				@echo "make fclean done..."

re:				fclean all

.PHONY:		all fclean clean re
