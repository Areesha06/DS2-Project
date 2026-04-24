#pragma once
#include "Point.h"

// rectangle used for quadtree boundaries and range queries
struct Rectangle {
    float x, y;  // center
    float halfW, halfH; // half dimensions

    Rectangle() : x(0), y(0), halfW(0), halfH(0) {} // defualt constructor
    Rectangle(float x, float y, float halfW, float halfH) : x(x), y(y), halfW(halfW), halfH(halfH) {} // constructor to set values

    // check if this rectangle contain a point
    bool contains(const Point& p) const {
        return (p.x >= x - halfW and p.x <= x + halfW and p.y >= y - halfH and p.y <= y + halfH); 
    }

    // other is right of this or other is left of this or other is above this or other is below this

    // check if this rectangle intersect another
    bool intersects(const Rectangle& other) const {
        return !(other.x - other.halfW > x + halfW or other.x + other.halfW < x - halfW or other.y - other.halfH > y + halfH or other.y + other.halfH < y - halfH);
    }

    float left() const {  // left recatngle boundary
        return x - halfW; 
    }
    float right() const {  // right recatngle boundary
        return x + halfW; 
    }
    float top() const { // top recatngle boundary
        return y - halfH; 
    }
    float bottom() const { // bottom recatngle boundary
        return y + halfH; 
    }
};
