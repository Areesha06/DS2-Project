#pragma once

struct Point {
    float x, y;
    int id; // optional identifier 0:schools, 1:hospitals, 2:restaurants, 3: office, 4: hotels, 5: universities

    Point() : x(0), y(0), id(-1) {}
    Point(float x, float y, int id = -1) : x(x), y(y), id(id) {}

    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
};
