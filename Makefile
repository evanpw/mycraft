CC=g++-4.8
CFLAGS=-g -Wall -Wextra -std=c++11 -isystem /usr/local/include -O2
LDFLAGS=-lglfw -framework OpenGL -framework Cocoa -framework IOkit -lGLEW -lpng -lz
SOURCES=$(wildcard *.cpp)
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=mycraft

all: $(EXECUTABLE)

clean:
	rm -f *.o mycraft

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

.cpp.o:
	$(CC) -c $(CFLAGS) $< -o $@
