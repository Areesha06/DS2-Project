/* Controls:
 *   1–6          Select location type to insert (School/Hospital/etc.)
 *   Left Click   Insert selected location type at cursor
 *   Right Drag   Draw a range query rectangle
 *   G            Generate 500 UNIFORM random points
 *   C            Generate 300 CLUSTERED real-world points (3 hotspots)
 *   R            Reset everything
 *   B            Run standard benchmark (N=500..50000) + write data.js for HTML
 *   K            Benchmark all current on-screen points against all 3 structures
 *   Q            Toggle quadtree grid
 *   ESC          Exit
 */

#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <random>
#include <string>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <cmath>
#include <algorithm>
#include "Quadtree.h"
#include "NaiveSearch.h"
#include "Benchmark.h"

// ── Window dimensions ─────────────────────────────────────────────────────────
const int   WIN_W    = 1100;
const int   WIN_H    = 820;
const float WORLD_X  = WIN_W / 2.f;
const float WORLD_Y  = WIN_H / 2.f;
const float WORLD_HW = WIN_W / 2.f;
const float WORLD_HH = WIN_H / 2.f;

// ── Location type definitions ─────────────────────────────────────────────────
struct LocType { const char* name; sf::Color color; };
const LocType LOC[6] = {
    {"School",     sf::Color(255, 220,  50)},
    {"Hospital",   sf::Color(255,  80,  80)},
    {"Restaurant", sf::Color( 80, 200, 120)},
    {"Office",     sf::Color(100, 160, 255)},
    {"Hotel",      sf::Color(200, 100, 255)},
    {"University", sf::Color(255, 160,  80)},
};

sf::Color getColor(int id) {
    return (id >= 0 && id < 6) ? LOC[id].color : sf::Color(200, 200, 200);
}

// ── Draw all quadtree subdivision lines ───────────────────────────────────────
void drawGrid(sf::RenderWindow& win, const Quadtree& qt) {
    std::vector<Rectangle> rects;
    qt.getAllBoundaries(rects);
    for (const Rectangle& r : rects) {
        sf::RectangleShape s(sf::Vector2f(r.halfW * 2, r.halfH * 2));
        s.setPosition(r.left(), r.top());
        s.setFillColor(sf::Color::Transparent);
        s.setOutlineColor(sf::Color(55, 100, 55, 150));
        s.setOutlineThickness(1.f);
        win.draw(s);
    }
}

// ── Draw a single point as a circle ──────────────────────────────────────────
void drawPt(sf::RenderWindow& win, const Point& p, sf::Color col, float r = 4.f) {
    sf::CircleShape c(r);
    c.setOrigin(r, r);
    c.setPosition(p.x, p.y);
    c.setFillColor(col);
    win.draw(c);
}

// ── Draw UI (stats panel + type selector — ONE legend only) ──────────────────
void drawUI(sf::RenderWindow& win, sf::Font& font,
            int total, int found, bool showGrid,
            long long qtUs, long long naiveUs,
            int selType, int catCounts[6])
{
    auto txt = [&](const std::string& s, float x, float y,
                   sf::Color col = sf::Color(200,200,200), int sz = 13) {
        sf::Text t;
        t.setFont(font); t.setString(s); t.setCharacterSize(sz);
        t.setFillColor(col); t.setPosition(x, y);
        win.draw(t);
    };
    auto panel = [&](float x, float y, float w, float h) {
        sf::RectangleShape r(sf::Vector2f(w, h));
        r.setPosition(x, y);
        r.setFillColor(sf::Color(8, 12, 18, 215));
        r.setOutlineColor(sf::Color(35, 50, 65));
        r.setOutlineThickness(1.f);
        win.draw(r);
    };

    // ── Top-left: stats ──────────────────────────────────────────────────────
    panel(8, 8, 252, 155);
    txt("CS201 - Quadtree Demo", 18, 16, sf::Color(80, 200, 120), 14);
    txt("Total points: " + std::to_string(total), 18, 40);
    txt("In query:     " + std::to_string(found), 18, 60,
        found > 0 ? sf::Color(255, 220, 80) : sf::Color(200, 200, 200));
    if (qtUs >= 0) {
        txt("Quadtree: " + std::to_string(qtUs) + " us",    18,  84, sf::Color(80, 220, 100));
        txt("Naive:    " + std::to_string(naiveUs) + " us", 18, 104, sf::Color(220, 80, 80));
        std::ostringstream ss;
        double sp = (qtUs > 0) ? (double)naiveUs / qtUs : 0.0;
        ss << std::fixed << std::setprecision(1) << sp << "x speedup";
        txt(ss.str(), 18, 124, sf::Color(255, 200, 50));
    }
    txt(showGrid ? "[Q] Grid: ON" : "[Q] Grid: OFF", 18, 140,
        showGrid ? sf::Color(80, 200, 120) : sf::Color(120, 120, 120));

    // ── Query breakdown ──────────────────────────────────────────────────────
    if (found > 0) {
        panel(8, 170, 252, 145);
        txt("Found by type:", 18, 176, sf::Color(140, 140, 140), 11);
        int row = 0;
        for (int i = 0; i < 6; i++) {
            if (catCounts[i] == 0) continue;
            txt(std::string(LOC[i].name) + ": " + std::to_string(catCounts[i]),
                18, 194.f + row * 22.f, LOC[i].color, 13);
            row++;
        }
    }

    // ── Top-right: location type selector (the only legend) ─────────────────
    float sx = WIN_W - 262.f;
    panel(sx - 8, 8, 270, 210);
    txt("INSERT TYPE  [1-6]", sx, 16, sf::Color(80, 200, 120), 13);

    for (int i = 0; i < 6; i++) {
        float ry = 40.f + i * 28.f;
        bool sel = (i == selType);

        if (sel) {
            sf::RectangleShape hi(sf::Vector2f(248, 24));
            hi.setPosition(sx - 4, ry - 2);
            hi.setFillColor(sf::Color(LOC[i].color.r, LOC[i].color.g, LOC[i].color.b, 40));
            hi.setOutlineColor(LOC[i].color);
            hi.setOutlineThickness(1.f);
            win.draw(hi);
        }

        sf::CircleShape dot(5.f);
        dot.setOrigin(5, 5);
        dot.setPosition(sx + 10, ry + 10);
        dot.setFillColor(LOC[i].color);
        win.draw(dot);

        std::string label = "[" + std::to_string(i+1) + "] " + LOC[i].name + (sel ? " <" : "");
        txt(label, sx + 24, ry + 2,
            sel ? LOC[i].color : sf::Color(170, 170, 170), 13);
    }

    // ── Controls (bottom-left) ───────────────────────────────────────────────
    panel(8, WIN_H - 165, 310, 157);
    txt("Left Click : Insert selected type",       18, WIN_H - 159, sf::Color(150,150,150), 12);
    txt("Right Drag : Range query",                18, WIN_H - 139, sf::Color(150,150,150), 12);
    txt("G : 500 uniform random points",           18, WIN_H - 119, sf::Color(150,150,150), 12);
    txt("C : 300 clustered points (3 hotspots)",   18, WIN_H -  99, sf::Color(150,150,150), 12);
    txt("B : Standard benchmark -> data.js+HTML",  18, WIN_H -  79, sf::Color(150,150,150), 12);
    txt("K : Benchmark current on-screen points",  18, WIN_H -  59, sf::Color(150,150,150), 12);
    txt("Q: Toggle grid   R: Reset   ESC: Exit",   18, WIN_H -  39, sf::Color(150,150,150), 12);
}

// ── Run benchmark on whatever points are currently on screen ──────────────────
// Rebuilds all 3 structures from the given points, then times a range query
// that covers the centre-third of the screen (a representative mid-size region).
void runScreenBenchmark(const std::vector<Point>& allPoints) {
    int n = (int)allPoints.size();
    if (n == 0) {
        std::cout << "\n[K] No points on screen — add some first!\n";
        return;
    }

    // Build all three structures from current screen points
    Rectangle world(WORLD_X, WORLD_Y, WORLD_HW, WORLD_HH);
    float cellSize = std::max(WIN_W / std::sqrt((float)n), 5.f);

    Quadtree    qt(world);
    NaiveSearch naive;
    UniformGrid grid(cellSize);
    for (const Point& p : allPoints) {
        qt.insert(p);
        naive.insert(p);
        grid.insert(p);
    }

    // Query covers the centre third of the window
    Rectangle query(WORLD_X, WORLD_Y, WIN_W / 6.f, WIN_H / 6.f);

    // Scale reps so small point counts still get stable timing
    int reps = std::max(200, 2000000 / std::max(n, 1));
    reps = std::min(reps, 5000);

    long long qtT = 0, naiveT = 0, gridT = 0;
    std::vector<Point> found;

    for (int r = 0; r < reps; r++) {
        found.clear();
        auto t1 = std::chrono::high_resolution_clock::now();
        qt.queryRange(query, found);
        auto t2 = std::chrono::high_resolution_clock::now();

        auto t3 = std::chrono::high_resolution_clock::now();
        naive.queryRange(query);
        auto t4 = std::chrono::high_resolution_clock::now();

        auto t5 = std::chrono::high_resolution_clock::now();
        grid.queryRange(query);
        auto t6 = std::chrono::high_resolution_clock::now();

        qtT    += std::chrono::duration_cast<std::chrono::nanoseconds>(t2-t1).count();
        naiveT += std::chrono::duration_cast<std::chrono::nanoseconds>(t4-t3).count();
        gridT  += std::chrono::duration_cast<std::chrono::nanoseconds>(t6-t5).count();
    }

    double qtUs    = (double)qtT    / reps / 1000.0;
    double naiveUs = (double)naiveT / reps / 1000.0;
    double gridUs  = (double)gridT  / reps / 1000.0;

    double qtSpeedup   = (qtUs   > 0) ? naiveUs / qtUs   : 0.0;
    double gridSpeedup = (gridUs > 0) ? naiveUs / gridUs : 0.0;

    const int W = 18;
    std::cout << "\n" << std::string(90, '=') << "\n";
    std::cout << "  [K] ON-SCREEN BENCHMARK  —  N = " << n << " points\n";
    std::cout << "  Query region: centre third of window (" << (int)found.size() << " points found)\n";
    // std::cout << "  Averaged over " << reps << " repetitions\n";
    std::cout << std::string(90, '=') << "\n";
    std::cout << std::left << std::fixed << std::setprecision(3)
              << std::setw(W) << "Structure"
              << std::setw(W) << "Time (us)"
              << std::setw(W) << "Speedup vs Naive"
              << "Complexity\n";
    std::cout << std::string(90, '-') << "\n";
    std::cout << std::setw(W) << "Quadtree"
              << std::setw(W) << qtUs
              << std::setw(W) << qtSpeedup
              << "O(log n + k)\n";
    std::cout << std::setw(W) << "UniformGrid"
              << std::setw(W) << gridUs
              << std::setw(W) << gridSpeedup
              << "O(cells + k)\n";
    std::cout << std::setw(W) << "Naive"
              << std::setw(W) << naiveUs
              << std::setw(W) << "1.0x (baseline)"
              << "O(n)\n";
    std::cout << std::string(90, '=') << "\n\n";
}

// ─────────────────────────────────────────────────────────────────────────────
// MAIN
// ─────────────────────────────────────────────────────────────────────────────
int main() {
    sf::RenderWindow window(
        sf::VideoMode(WIN_W, WIN_H),
        "CS201 - Quadtree Spatial Query System",
        sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(60);

    sf::Font font;
    bool fontLoaded =
        font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf") ||
        font.loadFromFile("C:/Windows/Fonts/arial.ttf") ||
        font.loadFromFile("Arial.ttf");

    Rectangle worldBounds(WORLD_X, WORLD_Y, WORLD_HW, WORLD_HH);
    Quadtree    qt(worldBounds);
    NaiveSearch naive;

    std::vector<Point> allPoints;
    std::vector<Point> queryResult;
    Rectangle queryRect(0, 0, 0, 0);
    bool hasQuery   = false;
    bool showGrid   = true;
    bool dragging   = false;
    sf::Vector2f dragStart;

    int  selType  = 0;
    int  catCounts[6] = {0};
    long long lastQtUs    = -1;
    long long lastNaiveUs = -1;

    std::mt19937 rng(std::random_device{}());

    const std::vector<std::pair<float,float>> HOTSPOTS = {
        {WIN_W * 0.18f, WIN_H * 0.22f},
        {WIN_W * 0.78f, WIN_H * 0.22f},
        {WIN_W * 0.50f, WIN_H * 0.78f},
    };

    while (window.isOpen()) {
        sf::Event ev;
        while (window.pollEvent(ev)) {

            if (ev.type == sf::Event::Closed ||
               (ev.type == sf::Event::KeyPressed &&
                ev.key.code == sf::Keyboard::Escape))
                window.close();

            if (ev.type == sf::Event::KeyPressed) {
                auto k = ev.key.code;

                if (k >= sf::Keyboard::Num1 && k <= sf::Keyboard::Num6)
                    selType = k - sf::Keyboard::Num1;

                if (k == sf::Keyboard::Q)
                    showGrid = !showGrid;

                if (k == sf::Keyboard::R) {
                    qt.clear(); naive.clear();
                    allPoints.clear(); queryResult.clear();
                    hasQuery = false;
                    lastQtUs = lastNaiveUs = -1;
                    std::fill(catCounts, catCounts + 6, 0);
                }

                // G: 500 uniform random points
                if (k == sf::Keyboard::G) {
                    std::uniform_real_distribution<float> dx(20.f, WIN_W - 20.f);
                    std::uniform_real_distribution<float> dy(20.f, WIN_H - 20.f);
                    std::uniform_int_distribution<int>    cat(0, 5);
                    for (int i = 0; i < 500; i++) {
                        Point p(dx(rng), dy(rng), cat(rng));
                        if (qt.insert(p)) { naive.insert(p); allPoints.push_back(p); }
                    }
                }

                // C: 300 clustered points
                if (k == sf::Keyboard::C) {
                    std::normal_distribution<float> spread(0.f, 35.f);
                    std::uniform_int_distribution<int> pickCluster(0, 2);
                    std::uniform_int_distribution<int> cat(0, 5);
                    for (int i = 0; i < 300; i++) {
                        auto& h = HOTSPOTS[pickCluster(rng)];
                        float x = std::max(10.f, std::min((float)WIN_W - 10.f, h.first  + spread(rng)));
                        float y = std::max(10.f, std::min((float)WIN_H - 10.f, h.second + spread(rng)));
                        Point p(x, y, cat(rng));
                        if (qt.insert(p)) { naive.insert(p); allPoints.push_back(p); }
                    }
                }

                // B: standard N-sizes benchmark → writes data.js for HTML
                if (k == sf::Keyboard::B) {
                    std::cout << "\n[B] Running standard benchmark (N = 500 to 50000)...\n";
                    std::vector<int> ns = {500, 1000, 5000, 10000, 25000, 50000};
                    std::vector<BenchmarkResult> uQ, cQ;
                    for (int n : ns) { uQ.push_back(Benchmark::runRangeQuery(n, false)); }
                    for (int n : ns) { cQ.push_back(Benchmark::runRangeQuery(n, true));  }
                    Benchmark::printQueryResults(uQ, "UNIFORM DATA");
                    Benchmark::printQueryResults(cQ, "CLUSTERED DATA");
                    Benchmark::writeDataJS(uQ, cQ, "data.js");
                    std::cout << "[B] Done. Open quadtree_analysis.html in a browser.\n\n";
                }

                // K: benchmark current on-screen points
                if (k == sf::Keyboard::K) {
                    runScreenBenchmark(allPoints);
                }
            }

            // Left click: insert point
            if (ev.type == sf::Event::MouseButtonPressed &&
                ev.mouseButton.button == sf::Mouse::Left) {
                Point p((float)ev.mouseButton.x, (float)ev.mouseButton.y, selType);
                if (qt.insert(p)) { naive.insert(p); allPoints.push_back(p); }
            }

            // Right drag start
            if (ev.type == sf::Event::MouseButtonPressed &&
                ev.mouseButton.button == sf::Mouse::Right) {
                dragStart = {(float)ev.mouseButton.x, (float)ev.mouseButton.y};
                dragging = true; hasQuery = false; queryResult.clear();
            }

            // Right drag release: run range query (quadtree + naive, shown in UI)
            if (ev.type == sf::Event::MouseButtonReleased &&
                ev.mouseButton.button == sf::Mouse::Right && dragging) {
                dragging = false;
                float mx = (float)ev.mouseButton.x;
                float my = (float)ev.mouseButton.y;
                float cx = (dragStart.x + mx) / 2.f;
                float cy = (dragStart.y + my) / 2.f;
                float hw = std::abs(mx - dragStart.x) / 2.f;
                float hh = std::abs(my - dragStart.y) / 2.f;

                if (hw > 2.f && hh > 2.f) {
                    queryRect = Rectangle(cx, cy, hw, hh);
                    hasQuery  = true;
                    queryResult.clear();

                    auto t1 = std::chrono::high_resolution_clock::now();
                    qt.queryRange(queryRect, queryResult);
                    auto t2 = std::chrono::high_resolution_clock::now();
                    lastQtUs = std::chrono::duration_cast<std::chrono::microseconds>(t2-t1).count();

                    std::fill(catCounts, catCounts + 6, 0);
                    for (const Point& p : queryResult)
                        if (p.id >= 0 && p.id < 6) catCounts[p.id]++;

                    auto t3 = std::chrono::high_resolution_clock::now();
                    naive.queryRange(queryRect);
                    auto t4 = std::chrono::high_resolution_clock::now();
                    lastNaiveUs = std::chrono::duration_cast<std::chrono::microseconds>(t4-t3).count();
                }
            }
        }

        // ── RENDER ────────────────────────────────────────────────────────────
        window.clear(sf::Color(14, 18, 24));

        if (showGrid) drawGrid(window, qt);

        for (const Point& p : allPoints)
            drawPt(window, p, getColor(p.id), 4.f);

        if (hasQuery) {
            sf::RectangleShape qbox(sf::Vector2f(queryRect.halfW*2, queryRect.halfH*2));
            qbox.setPosition(queryRect.left(), queryRect.top());
            qbox.setFillColor(sf::Color(255, 220, 50, 25));
            qbox.setOutlineColor(sf::Color(255, 220, 50, 200));
            qbox.setOutlineThickness(2.f);
            window.draw(qbox);
            for (const Point& p : queryResult)
                drawPt(window, p, getColor(p.id), 6.f);
        }

        if (dragging) {
            sf::Vector2i mp = sf::Mouse::getPosition(window);
            float rx = std::min(dragStart.x, (float)mp.x);
            float ry = std::min(dragStart.y, (float)mp.y);
            float rw = std::abs(mp.x - dragStart.x);
            float rh = std::abs(mp.y - dragStart.y);
            sf::RectangleShape dr(sf::Vector2f(rw, rh));
            dr.setPosition(rx, ry);
            dr.setFillColor(sf::Color(255, 220, 50, 18));
            dr.setOutlineColor(sf::Color(255, 220, 50, 130));
            dr.setOutlineThickness(1.f);
            window.draw(dr);
        }

        if (fontLoaded)
            drawUI(window, font, (int)allPoints.size(), (int)queryResult.size(),
                   showGrid, lastQtUs, lastNaiveUs, selType, catCounts);

        window.display();
    }

    return 0;
}