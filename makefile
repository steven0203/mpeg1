CC = g++
CFLAGS =  -std=c++11

SRCS = main.cpp  dataStream.cpp video.cpp vlc.cpp decoder.cpp
HEADER= code.h dataStream.h video.h vlc.h decoder.h
PROG = mpeg1_player
THREAD =-lpthread 


OPENCV = `pkg-config --cflags --libs opencv4`



$(PROG) : $(SRCS) $(HEADER)
	$(CC) $(CFLAGS) -o $(PROG) $(SRCS) $(OPENCV) $(THREAD)

