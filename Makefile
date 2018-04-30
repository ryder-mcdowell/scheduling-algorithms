all: hw4

hw4: hw4.o
	g++ -o hw4 hw4.o

hw4.o: hw4.cpp
	g++ -c hw4.cpp

clean:
	rm -f hw4.o
