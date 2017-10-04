OPTIONS = -Wall `pkg-config --cflags --libs glib-2.0`

all: mem_access_profiler

mem_access_profiler: mem_access_profiler.cpp utils.o
	g++ $(OPTIONS) -o mem_access_profiler mem_access_profiler.cpp utils.o

utils.o: utils.h utils.cpp
	g++ -c utils.cpp

clean:
	rm -f *.o mem_access_profiler
