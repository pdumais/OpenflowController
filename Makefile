EXCEPTIONS=./main.cpp $(wildcard ./*-test.cpp)
SOURCES=$(filter-out $(EXCEPTIONS),$(wildcard ./*.cpp)) $(wildcard ./appframework/*.cpp) $(wildcard ./management/*.cpp)
OBJ=$(SOURCES:.cpp=.o)
CFLAGS=-g -std=c++17 -I ./Dumaislib/include -I ./ -I ./l3 -I ./ipstack -I ./services
LDFLAGS=Dumaislib/lib/dumaislib.a services/services.a ipstack/ipstack.a -lpthread
all: app

.cpp.o:
	g++ $(CFLAGS) -c $< -o $@

ipstack.a: 
	cd ipstack && make

services.a: 
	cd services && make

app: $(OBJ) ipstack.a services.a main.o switch-test.o
	g++ $(OBJ) main.o $(LDFLAGS)
	g++ $(OBJ) switch-test.o $(LDFLAGS) -o switch-test

clean:
	-rm *.o
	-rm appframework/*.o
	-rm management/*.o
	-rm a.out
	-rm *-test
	cd ipstack && make clean
	cd services && make clean
