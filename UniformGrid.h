#pragma once
#include <vector>
#include <unordered_map>
#include <cmath>
#include "Point.h"
#include "Rectangle.h"

//Fixed size spatial hashing. Space is divided into equal sized cells. Each point hashes into exactly one cell. Range queries check only the cells that overlap the query rectangle. Complexity of Insert: O(1) average, Range Query: O(cells_checked + k)  where k = results, Point Query: O(1) average. However, performance degrades when data is heavily clustered into few cells

class UniformGrid {
private:
    float cellSize;     // Hash map storing grid cells: key = encoded (cellX, cellY) and value = list of points in that cell
    std::unordered_map<long long, std::vector<Point>> grid;

    long long hashCell(int x, int y) const { // Converts 2D grid coordinates into a single unique hash key
        return ((long long)x << 32) | (unsigned int)y;
    }

    std::pair<int,int> getCell(const Point& p) const { //Converts a point into its corresponding grid cell coordinates
        int cx = (int)std::floor(p.x / cellSize);
        int cy = (int)std::floor(p.y / cellSize);
        return {cx, cy};
    }

public:
    explicit UniformGrid(float cellSize) : cellSize(cellSize) {} // constructor 

    void insert(const Point& p) { // Insert a point into grid cell
        auto [cx, cy] = getCell(p);
        grid[hashCell(cx, cy)].push_back(p);
    }

    std::vector<Point> queryRange(const Rectangle& range) const {   // Returns all points inside a rectangular region, only checks grid cells that overlap the rectangle
        std::vector<Point> found;

        int minCX = (int)std::floor(range.left()/cellSize);
        int maxCX = (int)std::floor(range.right()/cellSize);
        int minCY = (int)std::floor(range.top()/cellSize);
        int maxCY = (int)std::floor(range.bottom()/cellSize);

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

    bool queryPoint(const Point& target) const {  // Checks if point exists in grid, searches only its corresponding cell
        auto [cx, cy] = getCell(target);
        auto it = grid.find(hashCell(cx, cy));
        if (it == grid.end()) return false;
        for (const Point& p : it->second) {
            if (p.x == target.x && p.y == target.y)
                return true;
        }
        return false;
    }

    void clear() { grid.clear(); } // clears all stored points from the grid, used in reset function 

    int count() const {  // Returns total number of points stored in the grid, used for benchmarking comparision 
        int total = 0;
        for (const auto& [key, vec] : grid)
            total += (int)vec.size();
        return total;
    }
};