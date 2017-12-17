EXCEPTIONS=./main.cpp $(wildcard ./*-test.cpp)
SOURCES=$(filter-out $(EXCEPTIONS),$(wildcard ./*.cpp))
OBJ=$(SOURCES:.cpp=.o)
CFLAGS=-g -std=c++17 -I ./json

all: app

.cpp.o:
	g++ $(CFLAGS) -c $< -o $@

json.a: 
	cd json && make

app: $(OBJ) json.a main.o switch-test.o
	g++ $(OBJ) main.o json/json.a
	g++ $(OBJ) switch-test.o json/json.a -o switch-test

clean:
	-rm *.o
	-rm a.out
	-rm *-test

