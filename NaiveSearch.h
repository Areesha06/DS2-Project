#pragma once
#include <vector>
#include "Point.h"
#include "Rectangle.h"

// Naive brute-force spatial search — O(n) per query
// Used for performance comparison against the quadtree approach
class NaiveSearch {
public:
    std::vector<Point> points;

    void insert(const Point& p) {
        points.push_back(p);
    }

    // Range query: iterate over ALL points — O(n)
    std::vector<Point> queryRange(const Rectangle& range) const {
        std::vector<Point> found;
        for (const Point& p : points) {
            if (range.contains(p))
                found.push_back(p);
        }
        return found;
    }

    // Point query: linear scan — O(n)
    bool queryPoint(const Point& target) const {
        for (const Point& p : points) {
            if (p.x == target.x && p.y == target.y)
                return true;
        }
        return false;
    }

    void clear() {
        points.clear();
    }

    int count() const {
        return (int)points.size();
    }
};
