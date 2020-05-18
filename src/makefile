all: server shell
CC = g++

server: server.cpp
	$(CC) -o Server server.cpp  -lssl -lcrypto 


shell: shell.cpp
	$(CC) -o Shell.cgi shell.cpp 

.PHONY: clean,run

clean:
	rm Server Shell

test:
	./Server