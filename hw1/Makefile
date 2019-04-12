CC = g++

CFLAGS = -Wall -O3

OBJS = Simple2DTransformation.o LoadShaders.o

run : $(OBJS)
	$(CC) -o $@ $(OBJS) -lglut -lGLU -lGL -lGLEW -lm

Simple2DTransformation.o : Simple2DTransformation.cpp

LoadShaders.o : Shaders/LoadShaders.cpp
	$(CC) -c Shaders/LoadShaders.cpp

clean:
	rm run $(OBJS)