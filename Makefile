CFLAGS=-g -Wall -Wextra -std=c++11 -isystem /usr/local/include -O2
LDFLAGS=-lpng -lz
ifeq ($(OS),Windows_NT)
    LDFLAGS=-L/usr/local/lib -lglfw -lglu32 -lglew32 -lopengl32
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
    	CC=g++    
    	LDFLAGS+= -lglfw -lGL -lGLEW
    endif
    ifeq ($(UNAME_S),Darwin)
    	CC=g++-4.8
        LDFLAGS+=-lglfw -framework OpenGL -framework Cocoa -framework IOkit -lGLEW
    endif
endif
SOURCES=$(wildcard *.cpp)
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=mycraft

all: $(EXECUTABLE)

clean:
	rm -f *.o mycraft

$(EXECUTABLE): $(OBJECTS)
	echo $(LDFLAGS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

.cpp.o:
	$(CC) -c $(CFLAGS) $< -o $@
