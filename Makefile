

DIR_INC = ./inc
DIR_SRC = ./src
DIR_OBJ = ./obj
DIR_BIN = ./

CC = gcc
CXX = g++
CFLAGS = -g -D_REENTRANT -Wall -std=c++11 -I${DIR_INC}
LDFLAGS = -lpthread

#wildcard 对 c文件进行展开
SRCS = $(wildcard ${DIR_SRC}/*.c) $(wildcard ${DIR_SRC}/*.cpp)
#去除路径信息
dir=$(notdir $(SRCS))
# 把后缀.c .cpp替换成 .o
OBJS = $(patsubst %.c, ${DIR_OBJ}/%.o, $(patsubst %.cpp, ${DIR_OBJ}/%.o,$(dir))  )


TAR_SERVER = server
TAR_CLIENT = client

BIN_TAR_SERVER = ${DIR_BIN}/${TAR_SERVER}
BIN_TAR_CLIENT = ${DIR_BIN}/${TAR_CLIENT}

BIN_TARGET = ${DIR_BIN}/${TAR_SERVER} ${DIR_BIN}/${TAR_CLIENT}



TARGET = ${BIN_TAR_SERVER} ${BIN_TAR_CLIENT}

all: $(TARGET)

${BIN_TAR_SERVER} : ${DIR_OBJ}/server.o $(filter-out ${DIR_OBJ}/client.o,$(OBJS)) 
	$(CXX) $(CFLAGS) -o $@ $^ $(LDFLAGS) ./pthreadpool/*.o
${BIN_TAR_CLIENT} :  ${DIR_OBJ}/client.o $(filter-out ${DIR_OBJ}/server.o,$(OBJS))
	$(CXX) $(CFLAGS) -o $@ $^ $(LDFLAGS) ./pthreadpool/*.o



${DIR_OBJ}/%.o : ${DIR_SRC}/%.c
	$(CC) -c $(CFLAGS)  $^ -o $@
${DIR_OBJ}/%.o : ${DIR_SRC}/%.cpp
	$(CXX) -c $(CFLAGS) $^ -o $@    

.PHONY : clean

clean :
	rm -f ${OBJS}
	rm -f ${TARGET}
	@echo "clean done"

# install:
#         mv Excute excute; cp -f ordermisd ../bin/;
