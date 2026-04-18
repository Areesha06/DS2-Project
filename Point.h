#pragma once

struct Point {
    float x, y;
    int id; // optional identifier

    Point() : x(0), y(0), id(-1) {}
    Point(float x, float y, int id = -1) : x(x), y(y), id(id) {}

    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
};
