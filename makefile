CC = g++
CFLAGS =  -std=c++11

SRCS = main.cpp  dataStream.cpp video.cpp vlc.cpp decoder.cpp
HEADER= code.h dataStream.h video.h vlc.h decoder.h
PROG = mpeg1_player.exe
THREAD =-Wl,-Bstatic -lpthread 


OPENCV = -I"C:\opencv\include" -L"C:\opencv\x64\mingw\lib" -lopencv_core2413 -lopencv_highgui2413



$(PROG) : $(SRCS) $(HEADER)
	$(CC) $(CFLAGS) -o $(PROG) $(SRCS) $(OPENCV) $(THREAD)

