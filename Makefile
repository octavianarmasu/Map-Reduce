build: tema1

tema1: main.cpp tema1.h
	g++ -o tema1 -pthread main.cpp

clean:
	rm tema1
