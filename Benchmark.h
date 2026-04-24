// #pragma once
// #include <chrono>
// #include <string>
// #include <iostream>
// #include <iomanip>
// #include <vector>
// #include <random>
// #include <cmath>
// #include "Quadtree.h"
// #include "NaiveSearch.h"
// #include "UniformGrid.h"

// using namespace std::chrono;

// struct BenchmarkResult {
//     long long quadtreeTime;  // microseconds
//     long long gridTime;      // microseconds
//     long long naiveTime;     // microseconds
//     int pointsFound;
//     int datasetSize;
//     std::string queryLabel;
// };

// class Benchmark {
// public:
//     static BenchmarkResult runRangeQuery(int numPoints, Rectangle worldBounds, Rectangle queryRange) {
//         std::mt19937 rng(42);
//         std::uniform_real_distribution<float> distX(worldBounds.left(),  worldBounds.right());
//         std::uniform_real_distribution<float> distY(worldBounds.top(),   worldBounds.bottom());

//         std::vector<Point> allPoints;
//         allPoints.reserve(numPoints);
//         for (int i = 0; i < numPoints; i++)
//             allPoints.emplace_back(distX(rng), distY(rng), i);

//         // Smart cell size heuristic: world_width / sqrt(n)
//         float worldW = worldBounds.halfW * 2.0f;
//         float cellSize = worldW / std::sqrt((float)numPoints);
//         cellSize = std::max(cellSize, 5.0f);

//         Quadtree qt(worldBounds);
//         NaiveSearch naive;
//         UniformGrid grid(cellSize);

//         for (const Point& p : allPoints) {
//             qt.insert(p);
//             naive.insert(p);
//             grid.insert(p);
//         }

//         // Quadtree
//         std::vector<Point> qtFound;
//         auto t1 = high_resolution_clock::now();
//         qt.queryRange(queryRange, qtFound);
//         auto t2 = high_resolution_clock::now();
//         long long qtTime = duration_cast<microseconds>(t2 - t1).count();

//         // Naive
//         auto t3 = high_resolution_clock::now();
//         naive.queryRange(queryRange);
//         auto t4 = high_resolution_clock::now();
//         long long naiveTime = duration_cast<microseconds>(t4 - t3).count();

//         // Uniform Grid
//         auto t5 = high_resolution_clock::now();
//         grid.queryRange(queryRange);
//         auto t6 = high_resolution_clock::now();
//         long long gridTime = duration_cast<microseconds>(t6 - t5).count();

//         BenchmarkResult result;
//         result.quadtreeTime = qtTime;
//         result.naiveTime    = naiveTime;
//         result.gridTime     = gridTime;
//         result.pointsFound  = (int)qtFound.size();
//         result.datasetSize  = numPoints;
//         result.queryLabel   = "Range Query (" + std::to_string(numPoints) + " pts)";
//         return result;
//     }

//     static void printResults(const std::vector<BenchmarkResult>& results) {
//         const int W = 18;
//         std::cout << "\n" << std::string(108, '=') << "\n";
//         std::cout << "              BENCHMARK: Quadtree vs Uniform Grid vs Naive\n";
//         std::cout << std::string(108, '=') << "\n";
//         std::cout << std::left
//                   << std::setw(W) << "Dataset (N)"
//                   << std::setw(W) << "Quadtree (us)"
//                   << std::setw(W) << "UniformGrid (us)"
//                   << std::setw(W) << "Naive (us)"
//                   << std::setw(W) << "QT Speedup"
//                   << std::setw(W) << "Grid Speedup"
//                   << "Pts Found\n";
//         std::cout << std::string(108, '-') << "\n";

//         for (const auto& r : results) {
//             double qtSpeedup   = (r.quadtreeTime > 0) ? (double)r.naiveTime / r.quadtreeTime : 0.0;
//             double gridSpeedup = (r.gridTime > 0)     ? (double)r.naiveTime / r.gridTime     : 0.0;
//             std::cout << std::left
//                       << std::setw(W) << r.datasetSize
//                       << std::setw(W) << r.quadtreeTime
//                       << std::setw(W) << r.gridTime
//                       << std::setw(W) << r.naiveTime
//                       << std::setw(W) << std::fixed << std::setprecision(2) << qtSpeedup
//                       << std::setw(W) << std::fixed << std::setprecision(2) << gridSpeedup
//                       << r.pointsFound << "\n";
//         }
//         std::cout << std::string(108, '=') << "\n\n";
//         std::cout << "Complexity Summary:\n";
//         std::cout << "  Quadtree    - Insert: O(log n)  | Query: O(log n + k)  [adaptive]\n";
//         std::cout << "  UniformGrid - Insert: O(1)      | Query: O(cells + k)  [fixed grid]\n";
//         std::cout << "  Naive       - Insert: O(1)      | Query: O(n)          [brute force]\n\n";
//     }
// };


#pragma once
#include <chrono>
#include <vector>
#include <random>
#include <cmath>
#include <string>
#include <algorithm>

#include "Quadtree.h"
#include "NaiveSearch.h"
#include "UniformGrid.h"

using namespace std::chrono;

enum DatasetType { UNIFORM, CLUSTERED };

struct BenchmarkResult {
    long long quadtreeTime;
    long long gridTime;
    long long naiveTime;
    int datasetSize;
};

class Benchmark {
public:
    static BenchmarkResult runRangeQuery(int n, Rectangle world, Rectangle query, DatasetType type) {
        std::mt19937 rng(42);
        std::vector<Point> pts;
        pts.reserve(n);

        // ─── DATASET GENERATION ─────────────────────────────
        if (type == UNIFORM) {
            std::uniform_real_distribution<float> dx(world.left(), world.right());
            std::uniform_real_distribution<float> dy(world.top(), world.bottom());

            for (int i = 0; i < n; i++)
                pts.emplace_back(dx(rng), dy(rng), i);
        }
        else {
            // Clustered (real-world-like)
            std::uniform_real_distribution<float> cx(world.left(), world.right());
            std::uniform_real_distribution<float> cy(world.top(), world.bottom());
            std::normal_distribution<float> offset(0.0f, 30.0f);

            int clusters = 5;
            std::vector<std::pair<float,float>> centers;

            for (int i = 0; i < clusters; i++)
                centers.push_back({cx(rng), cy(rng)});

            for (int i = 0; i < n; i++) {
                auto& c = centers[i % clusters];
                pts.emplace_back(c.first + offset(rng), c.second + offset(rng), i);
            }
        }

        // ─── STRUCTURES ─────────────────────────────────────
        float cellSize = (world.halfW * 2) / std::sqrt((float)n);
        cellSize = std::max(cellSize, 5.0f);

        Quadtree qt(world);
        NaiveSearch naive;
        UniformGrid grid(cellSize);

        for (auto& p : pts) {
            qt.insert(p);
            naive.insert(p);
            grid.insert(p);
        }

        const int REPEAT = 20;

        auto measure = [&](auto func) {
            long long total = 0;
            for (int i = 0; i < REPEAT; i++) {
                auto t1 = high_resolution_clock::now();
                func();
                auto t2 = high_resolution_clock::now();
                total += duration_cast<microseconds>(t2 - t1).count();
            }
            return total / REPEAT;
        };

        std::vector<Point> found;

        long long qtTime = measure([&]() {
            found.clear();
            qt.queryRange(query, found);
        });

        long long gridTime = measure([&]() {
            grid.queryRange(query);
        });

        long long naiveTime = measure([&]() {
            naive.queryRange(query);
        });

        return {qtTime, gridTime, naiveTime, n};
    }
};