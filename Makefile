CC := g++

INCLUDE := -I./

LIBS := -lpthread

CXXFLAGS := -g -Wall -D_REENTRANT

all : server client

TARGET1 := server
TARGET2 := client


OBJECT1 := socket.o epoll.o ftp_server.o server.o
OBJECT2 := socket.o ftp_client.o client.o

.cpp.o:
	$(CC)  $(CXXFLAGS) $(INCLUDE) -c $<

$(TARGET1) : $(OBJECT1)
	$(CC) -o  $(TARGET1) $(OBJECT1)
$(TARGET2) : $(OBJECT2)
	$(CC) -o $(TARGET2) $(OBJECT2)

.PHONY : clean
clean:
	-rm -f $(TARGET1) $(TARGET2) $(OBJECT1) $(OBJECT2)
