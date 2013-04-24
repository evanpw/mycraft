CC=g++
CFLAGS=-g -Wall -std=c++11 -isystem /usr/local/include -O3
LDFLAGS=-L/usr/local/lib -lboost_thread-mgw47-mt-1_53 -lboost_system-mgw47-mt-1_53 -lboost_chrono-mgw46-mt-1_53 -ltbb -lglfw -lglu32 -lglew32 -lopengl32 -lpng -lz
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
