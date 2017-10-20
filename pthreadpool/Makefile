CC := g++
INCLUDE = -I./
LIBS := -lpthread 

CXXFLAGS := -g -Wall -D_REENTRANT -std=c++11

all : server client
PROGRAM1 := server
PROGRAM2 := client

OBJECT1 := thread_pool.o task.o server.o
OBJECT2 := client.o

.cpp.o:
	$(CC) $(CXXFLAGS) $(INCLUDE) -c $<

server : $(OBJECT1)
	$(CC) -o $(PROGRAM1) $(OBJECT1) $(LIBS)

client : $(OBJECT2)
	$(CC) -o $(PROGRAM2) $(OBJECT2) $(LIBS)

.PHONY : clean
clean:
	-rm -f $(OBJECT1) $(OBJECT2) $(PROGRAM1) $(PROGRAM2)

#
