CC = /usr/bin/g++
CC_OPTIONS = -I.
LNK_OPTIONS = -lpthread -lrt -ldl

all: server.out example_client.out example_client2.out
	
server.out: server.cpp
	$(CC) $(CCFLAGS) server.cpp libmymed.a -o server.out $(LNK_OPTIONS) 

example_client.out: example_client.cpp
	$(CC) $(CCFLAGS) example_client.cpp libmymed.a -o example_client.out $(LNK_OPTIONS) 

example_client2.out: example_client2.cpp
	$(CC) $(CCFLAGS) example_client2.cpp libmymed.a -o example_client2.out $(LNK_OPTIONS) 

clean:
	rm *.out
