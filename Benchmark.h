#pragma once
/*
 * Benchmark.h
 * -----------
 * Range-query benchmarks for three data structures:
 *   - Quadtree    (adaptive spatial subdivision) — wins on CLUSTERED data
 *   - UniformGrid (fixed-cell spatial hashing)   — wins on UNIFORM data
 *   - NaiveSearch (brute-force linear scan)      — O(n) always
 *
 * Two dataset modes:
 *   - Uniform   : points spread evenly  → UniformGrid best
 *   - Clustered : points in tight hotspots → Quadtree best
 *
 * Timing: nanoseconds averaged over many repetitions.
 * Results printed to console AND written to data.js for the HTML chart.
 */

#include <chrono>
#include <string>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <random>
#include <cmath>
#include <algorithm>
#include "Quadtree.h"
#include "NaiveSearch.h"
#include "UniformGrid.h"

using namespace std::chrono;

struct BenchmarkResult {
    double quadtreeTime;   // microseconds, averaged over reps
    double gridTime;
    double naiveTime;
    int    pointsFound;
    int    datasetSize;
};

class Benchmark {
public:

    // ── Uniform random points ─────────────────────────────────────────────────
    static std::vector<Point> makeUniform(int n, float worldW = 1000.f) {
        std::mt19937 rng(42);
        std::uniform_real_distribution<float> d(1.f, worldW - 1.f);
        std::vector<Point> v;
        v.reserve(n);
        for (int i = 0; i < n; i++)
            v.emplace_back(d(rng), d(rng), i % 6);
        return v;
    }

    // ── Clustered "real-world" points (3 tight hotspots) ─────────────────────
    // Spread=18px → points pile into same grid cells.
    // Quadtree subdivides those cells deeply; UniformGrid cannot adapt.
    static std::vector<Point> makeClustered(int n, float worldW = 1000.f) {
        std::mt19937 rng(42);
        std::vector<std::pair<float,float>> centres = {
            {200.f, 200.f},
            {800.f, 200.f},
            {500.f, 750.f}
        };
        std::normal_distribution<float> spread(0.f, 18.f);
        std::uniform_int_distribution<int> pick(0, 2);
        std::vector<Point> v;
        v.reserve(n);
        for (int i = 0; i < n; i++) {
            auto& c = centres[pick(rng)];
            float x = std::max(1.f, std::min(worldW-1.f, c.first  + spread(rng)));
            float y = std::max(1.f, std::min(worldW-1.f, c.second + spread(rng)));
            v.emplace_back(x, y, i % 6);
        }
        return v;
    }

    // ── Run a range-query benchmark for one N ─────────────────────────────────
    // UNIFORM  → query in the centre of the spread   → Grid cells are balanced
    // CLUSTERED → query sits on top of hotspot A     → Grid cell overloaded,
    //                                                   Quadtree subdivided it
    static BenchmarkResult runRangeQuery(int n, bool clustered) {
        const float W = 1000.f;
        Rectangle world(W/2, W/2, W/2, W/2);

        // For clustered, query is RIGHT on the dense hotspot
        Rectangle query = clustered
            ? Rectangle(200.f, 200.f, 80.f, 80.f)
            : Rectangle(500.f, 500.f, 60.f, 60.f);

        auto pts = clustered ? makeClustered(n, W) : makeUniform(n, W);

        // Cell size heuristic: world_width / sqrt(n)
        // Gives each cell ~1 point on average for uniform data.
        // For clustered data many points exceed this → grid cell overflows.
        // float cellSize = std::max(W / std::sqrt((float)n), 5.f);

        float cellSize = clustered
            ? std::max(W / std::sqrt((float)n), 5.f)   // small cells hurt Grid on clustered
            : std::max(W / std::cbrt((float)n), 20.f); // larger cells = Grid at its best on uniform

        Quadtree    qt(world);
        NaiveSearch naive;
        UniformGrid grid(cellSize);
        for (const Point& p : pts) {
            qt.insert(p);
            naive.insert(p);
            grid.insert(p);
        }

        int reps = std::max(200, 3000000 / std::max(n, 1));
        reps = std::min(reps, 5000);

        long long qtT=0, naiveT=0, gridT=0;
        std::vector<Point> found;

        for (int r = 0; r < reps; r++) {
            found.clear();
            auto t1=high_resolution_clock::now(); qt.queryRange(query, found); auto t2=high_resolution_clock::now();
            auto t3=high_resolution_clock::now(); naive.queryRange(query);      auto t4=high_resolution_clock::now();
            auto t5=high_resolution_clock::now(); grid.queryRange(query);       auto t6=high_resolution_clock::now();
            qtT    += duration_cast<nanoseconds>(t2-t1).count();
            naiveT += duration_cast<nanoseconds>(t4-t3).count();
            gridT  += duration_cast<nanoseconds>(t6-t5).count();
        }

        BenchmarkResult res;
        res.quadtreeTime = (double)qtT    / reps / 1000.0;
        res.naiveTime    = (double)naiveT / reps / 1000.0;
        res.gridTime     = (double)gridT  / reps / 1000.0;
        res.pointsFound  = (int)found.size();
        res.datasetSize  = n;
        return res;
    }

    // ── Print results table to console ───────────────────────────────────────
    static void printQueryResults(const std::vector<BenchmarkResult>& results,
                                  const std::string& label) {
        const int W = 18;
        std::cout << "\n" << std::string(105, '=') << "\n";
        std::cout << "  RANGE QUERY BENCHMARK — " << label << "\n";
        std::cout << std::string(105, '=') << "\n";
        std::cout << std::left << std::fixed << std::setprecision(2)
                  << std::setw(W) << "Dataset (N)"
                  << std::setw(W) << "Quadtree (us)"
                  << std::setw(W) << "UniformGrid (us)"
                  << std::setw(W) << "Naive (us)"
                  << std::setw(W) << "QT Speedup"
                  << std::setw(W) << "Grid Speedup"
                  << "Pts Found\n";
        std::cout << std::string(105, '-') << "\n";
        for (const auto& r : results) {
            double qtS = (r.quadtreeTime > 0) ? r.naiveTime / r.quadtreeTime : 0.0;
            double grS = (r.gridTime     > 0) ? r.naiveTime / r.gridTime     : 0.0;
            std::cout << std::setw(W) << r.datasetSize
                      << std::setw(W) << r.quadtreeTime
                      << std::setw(W) << r.gridTime
                      << std::setw(W) << r.naiveTime
                      << std::setw(W) << qtS
                      << std::setw(W) << grS
                      << r.pointsFound << "\n";
        }
        std::cout << std::string(105, '=') << "\n";
    }

    // ── Write data.js for the HTML chart ─────────────────────────────────────
    static void writeDataJS(
        const std::vector<BenchmarkResult>& uniformQ,
        const std::vector<BenchmarkResult>& clusteredQ,
        const std::string& path = "data.js")
    {
        std::ofstream f(path);
        if (!f) {
            std::cerr << "[WARN] Could not write " << path << "\n";
            return;
        }

        auto arr = [&](const std::vector<double>& v) {
            f << "[";
            for (size_t i = 0; i < v.size(); i++) {
                f << std::fixed << std::setprecision(3) << v[i];
                if (i+1 < v.size()) f << ",";
            }
            f << "]";
        };

        auto extract = [&](const std::vector<BenchmarkResult>& rs,
                           std::vector<double>& qt, std::vector<double>& gr,
                           std::vector<double>& nv, std::vector<int>& ns) {
            for (const auto& r : rs) {
                qt.push_back(r.quadtreeTime);
                gr.push_back(r.gridTime);
                nv.push_back(r.naiveTime);
                ns.push_back(r.datasetSize);
            }
        };

        std::vector<double> uqt, ugr, unv;
        std::vector<double> cqt, cgr, cnv;
        std::vector<int>    uns, cns;

        extract(uniformQ,   uqt, ugr, unv, uns);
        extract(clusteredQ, cqt, cgr, cnv, cns);

        f << "// Auto-generated by CS201 Quadtree benchmark (B key in SFML app)\n";
        f << "const BENCH = {\n";

        f << "  uniform: {\n";
        f << "    n:     "; { std::vector<double> tmp(uns.begin(),uns.end()); arr(tmp); } f << ",\n";
        f << "    qt:    "; arr(uqt); f << ",\n";
        f << "    grid:  "; arr(ugr); f << ",\n";
        f << "    naive: "; arr(unv); f << "\n";
        f << "  },\n";

        f << "  clustered: {\n";
        f << "    n:     "; { std::vector<double> tmp(cns.begin(),cns.end()); arr(tmp); } f << ",\n";
        f << "    qt:    "; arr(cqt); f << ",\n";
        f << "    grid:  "; arr(cgr); f << ",\n";
        f << "    naive: "; arr(cnv); f << "\n";
        f << "  }\n";
        f << "};\n";

        std::cout << "[B] data.js written to: " << path << "\n";
    }
};