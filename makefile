SHELL := /bin/bash

.PHONY : test1 test2 test3

CC = gcc
GENERIC_FLAGS = -Wall -pedantic -std=gnu99 -g
THREAD_FLAGS = -pthread
INCLUDES = -I./common/

O_FOLDER = build/obj

all: build/server build/client

TARGETS : build/client \
          build/server 

libs/libserverlib.so: objs/filecache.o objs/filesessions.o objs/queue.o objs/configuration.o objs/logFile.o
	$(CC) -shared -o libs/libserverlib.so $^

objs/logFile.o: common/logFile.c
	$(CC) $(INCLUDES) $(GENERIC_FLAGS) $(THREAD_FLAGS) common/logFile.c -c -fPIC -o $@

objs/filecache.o: server/filecache.c
	$(CC) $(INCLUDES) $(GENERIC_FLAGS) $(THREAD_FLAGS) server/filecache.c -c -fPIC -o $@

objs/filesessions.o: server/filesessions.c
	$(CC) $(INCLUDES) $(GENERIC_FLAGS) $(THREAD_FLAGS) server/filesessions.c -c -fPIC -o $@

objs/queue.o: server/queue.c
	$(CC) $(INCLUDES) $(GENERIC_FLAGS) $(THREAD_FLAGS) server/queue.c -c -fPIC -o $@

objs/configuration.o: server/configuration.c
	$(CC) $(INCLUDES) $(GENERIC_FLAGS) $(THREAD_FLAGS) server/configuration.c -c -fPIC -o $@


client_dependencies = $(wildcard client/*.c)

build/client:	$(client_dependencies)
	$(CC) $(INCLUDES) $(GENERIC_FLAGS) $(THREAD_FLAGS) $^ -o ./build/client

server_dependencies = $(wildcard server/*.c)

build/libs/libserverlib.so: libs/libserverlib.so
	cd ./build; mkdir libs; cd ..;
	cp ./libs/libserverlib.so ./build/libs/libserverlib.so

build/server: $(server_dependencies) libs/libserverlib.so build/libs/libserverlib.so
	$(CC) $(INCLUDES) $(GENERIC_FLAGS) $(THREAD_FLAGS) $(server_dependencies) -o ./build/server -Wl,-rpath,./libs -L ./libs -lserverlib  


test1: build/client build/server cleandata
	cp ./build/configuration_test1.txt ./build/configuration.txt;
	cd ./build && ./test1.sh

test2: build/client build/server cleandata
	cp ./build/configuration_test2.txt ./build/configuration.txt;
	cd ./build && ./test2.sh

test3: build/client build/server cleandata
	cp ./build/configuration_test3.txt ./build/configuration.txt;
	cd ./build && ./test3.sh

cleandata:
	rm -f ./build/clientLog.txt;
	rm -f ./reader1/*;
	rm -f ./reader2/*;
	rm -f ./reader3/*;
	rm -f ./espulsi/*;

cleanall:  cleandata
	chmod 777 ./build/*.*;
	rm build/server; rm build/client; rm libs/libserverlib.so
	echo "clean executed"
	
