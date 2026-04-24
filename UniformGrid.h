#pragma once
#include <vector>
#include <unordered_map>
#include <cmath>
#include "Point.h"
#include "Rectangle.h"

/*
 * Uniform Grid — Fixed-size spatial hashing
 *
 * Space is divided into equal-sized cells. Each point hashes
 * into exactly one cell. Range queries check only the cells
 * that overlap the query rectangle.
 *
 * Complexity:
 *   Insert:      O(1) average
 *   Range Query: O(cells_checked + k)  where k = results
 *   Point Query: O(1) average
 *
 * Weakness: performance degrades when data is heavily clustered
 *           into few cells, or when cell size is poorly chosen.
 */
class UniformGrid {
private:
    float cellSize;
    std::unordered_map<long long, std::vector<Point>> grid;

    long long hashCell(int x, int y) const {
        return ((long long)x << 32) | (unsigned int)y;
    }

    std::pair<int,int> getCell(const Point& p) const {
        int cx = (int)std::floor(p.x / cellSize);
        int cy = (int)std::floor(p.y / cellSize);
        return {cx, cy};
    }

public:
    explicit UniformGrid(float cellSize) : cellSize(cellSize) {}

    void insert(const Point& p) {
        auto [cx, cy] = getCell(p);
        grid[hashCell(cx, cy)].push_back(p);
    }

    std::vector<Point> queryRange(const Rectangle& range) const {
        std::vector<Point> found;

        int minCX = (int)std::floor(range.left()   / cellSize);
        int maxCX = (int)std::floor(range.right()  / cellSize);
        int minCY = (int)std::floor(range.top()    / cellSize);
        int maxCY = (int)std::floor(range.bottom() / cellSize);

        for (int x = minCX; x <= maxCX; x++) {
            for (int y = minCY; y <= maxCY; y++) {
                auto it = grid.find(hashCell(x, y));
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
        auto it = grid.find(hashCell(cx, cy));
        if (it == grid.end()) return false;
        for (const Point& p : it->second) {
            if (p.x == target.x && p.y == target.y)
                return true;
        }
        return false;
    }

    void clear() { grid.clear(); }

    int count() const {
        int total = 0;
        for (const auto& [key, vec] : grid)
            total += (int)vec.size();
        return total;
    }
};