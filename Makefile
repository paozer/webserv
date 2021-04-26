############## VARIABLES ##############

NAME	=	webserv

SRCS 	=	srcs/main.cpp\
			srcs/Configuration/Configuration.cpp\
			srcs/Configuration/Parsing.cpp\
			srcs/Http/Authentication.cpp\
			srcs/Http/ChunkedBody.cpp\
			srcs/Http/Request.cpp\
			srcs/Http/Response.cpp\
			srcs/Server/ConnectionsManagement.cpp\
			srcs/Server/Methods.cpp\
			srcs/Server/Routing.cpp\
			srcs/Server/ServerManagement.cpp\
			srcs/Server/ServerSocket.cpp\
			srcs/Server/Worker.cpp\
			srcs/Utils/Files.cpp\
			srcs/Utils/Logger.cpp\
			srcs/Utils/Utils.cpp

UT_SRCS =	tests/unit_tests/main.cpp\
			tests/unit_tests/Methods.cpp\
			tests/unit_tests/Request.cpp\
			tests/unit_tests/Response.cpp

INCS	= $($(filter-out srcs/main.cpp, $(SRCS)):.cpp=.hpp)
INCS	:= $(addsuffix srcs/Http/Http.hpp, $(INCS))

CC		=	clang++
CFLAGS	=	-fsanitize=address -g3 -Wall -Wextra -std=c++11
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
				@$(CC) $(UT_CFLAGS) -DNO_UNIT_TESTS $(filter-out srcs/main.cpp, $(SRCS)) tests/unit_tests/main.cpp -o $(NAME)
				@echo "\033[32m > test main compiled\033[0m"
				@echo "\033[32m > execute ./$(NAME) to run test main\033[0m"

clean:
				@rm -rf $(NAME).o
				@rm -rf $(NAME).dSYM
				@echo "\033[31m > make clean done\033[0m"

fclean:			clean
				@rm -rf $(NAME)
				@rm -rf log
				@echo "\033[31m > make fclean done\033[0m"

re:				fclean $(NAME)
