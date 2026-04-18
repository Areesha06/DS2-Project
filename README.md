# CS201 - Data Structures II
## Efficient Spatial Query System using Quadtree

---

## Files Overview

| File              | Purpose                                           |
|-------------------|---------------------------------------------------|
| `Point.h`         | Basic 2D point struct                             |
| `Rectangle.h`     | AABB rectangle for boundaries and queries         |
| `Quadtree.h/cpp`  | Core Quadtree data structure implementation       |
| `NaiveSearch.h`   | Brute-force O(n) search for comparison            |
| `Benchmark.h`     | Timing and benchmark utilities                    |
| `main.cpp`        | SFML interactive visualization app                |
| `test_console.cpp`| Console-only tests + benchmarks (no SFML needed)  |
| `Makefile`        | Build file for Linux/Mac                          |

---

## Setup & Compilation

### Option A — Console Only (Easier, no SFML needed)
```bash
g++ -std=c++17 -O2 -o test test_console.cpp Quadtree.cpp
./test
```

### Option B — Full SFML Visualization

**Linux (Ubuntu/Debian):**
```bash
sudo apt install libsfml-dev
make
./quadtree_demo
```

**Windows (MinGW):**
1. Download SFML from https://sfml-dev.org (MinGW version)
2. Extract and set paths in your IDE or compile manually:
```
g++ -std=c++17 -O2 -o quadtree_demo main.cpp Quadtree.cpp -I<sfml>/include -L<sfml>/lib -lsfml-graphics -lsfml-window -lsfml-system
```

**Visual Studio (Windows):**
- Add all .h and .cpp files to project
- Link SFML .lib files in Project Properties → Linker → Input

---

## How to Use the Visualization

| Control              | Action                              |
|----------------------|-------------------------------------|
| Left Click           | Insert a point at cursor            |
| Right Click + Drag   | Draw a range query rectangle        |
| G                    | Generate 500 random points          |
| B                    | Run benchmarks (output in console)  |
| Q                    | Toggle quadtree grid visibility     |
| R                    | Reset / clear everything            |
| ESC                  | Exit                                |

---

## Quadtree Explanation

A **Quadtree** recursively divides 2D space into four quadrants:
- **NW** (North-West), **NE** (North-East)
- **SW** (South-West), **SE** (South-East)

Each node holds up to `QT_CAPACITY = 4` points. When full, it **subdivides** and redistributes points into children.

### Key Operations

| Operation     | Quadtree        | Naive (Brute Force) |
|---------------|-----------------|---------------------|
| Insert        | O(log n)        | O(1)                |
| Range Query   | O(log n + k)    | O(n)                |
| Point Query   | O(log n)        | O(n)                |

Where `n` = total points, `k` = points in result set.

---

## Important Constants (in Quadtree.h)
```cpp
const int QT_CAPACITY = 4;   // Points per node before subdivision
const int QT_MAX_DEPTH = 8;  // Max tree depth (prevents infinite recursion)
```
You can experiment with these values and observe the effect on the grid.

---

## Sample Benchmark Output
```
Dataset Size    Quadtree (us)   Naive (us)    Speedup    Pts Found
-----------------------------------------------------------------------
500             12              45            3.75x      47
1000            18              90            5.00x      96
5000            25              430           17.20x     489
10000           30              850           28.33x     978
50000           38              4200          110.53x    4890
```
The speedup grows as dataset size increases — demonstrating the O(log n) vs O(n) difference.
