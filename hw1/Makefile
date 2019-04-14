CC = g++

CFLAGS = -Wall -O3 -Werror

OBJS = Simple2DTransformation.o LoadShaders.o

run : $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) -lglut -lGLU -lGL -lGLEW -lm

Simple2DTransformation.o : Simple2DTransformation.cpp
	$(CC) $(CFLAGS) -c Simple2DTransformation.cpp

LoadShaders.o : Shaders/LoadShaders.cpp
	$(CC) $(CFLAGS) -Wno-unused-result -c Shaders/LoadShaders.cpp

clean:
	rm run $(OBJS)