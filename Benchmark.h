#pragma once
#include <chrono>
#include <string>
#include <iostream>
#include <iomanip>
#include <vector>
#include <random>
#include "Quadtree.h"
#include "NaiveSearch.h"
#include "UniformGrid.h"

using namespace std::chrono;

// Returns microseconds elapsed
struct BenchmarkResult {
    long long quadtreeTime;   // microseconds
    long long naiveTime;      // microseconds
    long long gridTime;
    int pointsFound;
    int datasetSize;
    std::string queryLabel;
};

class Benchmark {
public:
    static BenchmarkResult runRangeQuery(int numPoints, Rectangle worldBounds, Rectangle queryRange) {
        // --- Generate random points ---
        std::mt19937 rng(42); // fixed seed for reproducibility
        std::uniform_real_distribution<float> distX(worldBounds.left(), worldBounds.right());
        std::uniform_real_distribution<float> distY(worldBounds.top(), worldBounds.bottom());

        std::vector<Point> allPoints;
        allPoints.reserve(numPoints);
        for (int i = 0; i < numPoints; i++) {
            allPoints.emplace_back(distX(rng), distY(rng), i);
        }

        // --- Build Quadtree ---
        Quadtree qt(worldBounds);
        for (const Point& p : allPoints)
            qt.insert(p);

        // --- Build Naive ---
        NaiveSearch naive;
        for (const Point& p : allPoints)
            naive.insert(p);

        // --- Build Uniform Grid ---
        UniformGrid grid(20.0f); // ⚠️ tune this if needed
        for (const Point& p : allPoints)
            grid.insert(p);

        // --- Benchmark Quadtree Query ---
        std::vector<Point> qtFound;
        auto t1 = high_resolution_clock::now();
        qt.queryRange(queryRange, qtFound);
        auto t2 = high_resolution_clock::now();
        long long qtTime = duration_cast<microseconds>(t2 - t1).count();

        // --- Benchmark Naive Query ---
        auto t3 = high_resolution_clock::now();
        std::vector<Point> naiveFound = naive.queryRange(queryRange);
        auto t4 = high_resolution_clock::now();
        long long naiveTime = duration_cast<microseconds>(t4 - t3).count();

        // --- Benchmark Grid Query ---
        auto t5 = high_resolution_clock::now();
        std::vector<Point> gridFound = grid.queryRange(queryRange);
        auto t6 = high_resolution_clock::now();
        long long gridTime = duration_cast<microseconds>(t6 - t5).count();

        BenchmarkResult result;
        result.quadtreeTime = qtTime;
        result.naiveTime = naiveTime;
        result.gridTime = gridTime;
        result.pointsFound = (int)qtFound.size();
        result.datasetSize = numPoints;
        result.queryLabel = "Range Query (" + std::to_string(numPoints) + " pts)";

        return result;
    }

    static void printResults(const std::vector<BenchmarkResult>& results) {
        std::cout << "\n==========================================================================================================\n";
        std::cout << "                                  PERFORMANCE BENCHMARK RESULTS    \n";
        std::cout << "==========================================================================================================\n";
        std::cout << std::left
                  << std::setw(20) << "Dataset Size"
                  << std::setw(20) << "Quadtree (us)"
                  << std::setw(20) << "Grid (us)" 
                  << std::setw(20) << "Naive (us)"
                  << std::setw(15) << "Speedup"
                  << std::setw(12) << "Pts Found"
                  << "\n";
        std::cout << std::string(107, '-') << "\n";

        for (const auto& r : results) {
            double speedup = (r.quadtreeTime > 0)
                ? (double)r.naiveTime / r.quadtreeTime : 0.0;

            std::cout << std::left
                      << std::setw(20) << r.datasetSize
                      << std::setw(20) << r.quadtreeTime
                      << std::setw(20) << r.gridTime  
                      << std::setw(20) << r.naiveTime
                      << std::setw(15) << std::fixed << std::setprecision(2) << speedup
                      << std::setw(12) << r.pointsFound
                      << "\n";
        }
        std::cout << "===========================================================================================================\n\n";
    }
};
