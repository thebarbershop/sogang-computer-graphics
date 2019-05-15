CC = g++

CFLAGS = -Wall -Werror -O3

OBJS = HW3_S20120085.o LoadShaders.o

.PHONY: clean run $(OBJS)

run : $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) -lglut -lGLU -lGL -lGLEW -lm

HW3_S20120085.o : HW3_S20120085.cpp
	$(CC) $(CFLAGS) -c HW3_S20120085.cpp

LoadShaders.o : Shaders/LoadShaders.cpp
	$(CC) $(CFLAGS) -c Shaders/LoadShaders.cpp

clean:
	rm run $(OBJS)