all: mycraft

mycraft: mycraft.cpp
	g++ mycraft.cpp -std=c++0x -g -Wall -o mycraft -lglfw -lglu32 -lglew32 -lopengl32