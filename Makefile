all: mycraft

mycraft: mycraft.cpp
	g++ mycraft.cpp -std=c++0x -g -Wall -o mycraft -L. -lglfw -lglu32 -lglew32 -lSOIL -lopengl32