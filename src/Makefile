UNAME := $(shell uname -s)

OBJ=main.o base64.o md5.o global.o console.o threads.o binary.o net.o v8.o fs.o process.o linenoise.o editline.o async.o http.o

%.o: %.cpp SilkJS.h
	g++ -g -I/usr/local/silkjs/src/v8/include -c -o $*.o $*.cpp

all: $(OBJ)
	g++ -g -o v8t $(OBJ) -L/usr/local/silkjs/src/v8 -lv8 -lpthread -Wl,-rpath,/usr/local/silkjs/src/v8
