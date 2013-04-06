all: mycraft

mycraft: mycraft.cpp
	g++ mycraft.cpp -g -Wall -o mycraft -lglfw -lopengl32 -lglu32 -lglew32