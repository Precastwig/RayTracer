all: build

Release: build

fast:
	g++ -std=c++0x -Iinclude -Isrc -Ofast main.cpp src/octree.cpp src/shape.cpp src/scene.cpp src/bitmapsave.cpp -o main -lrt -lboost_system -lboost_chrono -lnlopt -lm

build:
	g++ -g -std=c++0x -Iinclude -Isrc -Og main.cpp src/octree.cpp src/shape.cpp src/scene.cpp src/bitmapsave.cpp -o main -lboost_system -lboost_chrono -lnlopt -lm
clean:
	rm main.o
	rm main
