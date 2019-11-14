default: prog

prog: main.o
	g++ main.o -o prog -L C:\Users\ManiacalDrWatts\Desktop\OpenGL\lib -std=c++11
main.o: main.cpp
	g++ main.cpp -c -I C:\Users\ManiacalDrWatts\Desktop\OpenGL\include -std=c++11
clean:
	rm -rf *.o prog.exe a.exe out.ppm
delout:
	rm -rf out.ppm