
all: Differentiator.o LogEntry.o
	g++ main.cpp Differentiator.o LogEntry.o -o main.o

main.o:
	gcc -c main.cpp Differentiator.cpp -o main.o	
	
LogEntry.o:
	gcc -c LogEntry.cpp -o LogEntry.o
	
clear:
	rm *.o