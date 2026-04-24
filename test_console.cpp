/*
 * CS201 - DS2 Project
 * Console Test + Benchmark (no SFML required)
 * Compile: g++ -std=c++17 -O2 -o test test_console.cpp Quadtree.cpp
 * Run:     ./test
 */

#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <iomanip>
#include <cassert>
#include "Quadtree.h"
#include "NaiveSearch.h"
#include "UniformGrid.h"
#include "Benchmark.h"

using namespace std;
using namespace std::chrono;

// ─── Unit Tests ───────────────────────────────────────────────────────────────

void testInsertAndCount() {
    cout << "[TEST] Insert & Count...";
    Rectangle world(400, 400, 400, 400);
    Quadtree qt(world);
    qt.insert(Point(100, 100));
    qt.insert(Point(200, 200));
    qt.insert(Point(300, 300));
    assert(qt.count() == 3);
    cout << " PASS\n";
}

void testRangeQuery() {
    cout << "[TEST] Range Query...";
    Rectangle world(400, 400, 400, 400);
    Quadtree qt(world);
    qt.insert(Point(100, 100));
    qt.insert(Point(150, 150));
    qt.insert(Point(600, 600));
    qt.insert(Point(700, 700));
    Rectangle query(125, 125, 75, 75);
    vector<Point> found;
    qt.queryRange(query, found);
    assert(found.size() == 2);
    cout << " PASS\n";
}

void testPointQuery() {
    cout << "[TEST] Point Query...";
    Rectangle world(400, 400, 400, 400);
    Quadtree qt(world);
    qt.insert(Point(200, 300));
    qt.insert(Point(500, 500));
    assert(qt.queryPoint(Point(200, 300)) == true);
    assert(qt.queryPoint(Point(999, 999)) == false);
    cout << " PASS\n";
}

void testSubdivision() {
    cout << "[TEST] Subdivision (insert > capacity)...";
    Rectangle world(400, 400, 400, 400);
    Quadtree qt(world);
    for (int i = 0; i < 20; i++)
        qt.insert(Point(50.f + i * 10, 50.f + i * 10, i));
    assert(qt.count() == 20);
    assert(qt.divided == true);
    cout << " PASS\n";
}

void testNaiveConsistency() {
    cout << "[TEST] Quadtree vs Naive consistency...";
    Rectangle world(500, 500, 500, 500);
    Quadtree qt(world);
    NaiveSearch naive;
    mt19937 rng(123);
    uniform_real_distribution<float> dx(10.f, 990.f);
    uniform_real_distribution<float> dy(10.f, 990.f);
    for (int i = 0; i < 200; i++) {
        Point p(dx(rng), dy(rng), i);
        qt.insert(p);
        naive.insert(p);
    }
    Rectangle query(500, 500, 200, 200);
    vector<Point> qtFound;
    qt.queryRange(query, qtFound);
    vector<Point> naiveFound = naive.queryRange(query);
    assert(qtFound.size() == naiveFound.size());
    cout << " PASS (" << qtFound.size() << " pts found)\n";
}

void testUniformGridConsistency() {
    cout << "[TEST] Quadtree vs UniformGrid consistency...";
    Rectangle world(500, 500, 500, 500);
    Quadtree qt(world);
    UniformGrid grid(50.0f);
    mt19937 rng(77);
    uniform_real_distribution<float> dx(10.f, 990.f);
    uniform_real_distribution<float> dy(10.f, 990.f);
    for (int i = 0; i < 200; i++) {
        Point p(dx(rng), dy(rng), i);
        qt.insert(p);
        grid.insert(p);
    }
    Rectangle query(500, 500, 150, 150);
    vector<Point> qtFound;
    qt.queryRange(query, qtFound);
    vector<Point> gridFound = grid.queryRange(query);
    // Both should return the same number of points
    assert(qtFound.size() == gridFound.size());
    cout << " PASS (" << qtFound.size() << " pts found)\n";
}

// ─── Benchmarks ───────────────────────────────────────────────────────────────

void runBenchmarks() {
    Rectangle world(500, 500, 500, 500);
    Rectangle query(500, 500, 150, 150); // ~9% of total area

    vector<BenchmarkResult> results;
    for (int n : {500, 1000, 5000, 10000, 25000, 50000})
        results.push_back(Benchmark::runRangeQuery(n, world, query));

    Benchmark::printResults(results);
}

// ─── Manual Demo ──────────────────────────────────────────────────────────────

void manualDemo() {
    cout << "========================================\n";
    cout << "    MANUAL DEMO — Karachi Map Points\n";
    cout << "========================================\n";

    Rectangle world(400, 400, 400, 400);
    Quadtree qt(world);
    UniformGrid grid(100.0f);

    struct Location { float x, y; const char* name; };
    vector<Location> locations = {
        {100, 100, "Clifton"},
        {200, 150, "Defence"},
        {350, 200, "Gulshan"},
        {500, 400, "Korangi"},
        {600, 600, "Malir"},
        {300, 500, "Landhi"},
        {150, 600, "Lyari"},
        {700, 200, "SITE"},
    };

    cout << "Inserting " << locations.size() << " locations into both Quadtree and UniformGrid...\n";
    int id = 0;
    for (auto& loc : locations) {
        qt.insert(Point(loc.x, loc.y, id));
        grid.insert(Point(loc.x, loc.y, id));
        cout << "  + " << loc.name << " (" << loc.x << ", " << loc.y << ")\n";
        id++;
    }

    Rectangle query(300, 300, 200, 200);
    vector<Point> qtFound;
    qt.queryRange(query, qtFound);
    vector<Point> gridFound = grid.queryRange(query);

    cout << "\nRange query: center (300,300), size 400x400\n";
    cout << "  Quadtree found:    " << qtFound.size()  << " locations\n";
    cout << "  UniformGrid found: " << gridFound.size() << " locations\n\n";
}

// ─── MAIN ─────────────────────────────────────────────────────────────────────

int main() {
    cout << "\n=========================================\n";
    cout << "  CS201 Quadtree Project - Console Test  \n";
    cout << "=========================================\n\n";

    cout << "--- Unit Tests ---\n";
    testInsertAndCount();
    testRangeQuery();
    testPointQuery();
    testSubdivision();
    testNaiveConsistency();
    testUniformGridConsistency();
    cout << "All tests passed!\n\n";

    manualDemo();
    runBenchmarks();

    return 0;
}