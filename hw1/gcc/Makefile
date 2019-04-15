CC = g++

CFLAGS = -Wall -O3 -Werror

OBJS = HW1_S20120085.o LoadShaders.o
SHOWOBJS = showcase.o LoadShaders.o

run : $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) -lglut -lGLU -lGL -lGLEW -lm

showcase: $(SHOWOBJS)
	$(CC) $(CFLAGS) -o $@ $(SHOWOBJS) -lglut -lGLU -lGL -lGLEW -lm

HW1_S20120085.o : HW1_S20120085.cpp
	$(CC) $(CFLAGS) -c HW1_S20120085.cpp

LoadShaders.o : Shaders/LoadShaders.cpp
	$(CC) $(CFLAGS) -Wno-unused-result -c Shaders/LoadShaders.cpp

showcase.o : showcase.cpp
	$(CC) $(CFLAGS) -c showcase.cpp

.PHONY: clean cleanshowcase
clean:
	rm run $(OBJS)

cleanshowcase:
	rm showcase $(SHOWOBJS)