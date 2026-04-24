#include "Quadtree.h"

Quadtree::Quadtree(Rectangle boundary, int depth)
    : boundary(boundary), divided(false), depth(depth),
      NE(nullptr), NW(nullptr), SE(nullptr), SW(nullptr) {}

Quadtree::~Quadtree() {
    clear();
}

void Quadtree::subdivide() {
    float x = boundary.x;
    float y = boundary.y;
    float hw = boundary.halfW / 2.0f;
    float hh = boundary.halfH / 2.0f;

    NE = new Quadtree(Rectangle(x + hw, y - hh, hw, hh), depth + 1);
    NW = new Quadtree(Rectangle(x - hw, y - hh, hw, hh), depth + 1);
    SE = new Quadtree(Rectangle(x + hw, y + hh, hw, hh), depth + 1);
    SW = new Quadtree(Rectangle(x - hw, y + hh, hw, hh), depth + 1);

    divided = true;

    // Re-insert existing points into the correct child
    for (const Point& p : points) {
        if (!NE->insert(p))
        if (!NW->insert(p))
        if (!SE->insert(p))
            SW->insert(p);
    }
    points.clear();
}

bool Quadtree::insert(const Point& p) {
    // If point is not within boundary, reject it
    if (!boundary.contains(p))
        return false;

    // If there's capacity and not yet divided, store here
    if ((int)points.size() < QT_CAPACITY && !divided) {
        points.push_back(p);
        return true;
    }

    // Subdivide if we haven't already (and we're not too deep)
    if (!divided) {
        if (depth >= QT_MAX_DEPTH) {
            // At max depth, just push even if over capacity
            points.push_back(p);
            return true;
        }
        subdivide();
    }

    // Try to insert into children
    if (NE->insert(p)) return true;
    if (NW->insert(p)) return true;
    if (SE->insert(p)) return true;
    if (SW->insert(p)) return true;

    return false; // Should never reach here if boundary check passes
}

void Quadtree::queryRange(const Rectangle& range, std::vector<Point>& found) const {
    // If the query range doesn't intersect this node's boundary, skip
    if (!boundary.intersects(range))
        return;

    // Check points stored in this node
    for (const Point& p : points) {
        if (range.contains(p))
            found.push_back(p);
    }

    // Recurse into children
    if (divided) {
        NE->queryRange(range, found);
        NW->queryRange(range, found);
        SE->queryRange(range, found);
        SW->queryRange(range, found);
    }

}

bool Quadtree::queryPoint(const Point& p) const {
    if (!boundary.contains(p))
        return false;

    for (const Point& pt : points) {
        if (pt.x == p.x && pt.y == p.y)
            return true;
    }

    if (divided) {
        return NE->queryPoint(p) || NW->queryPoint(p) ||
               SE->queryPoint(p) || SW->queryPoint(p);
    }

    return false;
}

void Quadtree::clear() {
    points.clear();
    if (divided) {
        delete NE; NE = nullptr;
        delete NW; NW = nullptr;
        delete SE; SE = nullptr;
        delete SW; SW = nullptr;
        divided = false;
    }
}

int Quadtree::count() const {
    int total = (int)points.size();
    if (divided) {
        total += NE->count();
        total += NW->count();
        total += SE->count();
        total += SW->count();
    }
    return total;
}

void Quadtree::getAllBoundaries(std::vector<Rectangle>& rects) const {
    rects.push_back(boundary);
    if (divided) {
        NE->getAllBoundaries(rects);
        NW->getAllBoundaries(rects);
        SE->getAllBoundaries(rects);
        SW->getAllBoundaries(rects);
    }
}
