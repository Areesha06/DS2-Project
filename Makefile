# CS201 Quadtree Project - Makefile
# Requirements: g++, SFML (libsfml-dev)
#
# Ubuntu/Debian install: sudo apt install libsfml-dev
# Windows (MinGW):       Download SFML from https://sfml-dev.org

CXX      = g++
CXXFLAGS = -std=c++17 -O2 -Wall
SFML     = -lsfml-graphics -lsfml-window -lsfml-system

TARGET   = quadtree_demo
SRCS     = main.cpp Quadtree.cpp

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRCS) $(SFML)

clean:
	rm -f $(TARGET)

run: all
	./$(TARGET)

.PHONY: all clean run
