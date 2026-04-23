#pragma once
#include <vector>
#include <unordered_map>
#include <cmath>
#include "Point.h"
#include "Rectangle.h"

class UniformGrid {
private:
    float cellSize;

    // Key = hashed cell coordinate
    std::unordered_map<long long, std::vector<Point>> grid;

    long long hash(int x, int y) const {
        return ((long long)x << 32) | (unsigned int)y;
    }

    std::pair<int, int> getCell(const Point& p) const {
        int cx = (int)std::floor(p.x / cellSize);
        int cy = (int)std::floor(p.y / cellSize);
        return {cx, cy};
    }

public:
    UniformGrid(float cellSize) : cellSize(cellSize) {}

    void insert(const Point& p) {
        auto [cx, cy] = getCell(p);
        grid[hash(cx, cy)].push_back(p);
    }

    std::vector<Point> queryRange(const Rectangle& range) const {
        std::vector<Point> found;

        int minX = (int)std::floor(range.left() / cellSize);
        int maxX = (int)std::floor(range.right() / cellSize);
        int minY = (int)std::floor(range.top() / cellSize);
        int maxY = (int)std::floor(range.bottom() / cellSize);

        for (int x = minX; x <= maxX; x++) {
            for (int y = minY; y <= maxY; y++) {
                auto it = grid.find(hash(x, y));
                if (it == grid.end()) continue;

                for (const Point& p : it->second) {
                    if (range.contains(p))
                        found.push_back(p);
                }
            }
        }

        return found;
    }

    bool queryPoint(const Point& target) const {
        auto [cx, cy] = getCell(target);
        auto it = grid.find(hash(cx, cy));

        if (it == grid.end()) return false;

        for (const Point& p : it->second) {
            if (p.x == target.x && p.y == target.y)
                return true;
        }

        return false;
    }

    void clear() {
        grid.clear();
    }
};