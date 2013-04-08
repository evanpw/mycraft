all: mycraft

mycraft: mycraft.cpp
	g++ mycraft.cpp -std=c++0x -g -Wall -o mycraft -L. -I/usr/local/include -L/usr/local/lib -lglfw -lglu32 -lglew32 -lopengl32 -lpng -lz