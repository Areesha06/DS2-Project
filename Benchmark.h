#pragma once
#include <chrono>
#include <string>
#include <iostream>
#include <iomanip>
#include <vector>
#include <random>
#include <cmath>
#include "Quadtree.h"
#include "NaiveSearch.h"
#include "UniformGrid.h"

using namespace std::chrono;

// Number of times each query is repeated and averaged
// to eliminate timing noise/spikes at small N
const int BENCH_REPEATS = 50;

struct BenchmarkResult {
    long long quadtreeTime;  // microseconds (averaged)
    long long gridTime;      // microseconds (averaged)
    long long naiveTime;     // microseconds (averaged)
    int pointsFound;
    int datasetSize;
};

class Benchmark {
public:
    static BenchmarkResult runRangeQuery(int numPoints, Rectangle worldBounds, Rectangle queryRange) {
        std::mt19937 rng(42); // fixed seed
        std::uniform_real_distribution<float> distX(worldBounds.left(), worldBounds.right());
        std::uniform_real_distribution<float> distY(worldBounds.top(),  worldBounds.bottom());

        std::vector<Point> allPoints;
        allPoints.reserve(numPoints);
        for (int i = 0; i < numPoints; i++)
            allPoints.emplace_back(distX(rng), distY(rng), i);

        // Smart cell size: world_width / sqrt(n)
        float worldW  = worldBounds.halfW * 2.0f;
        float cellSize = worldW / std::sqrt((float)numPoints);
        cellSize = std::max(cellSize, 5.0f);

        Quadtree    qt(worldBounds);
        NaiveSearch naive;
        UniformGrid grid(cellSize);

        for (const Point& p : allPoints) {
            qt.insert(p);
            naive.insert(p);
            grid.insert(p);
        }

        // ── Averaged timing: run BENCH_REPEATS times, sum, divide ──
        long long qtTotal = 0, naiveTotal = 0, gridTotal = 0;
        std::vector<Point> qtFound;

        for (int r = 0; r < BENCH_REPEATS; r++) {
            // Quadtree
            qtFound.clear();
            auto t1 = high_resolution_clock::now();
            qt.queryRange(queryRange, qtFound);
            auto t2 = high_resolution_clock::now();
            qtTotal += duration_cast<microseconds>(t2 - t1).count();

            // Naive
            auto t3 = high_resolution_clock::now();
            naive.queryRange(queryRange);
            auto t4 = high_resolution_clock::now();
            naiveTotal += duration_cast<microseconds>(t4 - t3).count();

            // Grid
            auto t5 = high_resolution_clock::now();
            grid.queryRange(queryRange);
            auto t6 = high_resolution_clock::now();
            gridTotal += duration_cast<microseconds>(t6 - t5).count();
        }

        BenchmarkResult result;
        result.quadtreeTime = qtTotal   / BENCH_REPEATS;
        result.naiveTime    = naiveTotal / BENCH_REPEATS;
        result.gridTime     = gridTotal  / BENCH_REPEATS;
        result.pointsFound  = (int)qtFound.size();
        result.datasetSize  = numPoints;
        return result;
    }

    static void printResults(const std::vector<BenchmarkResult>& results) {
        const int W = 18;
        std::cout << "\n" << std::string(110, '=') << "\n";
        std::cout << "         BENCHMARK: Quadtree vs Uniform Grid vs Naive  (avg over "
                  << BENCH_REPEATS << " runs)\n";
        std::cout << std::string(110, '=') << "\n";
        std::cout << std::left
                  << std::setw(W) << "Dataset (N)"
                  << std::setw(W) << "Quadtree (us)"
                  << std::setw(W) << "UniformGrid (us)"
                  << std::setw(W) << "Naive (us)"
                  << std::setw(W) << "QT Speedup"
                  << std::setw(W) << "Grid Speedup"
                  << "Pts Found\n";
        std::cout << std::string(110, '-') << "\n";

        for (const auto& r : results) {
            double qtSpeedup   = (r.quadtreeTime > 0) ? (double)r.naiveTime / r.quadtreeTime : 0.0;
            double gridSpeedup = (r.gridTime > 0)     ? (double)r.naiveTime / r.gridTime     : 0.0;
            std::cout << std::left
                      << std::setw(W) << r.datasetSize
                      << std::setw(W) << r.quadtreeTime
                      << std::setw(W) << r.gridTime
                      << std::setw(W) << r.naiveTime
                      << std::setw(W) << std::fixed << std::setprecision(2) << qtSpeedup
                      << std::setw(W) << std::fixed << std::setprecision(2) << gridSpeedup
                      << r.pointsFound << "\n";
        }
        std::cout << std::string(110, '=') << "\n\n";
        std::cout << "Complexity:\n";
        std::cout << "  Quadtree    - Insert: O(log n) | Query: O(log n + k) [adaptive subdivision]\n";
        std::cout << "  UniformGrid - Insert: O(1)     | Query: O(cells + k) [fixed cell size]\n";
        std::cout << "  Naive       - Insert: O(1)     | Query: O(n)         [brute force]\n\n";
    }
};