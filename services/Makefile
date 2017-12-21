SOURCES=$(filter-out $(EXCEPTIONS),$(wildcard ./*.cpp))
OBJ=$(SOURCES:.cpp=.o)
CFLAGS=-g -std=c++17  -I ./ -I ../ipstack -I../

all: app

.cpp.o:
	g++ $(CFLAGS) -c $< -o $@

app: $(OBJ) 
	$(AR) rcs services.a $(OBJ)	

clean:
	-rm *.o
	-rm *.a

