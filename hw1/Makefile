CC = g++

CFLAGS = -Wall -Wno-unused-result -O3

OBJS = Simple2DTransformation.o LoadShaders.o

run : $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) -lglut -lGLU -lGL -lGLEW -lm

Simple2DTransformation.o : Simple2DTransformation.cpp
	$(CC) $(CFLAGS) -c Simple2DTransformation.cpp

LoadShaders.o : Shaders/LoadShaders.cpp
	$(CC) $(CFLAGS) -c Shaders/LoadShaders.cpp

clean:
	rm run $(OBJS)