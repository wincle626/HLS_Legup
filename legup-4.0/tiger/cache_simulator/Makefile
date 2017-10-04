OPTIONS = -Wall

all: cache_sim parser

cache_sim: cache_sim.cpp cache_sim_utils.o
	g++ $(OPTIONS) -o cache_sim cache_sim.cpp cache_sim_utils.o

cache_sim_utils.o: cache_sim_utils.cpp cache_sim_utils.h
	g++ $(OPTIONS) -c cache_sim_utils.cpp

parser: parser.cpp
	g++ $(OPTIONS) -o parser parser.cpp

clean:
	rm -f *.o cache_sim parser
