OBJ=main.o base64.o md5.o global.o console.o threads.o net.o v8.o fs.o process.o linenoise.o editline.o

.cpp.o:
	g++ -I/usr/local/silkjs/src/v8/include -c -o $*.o $*.cpp

all: $(OBJ)
	g++ -o v8t $(OBJ) -L/usr/local/silkjs/src/v8 -lv8 -lpthread -Wl,-rpath,/usr/local/silkjs/src/v8
