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

    qt.insert(Point(100, 100)); // inside
    qt.insert(Point(150, 150)); // inside
    qt.insert(Point(600, 600)); // outside query
    qt.insert(Point(700, 700)); // outside query

    Rectangle query(125, 125, 75, 75); // center (125,125), size 150x150
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

    // Insert more than QT_CAPACITY points to force subdivision
    for (int i = 0; i < 20; i++) {
        qt.insert(Point(50.f + i * 10, 50.f + i * 10, i));
    }

    assert(qt.count() == 20);
    assert(qt.divided == true); // Should have subdivided
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

    // Both should find same count
    assert(qtFound.size() == naiveFound.size());
    cout << " PASS (" << qtFound.size() << " pts found)\n";
}

// ─── Benchmarks ───────────────────────────────────────────────────────────────

void runBenchmarks() {
    cout << "\n";
    cout << "==========================================================================================================\n";
    cout << "                               QUADTREE vs NAIVE vs UNIFORMGRID BENCHMARK\n";
    cout << "==========================================================================================================\n";

    Rectangle world(500, 500, 500, 500);
    Rectangle query(500, 500, 150, 150); // ~9% of area

    vector<BenchmarkResult> results;

    for (int n : {500, 1000, 5000, 10000, 25000, 50000}) {
        BenchmarkResult r = Benchmark::runRangeQuery(n, world, query);
        results.push_back(r);
    }

    Benchmark::printResults(results);

    cout << "Analysis:\n";
    cout << "  - Quadtree skips irrelevant regions via spatial indexing\n";
    cout << "  - Naive checks every single point → O(n) per query\n";
    cout << "  - Quadtree average case → O(log n + k) where k = results\n\n";
}

// ─── Manual Demo ──────────────────────────────────────────────────────────────

void manualDemo() {
    cout << "========================================\n";
    cout << "    MANUAL DEMO\n";
    cout << "========================================\n";

    Rectangle world(400, 400, 400, 400);
    Quadtree qt(world);

    // Insert some named "map locations"
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

    cout << "Inserting " << locations.size() << " map locations...\n";
    int id = 0;
    for (auto& loc : locations) {
        qt.insert(Point(loc.x, loc.y, id++));
        cout << "  + " << loc.name << " (" << loc.x << ", " << loc.y << ")\n";
    }

    cout << "\nTotal points in quadtree: " << qt.count() << "\n\n";

    // Query a region
    Rectangle query(300, 300, 200, 200);
    vector<Point> found;
    qt.queryRange(query, found);

    cout << "Range query: area around (300,300) with 400x400 box\n";
    cout << "Locations found: " << found.size() << "\n";
    for (auto& p : found)
        cout << "  -> Point at (" << p.x << ", " << p.y << ")\n";

    cout << "\n";
}

// ─── MAIN ─────────────────────────────────────────────────────────────────────

int main() {
    cout << "\n";
    cout << "=========================================\n";
    cout << "  CS201 Quadtree Project - Console Test  \n";
    cout << "=========================================\n\n";

    // Run unit tests
    cout << "--- Unit Tests ---\n";
    testInsertAndCount();
    testRangeQuery();
    testPointQuery();
    testSubdivision();
    testNaiveConsistency();
    cout << "All tests passed!\n\n";

    // Manual demo
    manualDemo();

    // Performance benchmarks
    runBenchmarks();

    return 0;
}
