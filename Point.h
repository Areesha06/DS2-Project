#pragma once // Ensures this header file is included only once during compilation. Prevents duplicate definition errors


// Simple 2D point struct with x, y, id
struct Point {
    float x, y; // x, y coordinates of the point (2D space)
    int id; 

    Point() : x(0), y(0), id(-1) {}
    Point(float x, float y, int id = -1) : x(x), y(y), id(id) {}

    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
};
