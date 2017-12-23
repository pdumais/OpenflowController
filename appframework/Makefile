Exceptions=
SOURCES=$(filter-out $(EXCEPTIONS),$(wildcard ./*.cpp))
OBJ=$(SOURCES:.cpp=.o)
LIB=appframework.a
CFLAGS=-g -std=c++17 -I ../

all: lib

.cpp.o:
	g++ $(CFLAGS) -c $< -o $@

lib: $(OBJ)
	ar rcs $(LIB) $(OBJ)

clean:
	-rm *.a
	-rm *.o


