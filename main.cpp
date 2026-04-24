/*
 * CS201 - Data Structures II
 * Project: Efficient Spatial Query System using Quadtree
 *
 * Controls:
 *   Left Click       - Insert a point
 *   Right Click+Drag - Draw a query rectangle
 *   R                - Reset / clear all points
 *   G                - Generate random points (500)
 *   B                - Run benchmarks
 *   Q                - Toggle quadtree grid visibility
 *   ESC              - Exit
 */

#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <random>
#include <string>
#include <sstream>
#include "Quadtree.h"
#include "NaiveSearch.h"
#include "Benchmark.h"

// ─────────────────────────────────────────────
// Window & world settings
// ─────────────────────────────────────────────
const int   WIN_W   = 1000;
const int   WIN_H   = 800;
const float WORLD_X = WIN_W / 2.0f;
const float WORLD_Y = WIN_H / 2.0f;
const float WORLD_HW = WIN_W / 2.0f;
const float WORLD_HH = WIN_H / 2.0f;

// ─────────────────────────────────────────────
// Helper: draw all quadtree grid lines
// ─────────────────────────────────────────────
void drawQuadtreeGrid(sf::RenderWindow& window, const Quadtree& qt) {
    std::vector<Rectangle> rects;
    qt.getAllBoundaries(rects);

    for (const Rectangle& r : rects) {
        sf::RectangleShape shape(sf::Vector2f(r.halfW * 2, r.halfH * 2));
        shape.setPosition(r.left(), r.top());
        shape.setFillColor(sf::Color::Transparent);
        shape.setOutlineColor(sf::Color(80, 120, 80, 180));
        shape.setOutlineThickness(1.f);
        window.draw(shape);
    }
}

// ─────────────────────────────────────────────
// Helper: draw a circle for a point
// ─────────────────────────────────────────────
void drawPoint(sf::RenderWindow& window, const Point& p,
               sf::Color color = sf::Color(100, 200, 255), float radius = 3.f) {
    sf::CircleShape circle(radius);
    circle.setOrigin(radius, radius);
    circle.setPosition(p.x, p.y);
    circle.setFillColor(color);
    window.draw(circle);
}

// ─────────────────────────────────────────────
// Helper: draw status text
// ─────────────────────────────────────────────
void drawUI(sf::RenderWindow& window, sf::Font& font, int totalPts,
            int found, bool showGrid, long long qtUs, long long naiveUs, int counts[]) {
    // Semi-transparent panel
    sf::RectangleShape panel(sf::Vector2f(280, 200));
    panel.setPosition(10, 10);
    panel.setFillColor(sf::Color(10, 10, 10, 200));
    panel.setOutlineColor(sf::Color(60, 60, 60));
    panel.setOutlineThickness(1.f);
    window.draw(panel);

    auto makeText = [&](const std::string& s, float x, float y, sf::Color col = sf::Color::White) {
        sf::Text t;
        t.setFont(font);
        t.setString(s);
        t.setCharacterSize(14);
        t.setFillColor(col);
        t.setPosition(x, y);
        window.draw(t);
    };

    makeText("CS201 - Quadtree Demo", 20, 18, sf::Color(80, 200, 120));
    makeText("Points: " + std::to_string(totalPts), 20, 42);
    makeText("Found in query: " + std::to_string(found), 20, 62,
             found > 0 ? sf::Color(255, 220, 80) : sf::Color::White);

    if (qtUs >= 0) {
        makeText("Quadtree: " + std::to_string(qtUs) + " us", 20, 86, sf::Color(100, 220, 100));
        makeText("Naive:    " + std::to_string(naiveUs) + " us", 20, 106, sf::Color(220, 100, 100));
        double speedup = (qtUs > 0) ? (double)naiveUs / qtUs : 0.0;
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(1) << speedup << "x speedup";
        makeText(ss.str(), 20, 126, sf::Color(255, 200, 50));
    }

    makeText(showGrid ? "[Q] Grid: ON" : "[Q] Grid: OFF", 20, 150,
             showGrid ? sf::Color(80, 200, 120) : sf::Color(140, 140, 140));

    // Controls help
    sf::RectangleShape help(sf::Vector2f(280, 120));
    help.setPosition(10, WIN_H - 135);
    help.setFillColor(sf::Color(10, 10, 10, 190));
    help.setOutlineColor(sf::Color(50, 50, 50));
    help.setOutlineThickness(1.f);
    window.draw(help);

    makeText("Left Click:   Add point",       20, WIN_H - 128, sf::Color(180, 180, 180));
    makeText("Right Drag:   Range query",     20, WIN_H - 110, sf::Color(180, 180, 180));
    makeText("G: Random pts  R: Reset",       20, WIN_H - 92,  sf::Color(180, 180, 180));
    makeText("B: Benchmark   Q: Grid",        20, WIN_H - 74,  sf::Color(180, 180, 180));
    makeText("ESC: Exit",                     20, WIN_H - 56,  sf::Color(180, 180, 180));

    std::string labels[] = {"Schools", "Hospitals", "Restaurants", "Offices", "Hotels", "Universities"};
    for (int i = 0; i < 6; i++) {
        makeText(labels[i] + ": " + std::to_string(counts[i]), 20, 170 + i * 18);
}
}

// Add this helper function at the top of main.cpp
sf::Color getPointColor(int id) {
    switch (id) {
        case 0: return sf::Color(255, 220, 50);  // Schools    - yellow
        case 1: return sf::Color(255, 80, 80);   // Hospitals  - red
        case 2: return sf::Color(80, 200, 120);  // Restaurants- green
        case 3: return sf::Color(100, 160, 255); // Offices    - blue
        case 4: return sf::Color(200, 100, 255); // Hotels     - purple
        case 5: return sf::Color(255, 160, 80);  // Universities- orange
        default: return sf::Color(255, 255, 255); // white
    }
}

int counts[6] = {0};
// ─────────────────────────────────────────────
// MAIN
// ─────────────────────────────────────────────
int main() {
    sf::RenderWindow window(sf::VideoMode(WIN_W, WIN_H),
                            "CS201 - Quadtree Spatial Query System",
                            sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(60);

    // Load font (fallback to default if not found)
    sf::Font font;
    bool fontLoaded = font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf");
    if (!fontLoaded)
        fontLoaded = font.loadFromFile("C:/Windows/Fonts/arial.ttf");

    // World boundary for quadtree (centered in window)
    Rectangle worldBounds(WORLD_X, WORLD_Y, WORLD_HW, WORLD_HH);
    Quadtree qt(worldBounds);
    NaiveSearch naive;

    std::vector<Point> allPoints; // keep track for re-drawing
    std::vector<Point> queryResult;
    Rectangle queryRect(0, 0, 0, 0);
    bool hasQuery = false;
    bool showGrid = true;
    bool dragging = false;
    sf::Vector2f dragStart;

    int pointId = 0;
    long long lastQtUs    = -1;
    long long lastNaiveUs = -1;

    std::mt19937 rng(std::random_device{}());

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            // ── Close ──────────────────────────────
            if (event.type == sf::Event::Closed ||
               (event.type == sf::Event::KeyPressed &&
                event.key.code == sf::Keyboard::Escape))
                window.close();

            // ── Key presses ────────────────────────
            if (event.type == sf::Event::KeyPressed) {
                // Q - toggle grid
                if (event.key.code == sf::Keyboard::Q)
                    showGrid = !showGrid;

                // R - reset
                if (event.key.code == sf::Keyboard::R) {
                    qt.clear();
                    naive.clear();
                    allPoints.clear();
                    queryResult.clear();
                    hasQuery = false;
                    lastQtUs = lastNaiveUs = -1;
                    pointId = 0;
                }

                // G - generate 500 random points
                if (event.key.code == sf::Keyboard::G) {
                    std::uniform_real_distribution<float> dx(20.f, WIN_W - 20.f);
                    std::uniform_real_distribution<float> dy(20.f, WIN_H - 20.f);
                    std::uniform_int_distribution<int> catDist(0, 5);
                    // Point p(mx, my, catDist(rng)); // instead of pointId++
                    for (int i = 0; i < 500; i++) {
                        Point p(dx(rng), dy(rng), catDist(rng)); // instead of pointId++
                        qt.insert(p);
                        naive.insert(p);
                        allPoints.push_back(p);
                    }
                }

                // B - run benchmarks in console
                if (event.key.code == sf::Keyboard::B) {
                    std::vector<BenchmarkResult> results;
                    Rectangle qr(WIN_W / 2.f, WIN_H / 2.f, WIN_W / 4.f, WIN_H / 4.f);
                    for (int n : {500, 1000, 5000, 10000, 50000}) {
                        results.push_back(Benchmark::runRangeQuery(n, worldBounds, qr));
                    }
                    Benchmark::printResults(results);
                }
            }

            // ── Left Click - insert point ───────────
            if (event.type == sf::Event::MouseButtonPressed &&
                event.mouseButton.button == sf::Mouse::Left) {
                float mx = (float)event.mouseButton.x;
                float my = (float)event.mouseButton.y;
                std::uniform_int_distribution<int> catDist(0, 5);
                Point p(mx, my, catDist(rng)); // instead of pointId++
                // Point p(mx, my, pointId++);
                if (qt.insert(p)) {
                    naive.insert(p);
                    allPoints.push_back(p);
                }
            }

            // ── Right Click Drag - range query ──────
            if (event.type == sf::Event::MouseButtonPressed &&
                event.mouseButton.button == sf::Mouse::Right) {
                dragStart = {(float)event.mouseButton.x, (float)event.mouseButton.y};
                dragging  = true;
                hasQuery  = false;
                queryResult.clear();
            }

            if (event.type == sf::Event::MouseButtonReleased &&
                event.mouseButton.button == sf::Mouse::Right && dragging) {
                dragging = false;
                float mx = (float)event.mouseButton.x;
                float my = (float)event.mouseButton.y;

                float cx = (dragStart.x + mx) / 2.f;
                float cy = (dragStart.y + my) / 2.f;
                float hw = std::abs(mx - dragStart.x) / 2.f;
                float hh = std::abs(my - dragStart.y) / 2.f;

                if (hw > 2.f && hh > 2.f) {
                    queryRect = Rectangle(cx, cy, hw, hh);
                    hasQuery  = true;

                    // Quadtree query timing
                    queryResult.clear();
                    auto t1 = std::chrono::high_resolution_clock::now();
                    qt.queryRange(queryRect, queryResult);

                    auto t2 = std::chrono::high_resolution_clock::now();
                    lastQtUs = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();

                    // int counts[6] = {0}; //counting locations by categories
                    std::fill(std::begin(counts), std::end(counts), 0);
                    for (const Point& p : queryResult) {
                        if (p.id >= 0 && p.id <= 5)
                            counts[p.id]++;
                    }
                    // Naive query timing
                    auto t3 = std::chrono::high_resolution_clock::now();
                    std::vector<Point> nf = naive.queryRange(queryRect);
                    auto t4 = std::chrono::high_resolution_clock::now();
                    lastNaiveUs = std::chrono::duration_cast<std::chrono::microseconds>(t4 - t3).count();
                }
            }
        }
        

        // ── RENDER ──────────────────────────────────
        window.clear(sf::Color(18, 20, 22));

        // Draw grid
        if (showGrid)
            drawQuadtreeGrid(window, qt);

        // Draw all points
        for (const Point& p : allPoints)
            drawPoint(window, p, getPointColor(p.id), 3.f);
            // drawPoint(window, p, sf::Color(100, 160, 255), 3.f);

        // Draw query rectangle + highlighted results
        if (hasQuery) {
            sf::RectangleShape qbox(sf::Vector2f(queryRect.halfW * 2, queryRect.halfH * 2));
            qbox.setPosition(queryRect.left(), queryRect.top());
            qbox.setFillColor(sf::Color(255, 220, 50, 30));
            qbox.setOutlineColor(sf::Color(255, 220, 50, 200));
            qbox.setOutlineThickness(2.f);
            window.draw(qbox);

            for (const Point& p : queryResult)
                drawPoint(window, p, getPointColor(p.id), 3.f);
                // drawPoint(window, p, sf::Color(255, 80, 80), 5.f);
        }

        // Draw live drag rectangle
        if (dragging) {
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            float x = std::min(dragStart.x, (float)mousePos.x);
            float y = std::min(dragStart.y, (float)mousePos.y);
            float w = std::abs(mousePos.x - dragStart.x);
            float h = std::abs(mousePos.y - dragStart.y);

            sf::RectangleShape drag(sf::Vector2f(w, h));
            drag.setPosition(x, y);
            drag.setFillColor(sf::Color(255, 220, 50, 20));
            drag.setOutlineColor(sf::Color(255, 220, 50, 150));
            drag.setOutlineThickness(1.f);
            window.draw(drag);
        }

        // UI overlay
        if (fontLoaded)
            drawUI(window, font, (int)allPoints.size(),
                   (int)queryResult.size(), showGrid, lastQtUs, lastNaiveUs, counts);

        window.display();
    }

    return 0;
}
