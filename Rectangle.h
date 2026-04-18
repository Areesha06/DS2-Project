#pragma once
#include "Point.h"

// Axis-Aligned Bounding Box used for quadtree boundaries and range queries
struct Rectangle {
    float x, y;        // center
    float halfW, halfH; // half-dimensions

    Rectangle() : x(0), y(0), halfW(0), halfH(0) {}
    Rectangle(float x, float y, float halfW, float halfH)
        : x(x), y(y), halfW(halfW), halfH(halfH) {}

    // Does this rectangle contain a point?
    bool contains(const Point& p) const {
        return (p.x >= x - halfW && p.x <= x + halfW &&
                p.y >= y - halfH && p.y <= y + halfH);
    }

    // Does this rectangle intersect another?
    bool intersects(const Rectangle& other) const {
        return !(other.x - other.halfW > x + halfW ||
                 other.x + other.halfW < x - halfW ||
                 other.y - other.halfH > y + halfH ||
                 other.y + other.halfH < y - halfH);
    }

    float left()   const { return x - halfW; }
    float right()  const { return x + halfW; }
    float top()    const { return y - halfH; }
    float bottom() const { return y + halfH; }
};
