EXCEPTIONS=./main.cpp $(wildcard ./*-test.cpp)
SOURCES=$(filter-out $(EXCEPTIONS),$(wildcard ./*.cpp))
OBJ=$(SOURCES:.cpp=.o)
CFLAGS=-g -std=c++17 -I ./json -I ./ -I ./l3 -I ./ipstack -I ./services

all: app

.cpp.o:
	g++ $(CFLAGS) -c $< -o $@

ipstack.a: 
	cd ipstack && make

json.a: 
	cd json && make

services.a: 
	cd services && make

app: $(OBJ) ipstack.a services.a json.a main.o switch-test.o
	g++ $(OBJ) main.o json/json.a services/services.a ipstack/ipstack.a
	g++ $(OBJ) switch-test.o json/json.a services/services.a ipstack/ipstack.a -o switch-test

clean:
	-rm *.o
	-rm a.out
	-rm *-test
	cd ipstack && make clean
	cd services && make clean
