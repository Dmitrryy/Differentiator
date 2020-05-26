
all: Differentiator.o LogEntry.o 
	g++ main.cpp Differentiator.o LogEntry.o -o Differentiator.exe

Differentiator.o:
	gcc -c Differentiator.cpp -o Differentiator.o	
	
LogEntry.o:
	gcc -c LogEntry.cpp -o LogEntry.o
	
clear:
	del *.o