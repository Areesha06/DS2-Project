/*
 *  - Data Structures II
 * Project: Efficient Spatial Query System using Quadtree
 *
 * Controls:
 *   Left Click       - Insert selected location type at cursor
 *   Right Click+Drag - Draw a range query rectangle
 *   1-6              - Select location type to insert
 *   R                - Reset / clear all points
 *   G                - Generate 500 random points
 *   B                - Run benchmarks (console)
 *   Q                - Toggle quadtree grid
 *   ESC              - Exit
 */

#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <random>
#include <string>
#include <sstream>
#include <iomanip>
#include <chrono>
#include "Quadtree.h"
#include "NaiveSearch.h"
#include "Benchmark.h"

const int   WIN_W   = 1100;
const int   WIN_H   = 820;
const float WORLD_X = WIN_W / 2.0f;
const float WORLD_Y = WIN_H / 2.0f;
const float WORLD_HW = WIN_W / 2.0f;
const float WORLD_HH = WIN_H / 2.0f;

// ─── Location types ────────────────────────────────────────────────────────
// id: 0=School 1=Hospital 2=Restaurant 3=Office 4=Hotel 5=University
struct LocType { const char* name; sf::Color color; };
const LocType LOC_TYPES[6] = {
    {"School",     sf::Color(255, 220,  50)},  // yellow
    {"Hospital",   sf::Color(255,  80,  80)},  // red
    {"Restaurant", sf::Color( 80, 200, 120)},  // green
    {"Office",     sf::Color(100, 160, 255)},  // blue
    {"Hotel",      sf::Color(200, 100, 255)},  // purple
    {"University", sf::Color(255, 160,  80)},  // orange
};

sf::Color getPointColor(int id) {
    if (id >= 0 && id < 6) return LOC_TYPES[id].color;
    return sf::Color(200, 200, 200);
}

// ─── Draw quadtree grid ────────────────────────────────────────────────────
void drawQuadtreeGrid(sf::RenderWindow& win, const Quadtree& qt) {
    std::vector<Rectangle> rects;
    qt.getAllBoundaries(rects);
    for (const Rectangle& r : rects) {
        sf::RectangleShape shape(sf::Vector2f(r.halfW * 2, r.halfH * 2));
        shape.setPosition(r.left(), r.top());
        shape.setFillColor(sf::Color::Transparent);
        shape.setOutlineColor(sf::Color(60, 110, 60, 160));
        shape.setOutlineThickness(1.f);
        win.draw(shape);
    }
}

// ─── Draw a point ─────────────────────────────────────────────────────────
void drawPoint(sf::RenderWindow& win, const Point& p, sf::Color col, float r = 4.f) {
    sf::CircleShape c(r);
    c.setOrigin(r, r);
    c.setPosition(p.x, p.y);
    c.setFillColor(col);
    win.draw(c);
}

// ─── Draw UI panel ────────────────────────────────────────────────────────
void drawUI(sf::RenderWindow& win, sf::Font& font,
            int totalPts, int found, bool showGrid,
            long long qtUs, long long naiveUs,
            int selectedType, int catCounts[6])
{
    auto txt = [&](const std::string& s, float x, float y,
                   sf::Color col = sf::Color(200,200,200), int sz = 13) {
        sf::Text t; t.setFont(font); t.setString(s);
        t.setCharacterSize(sz); t.setFillColor(col); t.setPosition(x, y);
        win.draw(t);
    };
    auto panel = [&](float x, float y, float w, float h) {
        sf::RectangleShape r(sf::Vector2f(w, h));
        r.setPosition(x, y);
        r.setFillColor(sf::Color(8, 12, 18, 210));
        r.setOutlineColor(sf::Color(40, 55, 70));
        r.setOutlineThickness(1.f);
        win.draw(r);
    };

    // ── Top-left info panel ─────────────────────────────────────────────────
    panel(8, 8, 250, 190);
    txt(" - Quadtree Demo", 18, 16, sf::Color(80, 200, 120), 14);
    txt("Total points: " + std::to_string(totalPts), 18, 40);
    txt("In query:     " + std::to_string(found), 18, 60,
        found > 0 ? sf::Color(255, 220, 80) : sf::Color(200,200,200));
    if (qtUs >= 0) {
        txt("Quadtree: " + std::to_string(qtUs) + " us", 18, 84, sf::Color(80, 220, 100));
        txt("Naive:    " + std::to_string(naiveUs) + " us", 18, 104, sf::Color(220, 80, 80));
        std::ostringstream ss;
        double sp = (qtUs > 0) ? (double)naiveUs / qtUs : 0.0;
        ss << std::fixed << std::setprecision(1) << sp << "x speedup";
        txt(ss.str(), 18, 124, sf::Color(255, 200, 50));
    }
    txt(showGrid ? "[Q] Grid: ON" : "[Q] Grid: OFF", 18, 148,
        showGrid ? sf::Color(80, 200, 120) : sf::Color(120, 120, 120));

    // ── Location selector panel (top-right) ────────────────────────────────
    float sx = WIN_W - 260.f;
    panel(sx - 8, 8, 268, 210);
    txt("INSERT MODE  [1-6]", sx, 16, sf::Color(80, 200, 120), 13);

    for (int i = 0; i < 6; i++) {
        float ry = 40.f + i * 28.f;
        bool sel = (i == selectedType);

        // Highlight selected
        if (sel) {
            sf::RectangleShape hi(sf::Vector2f(246, 24));
            hi.setPosition(sx - 4, ry - 2);
            hi.setFillColor(sf::Color(LOC_TYPES[i].color.r,
                                      LOC_TYPES[i].color.g,
                                      LOC_TYPES[i].color.b, 40));
            hi.setOutlineColor(LOC_TYPES[i].color);
            hi.setOutlineThickness(1.f);
            win.draw(hi);
        }

        // Color dot
        sf::CircleShape dot(6.f);
        dot.setOrigin(6,6);
        dot.setPosition(sx + 10, ry + 10);
        dot.setFillColor(LOC_TYPES[i].color);
        win.draw(dot);

        std::string label = "[" + std::to_string(i+1) + "] " + LOC_TYPES[i].name;
        if (sel) label += " <";
        txt(label, sx + 26, ry + 2,
            sel ? LOC_TYPES[i].color : sf::Color(170, 170, 170), 13);
    }

    // ── Query breakdown (bottom-left) ─────────────────────────────────────
    if (found > 0) {
        panel(8, WIN_H - 195, 250, 185);
        txt("Query results by type:", 18, WIN_H - 190, sf::Color(150, 150, 150), 12);
        for (int i = 0; i < 6; i++) {
            if (catCounts[i] == 0) continue;
            std::string s = LOC_TYPES[i].name + std::string(": ") + std::to_string(catCounts[i]);
            txt(s, 18, WIN_H - 170 + i * 22, LOC_TYPES[i].color, 13);
        }
    }

    // ── Controls (bottom-right) ────────────────────────────────────────────
    float cx = WIN_W - 268.f;
    panel(cx - 8, WIN_H - 130, 276, 122);
    txt("Left Click:  Insert selected type", cx, WIN_H - 124, sf::Color(160,160,160), 12);
    txt("Right Drag:  Range query",          cx, WIN_H - 104, sf::Color(160,160,160), 12);
    txt("G: 500 random pts   R: Reset",      cx, WIN_H - 84,  sf::Color(160,160,160), 12);
    txt("B: Benchmark (console)",            cx, WIN_H - 64,  sf::Color(160,160,160), 12);
    txt("Q: Toggle grid   ESC: Exit",        cx, WIN_H - 44,  sf::Color(160,160,160), 12);
}

// ─── MAIN ─────────────────────────────────────────────────────────────────
int main() {
    sf::RenderWindow window(sf::VideoMode(WIN_W, WIN_H),
                            " - Quadtree Spatial Query System",
                            sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(60);

    sf::Font font;
    bool fontLoaded = font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf");
    if (!fontLoaded)
        fontLoaded = font.loadFromFile("C:/Windows/Fonts/arial.ttf");
    if (!fontLoaded)
        fontLoaded = font.loadFromFile("Arial.ttf");

    Rectangle worldBounds(WORLD_X, WORLD_Y, WORLD_HW, WORLD_HH);
    Quadtree qt(worldBounds);
    NaiveSearch naive;

    std::vector<Point> allPoints;
    std::vector<Point> queryResult;
    Rectangle queryRect(0, 0, 0, 0);
    bool hasQuery   = false;
    bool showGrid   = true;
    bool dragging   = false;
    sf::Vector2f dragStart;

    int selectedType = 0;         // currently selected location type (0–5)
    int catCounts[6] = {0};
    long long lastQtUs    = -1;
    long long lastNaiveUs = -1;

    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> catDist(0, 5);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed ||
               (event.type == sf::Event::KeyPressed &&
                event.key.code == sf::Keyboard::Escape))
                window.close();

            if (event.type == sf::Event::KeyPressed) {
                auto k = event.key.code;

                // 1–6: select location type
                if (k >= sf::Keyboard::Num1 && k <= sf::Keyboard::Num6)
                    selectedType = k - sf::Keyboard::Num1;

                // Q: toggle grid
                if (k == sf::Keyboard::Q) showGrid = !showGrid;

                // R: reset
                if (k == sf::Keyboard::R) {
                    qt.clear(); naive.clear();
                    allPoints.clear(); queryResult.clear();
                    hasQuery = false;
                    lastQtUs = lastNaiveUs = -1;
                    std::fill(catCounts, catCounts+6, 0);
                }

                // G: generate 500 random points
                if (k == sf::Keyboard::G) {
                    std::uniform_real_distribution<float> dx(20.f, WIN_W - 20.f);
                    std::uniform_real_distribution<float> dy(20.f, WIN_H - 20.f);
                    for (int i = 0; i < 500; i++) {
                        Point p(dx(rng), dy(rng), catDist(rng));
                        if (qt.insert(p)) { naive.insert(p); allPoints.push_back(p); }
                    }
                }

                // B: benchmark
                if (k == sf::Keyboard::B) {
                    std::vector<BenchmarkResult> results;
                    Rectangle qr(WORLD_X, WORLD_Y, WIN_W/4.f, WIN_H/4.f);
                    for (int n : {500, 1000, 5000, 10000, 50000})
                        results.push_back(Benchmark::runRangeQuery(n, worldBounds, qr));
                    Benchmark::printResults(results);
                }
            }

            // Left click: insert selected location type
            if (event.type == sf::Event::MouseButtonPressed &&
                event.mouseButton.button == sf::Mouse::Left) {
                float mx = (float)event.mouseButton.x;
                float my = (float)event.mouseButton.y;
                Point p(mx, my, selectedType);
                if (qt.insert(p)) { naive.insert(p); allPoints.push_back(p); }
            }

            // Right drag: range query
            if (event.type == sf::Event::MouseButtonPressed &&
                event.mouseButton.button == sf::Mouse::Right) {
                dragStart = {(float)event.mouseButton.x, (float)event.mouseButton.y};
                dragging = true; hasQuery = false; queryResult.clear();
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
                    hasQuery = true;
                    queryResult.clear();

                    auto t1 = std::chrono::high_resolution_clock::now();
                    qt.queryRange(queryRect, queryResult);
                    auto t2 = std::chrono::high_resolution_clock::now();
                    lastQtUs = std::chrono::duration_cast<std::chrono::microseconds>(t2-t1).count();

                    std::fill(catCounts, catCounts+6, 0);
                    for (const Point& p : queryResult)
                        if (p.id >= 0 && p.id < 6) catCounts[p.id]++;

                    auto t3 = std::chrono::high_resolution_clock::now();
                    naive.queryRange(queryRect);
                    auto t4 = std::chrono::high_resolution_clock::now();
                    lastNaiveUs = std::chrono::duration_cast<std::chrono::microseconds>(t4-t3).count();
                }
            }
        }

        // rendering 
        window.clear(sf::Color(14, 18, 24));

        if (showGrid) drawQuadtreeGrid(window, qt);

        // All points
        for (const Point& p : allPoints)
            drawPoint(window, p, getPointColor(p.id), 4.f);

        // Query box
        if (hasQuery) {
            sf::RectangleShape qbox(sf::Vector2f(queryRect.halfW*2, queryRect.halfH*2));
            qbox.setPosition(queryRect.left(), queryRect.top());
            qbox.setFillColor(sf::Color(255, 220, 50, 25));
            qbox.setOutlineColor(sf::Color(255, 220, 50, 200));
            qbox.setOutlineThickness(2.f);
            window.draw(qbox);
            // Highlighted results: slightly bigger dot
            for (const Point& p : queryResult)
                drawPoint(window, p, getPointColor(p.id), 6.f);
        }

        // Live drag preview
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
                   showGrid, lastQtUs, lastNaiveUs, selectedType, catCounts);

        window.display();
    }
    return 0;
}