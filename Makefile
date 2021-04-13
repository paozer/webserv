############## VARIABLES ##############

NAME		=	webserv

SRCS_PATH	= srcs
OBJS_PATH	= objs

OBJS		= $(addprefix $(OBJS_PATH)/, $(SRCS:.cpp=.o))
SRCS		= Config.cpp		Parsing.cpp\
			  Utils.cpp			main.cpp\
			  Server.cpp		ServerSocket.cpp\
			  webPages.cpp		Logger.cpp

INCS		= 	$(SRCS:.cpp:.hpp)

CC			=	clang++

FLAGS 		=	-Wall -Wextra -g3 -std=c++98

############## RECIPES ##############

all:		$(NAME)

# compile program without unit tests

$(NAME):			$(OBJS)
					$(CC) $(FLAGS) -o $(NAME) $(OBJS)
					@echo "\033[32m > \033[1m$(NAME)\033[0;32m created !\033[0m"
					@echo "execute ./$(NAME) <file.conf> to run webserver"

$(OBJS_PATH)/%.o:	$(SRCS_PATH)/%.cpp
					@$(shell mkdir -p $(OBJS_PATH))
					$(CC) $(FLAGS) -c $< -o $@


clean:
					@rm -rf $(OBJS_PATH)
					@echo "\033[31m > \033[1mobjs/*\033[0;31m delete.\033[0m"

fclean:				clean
					@rm -rf $(NAME)
					@echo "\033[31m > \033[1m$(NAME)\033[0;31m delete.\033[0m"

re:					fclean all

.PHONY:				all fclean clean re