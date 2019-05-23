CC = g++

NAME = HW3_S120085
CFLAGS = -Wall -Werror -O3
OBJS = $(NAME).o LoadShaders.o

.PHONY: clean run $(OBJS)

run : $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) -lglut -lGLU -lGL -lGLEW -lm

$(NAME).o : $(NAME).cpp
	$(CC) $(CFLAGS) -c $(NAME).cpp

LoadShaders.o : Shaders/LoadShaders.cpp
	$(CC) $(CFLAGS) -c Shaders/LoadShaders.cpp

clean:
	rm run $(OBJS)