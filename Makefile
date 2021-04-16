############## VARIABLES ##############

NAME	=	webserv

SRCS 	=	srcs/main.cpp\
			srcs/Server.cpp\
			srcs/ServerSocket.cpp\
			srcs/Config.cpp\
			srcs/Parsing.cpp\
			srcs/Http/Request.cpp\
			srcs/Http/Response.cpp\
			srcs/Http/ChunkedBody.cpp\
			srcs/Logger.cpp\
			srcs/webPages.cpp\
			srcs/Utils.cpp

UT_SRCS =	tests/main.cpp\
			tests/ChunkedBody.cpp\
			tests/Request.cpp\
			tests/Response.cpp

INCS	= $($(filter-out srcs/main.cpp, $(SRCS)):.cpp=.hpp)
INCS	:= $(addsuffix srcs/Http/Http.hpp, $(INCS))

CC		=	clang++
CFLAGS	=	-fsanitize=address -g3 -Wall -Wextra -std=c++98
UT_CFLAGS =	-fsanitize=address -g3 -Wall -Wextra -std=c++11


############## RECIPES ##############

# compile server
$(NAME):		Makefile $(SRCS) $(INCS)
				@echo "\033[33m > compiling...\033[0m"
				@$(CC) $(CFLAGS) $(SRCS) -o $(NAME)
				@echo "\033[32m > server compiled\033[0m"
				@echo "\033[32m > execute ./$(NAME) to run server\033[0m"

# compile program with unit tests
unit_tests: 	Makefile $(filter-out srcs/main.cpp, $(SRCS)) $(UT_SRCS) $(INCS)
				@echo "\033[33m > compiling...\033[0m"
				@$(CC) $(UT_CFLAGS) $(filter-out srcs/main.cpp, $(SRCS)) $(UT_SRCS) -o $(NAME)
				@echo "\033[32m > unit tests compiled\033[0m"
				@echo "\033[32m > execute ./$(NAME) to run all unit tests\033[0m"

# compile program with test main (tests/main.cpp)
no_unit_tests: 	Makefile $(filter-out srcs/main.cpp, $(SRCS)) $(UT_SRCS) $(INCS)
				@echo "\033[33m > compiling...\033[0m"
				@$(CC) $(UT_CFLAGS) -DNO_UNIT_TESTS $(filter-out srcs/main.cpp, $(SRCS)) tests/main.cpp -o $(NAME)
				@echo "\033[32m > test main compiled\033[0m"
				@echo "\033[32m > execute ./$(NAME) to run test main\033[0m"

clean:
				@rm -rf $(NAME).o
				@rm -rf $(NAME).dSYM
				@echo "\033[31m > make clean done\033[0m"

fclean:			clean
				@rm -rf $(NAME)
				@echo "\033[31m > make fclean done\033[0m"

re:				fclean $(NAME)
