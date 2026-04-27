# Data Structures II
## Efficient Spatial Query System using Quadtree

---


| File              | Purpose                                                |
|-------------------|--------------------------------------------------------|
| `Point.h`         | Basic 2D point structure                               |
| `Rectangle.h`     | AABB rectangle for boundaries and queries             |
| `Quadtree.h/cpp`  | Adaptive spatial partitioning structure               |
| `UniformGrid.h`   | Fixed-cell spatial hashing structure                  |
| `NaiveSearch.h`   | Brute-force O(n) search for comparison               |
| `Benchmark.h`     | Performance benchmarking utilities                    |
| `main.cpp`        | SFML interactive visualization + simulation          |
| `data.js`         | Auto-generated benchmark data                        |
| `index.html`      | HTML dashboard for visualization                     |
| `Makefile`        | Build configuration (Linux/Mac)                      |

---

## Setup & Compilation

### Linux (Ubuntu/Debian)
```bash
sudo apt install libsfml-dev
make
./quadtree_demo

---

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
| B                    | Run benchmarks and export data.js   |
| Q                    | Toggle quadtree grid visibility     |
| C                    | Generate clustered points           |
| K	                   | Benchmark current screen points     |
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

=========================================================================================================
  RANGE QUERY BENCHMARK — UNIFORM DATA
=========================================================================================================
Dataset (N)       Quadtree (us)     UniformGrid (us)  Naive (us)        QT Speedup        Grid Speedup      Pts Found
---------------------------------------------------------------------------------------------------------
500               1.36              9.94              5.21              3.84              0.52              43
1000              5.74              4.92              6.77              1.18              1.38              104
5000              11.88             21.92             55.44             4.67              2.53              470
10000             93.66             54.39             174.58            1.86              3.21              921
25000             103.52            308.01            304.35            2.94              0.99              2300
50000             422.40            672.77            812.75            1.92              1.21              4592
=========================================================================================================

=========================================================================================================
  RANGE QUERY BENCHMARK — CLUSTERED DATA
=========================================================================================================
Dataset (N)       Quadtree (us)     UniformGrid (us)  Naive (us)        QT Speedup        Grid Speedup      Pts Found
---------------------------------------------------------------------------------------------------------
500               2.04              2.30              2.34              1.15              1.02              158
1000              4.34              4.65              5.78              1.33              1.24              313
5000              16.96             19.34             33.72             1.99              1.74              1640
10000             41.42             43.20             74.41             1.80              1.72              3275
25000             123.97            159.03            230.87            1.86              1.45              8307
50000             194.50            293.22            439.03            2.26              1.50              16546
=========================================================================================================
```
The speedup grows as dataset size increases — demonstrating the O(log n) vs O(n) difference.
