CC=g++
CFLAGS=-g -Wall -Wextra -std=c++11 -isystem /usr/local/include -O2
LDFLAGS=-L/usr/local/lib -lglfw -lglu32 -lglew32 -lopengl32 -lpng -lz
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
