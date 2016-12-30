main.cpp : TimeLapseControl.o TimeLapseControl.h TimeLapseControl.cpp
	g++ -I /usr/include -L /usr/lib main.cpp TimeLapseControl.h TimeLapseControl.o -o TimeLapseRun -lueye_api

#TimeLapseControl.o : TimeLapseControl.h TimeLapseControl.cpp
#	g++ -I /usr/include -L /usr/lib -c TimeLapseControl.cpp uEye.h -o TimeLapseControl.o -lueye_api