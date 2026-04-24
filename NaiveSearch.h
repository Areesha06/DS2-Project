#pragma once
#include <vector>
#include "Point.h"
#include "Rectangle.h"

// Naive brute force spatial search : O(n) per query
// Used for performance comparison against the quadtree approach
class NaiveSearch { //  store all points in a single array (no partitioning), every query scans all points linearly
public:
    std::vector<Point> points;     // list to store points

    void insert(const Point& p) { // inserting point in constant time
        points.push_back(p);
    }

    // iterate over all points: O(n) for range query
    std::vector<Point> queryRange(const Rectangle& range) const {
        std::vector<Point> found;
        for (const Point& p : points) {
            if (range.contains(p))
                found.push_back(p);
        }
        return found;
    }

    // linear scan is O(n), for point query as well
    bool queryPoint(const Point& target) const {
        for (const Point& p : points) {
            if (p.x == target.x and p.y == target.y)
                return true;
        }
        return false;
    }

    void clear() { // clear points 
        points.clear();
    }

    int count() const { // returns total no. of points, used for comparision
        return (int)points.size();
    }
};
