main.cpp : TimeLapseControl.o TimeLapseControl.h TimeLapseControl.cpp
	g++ -I /usr/include -L /usr/lib main.cpp TimeLapseControl.h TimeLapseControl.o -o TimeLapseRun -lueye_api -lpthread
