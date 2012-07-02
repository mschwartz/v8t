OBJ=main.o global.o threads.o

.cpp.o:
	g++ -I/usr/local/silkjs/src/v8/include -c -o $*.o $*.cpp

all: $(OBJ)
	g++ -o test $(OBJ) -L/usr/local/silkjs/src/v8 -lv8 -lpthread -Wl,-rpath,/usr/local/silkjs/src/v8
