The aim of this project is to introduce students to internet protocols and their RFC's. This is achieved by writing an HTTP server.

### Rules
#### Mandatory
This project should be C++98 compliant and the use of external libraries or C++11/17/20 is forbidden. Students should aim to be compliant with the main points of the RFC's 7230 to 7235. Above all the server should be resilient and never hang.

The server has to feature an nginx-like configuration using a file. Ports, hosts, server names, error pages, routes as well as a route's index, accepted methods and autoindex have to be configurable. Headers that are to be implemented can be found in `srcs/Server/Methods.cpp` along with an explanation to when they are parsed from requests or present in responses. The server has to be able to execute CGI based on a configured file extension and binary.

If you find weird and outdated code in the codebase is it most likely the result of the subjects restrictions on data structures, headers and functions that we were allowed or forced to use. A day after we successfully evaluated our project a new subject was released which might be the opportunity to refactore our codebase to a simpler and more efficient web server.

#### Bonus
We implemented workers as threads that are able to process multiple clients in order to balance out the load. The maximal number of workers can be configured using the `nb_workers [digit]` directive.

### Tests
To run the test binary supplied by the school execute `make && ./webserv tests/intra_tests/macos_tester.conf` in one terminal and `./tests/intra_tests/tester` in another. If you're under Linux use the `ubuntu_tester.conf` and `ubuntu_tester` instead.

To run [siege](https://github.com/JoeDog/siege) install it on your system before executing `make && ./webserv tests/intra_tests/macos_tester.conf` in one terminal and `siege -b 127.0.0.1:8080` in another. Siege statistics will be printed when you ctrl-C siege.

Unit tests were written for the code handling requests, responses and request routing. To run the tests execute `make unit_tests && ./webserv`.
In order to test content negotiation we supplied a shell script which can be run by executing `make && ./webserv www/config.conf` in one terminal and `./tests/unit_tests/contentNegotiation.sh` in another.

