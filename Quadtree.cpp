#include "Quadtree.h"

Quadtree::Quadtree(Rectangle boundary, int depth) : boundary(boundary), divided(false), depth(depth), NE(nullptr), NW(nullptr), SE(nullptr), SW(nullptr) {} // constructor, no children initially

Quadtree::~Quadtree() { // destructor
    clear();
}

void Quadtree::subdivide() {
    float x = boundary.x;
    float y = boundary.y;
    float hw = boundary.halfW / 2.0f;
    float hh = boundary.halfH / 2.0f;

    // values to reinsert existing points into appropriate children

    NE = new Quadtree(Rectangle(x + hw, y - hh, hw, hh), depth + 1);
    NW = new Quadtree(Rectangle(x - hw, y - hh, hw, hh), depth + 1);
    SE = new Quadtree(Rectangle(x + hw, y + hh, hw, hh), depth + 1);
    SW = new Quadtree(Rectangle(x - hw, y + hh, hw, hh), depth + 1);

    divided = true;

    // reinsert existing points into the correct child
    for (const Point& p : points) {
        if (!NE->insert(p))
        if (!NW->insert(p))
        if (!SE->insert(p))
            SW->insert(p);
    }
    points.clear();  // clear points from parent after redistribution
}

bool Quadtree::insert(const Point& p) { // Insert a point into the quadtree

    if (!boundary.contains(p)) // reject if point outside boundary
        return false;

    // if theres capacity and not yet divided, store here
    if ((int)points.size() < QT_CAPACITY and !divided) {
        points.push_back(p);
        return true;
    }

    // Subdivide if we havent already
    if (!divided) {
        if (depth >= QT_MAX_DEPTH) { // and we are not too deep
            // at max depth, just push even if over capacity
            points.push_back(p);
            return true;
        }
        subdivide();
    }

    // try to insert into children
    if (NE->insert(p)) return true;
    if (NW->insert(p)) return true;
    if (SE->insert(p)) return true;
    if (SW->insert(p)) return true;

    return false; // if boundary test fails
}

void Quadtree::queryRange(const Rectangle& range, std::vector<Point>& found) const { // finds all points inside a given rectangular region

    if (!boundary.intersects(range))     // If the query range doenst intersect this nodes boundary, return
        return;

    // Check points stored in this current node
    for (const Point& p : points) {
        if (range.contains(p))
            found.push_back(p);
    }

    // recurse into children
    if (divided== true) {
        NE->queryRange(range, found);
        NW->queryRange(range, found);
        SE->queryRange(range, found);
        SW->queryRange(range, found);
    }

}

bool Quadtree::queryPoint(const Point& p) const { // check if point exists in the quadtree
    if (!boundary.contains(p))
        return false;

    for (const Point& pt : points) {
        if (pt.x == p.x and pt.y == p.y)
            return true;
    }

    if (divided) {
        return NE->queryPoint(p) or NW->queryPoint(p) or SE->queryPoint(p) or SW->queryPoint(p);
    }

    return false;
}

void Quadtree::clear() { // for resetting
    points.clear();
    if (divided) {
        delete NE; NE = nullptr;
        delete NW; NW = nullptr;
        delete SE; SE = nullptr;
        delete SW; SW = nullptr;
        divided = false;
    }
}

int Quadtree::count() const { // returns total number of points in the entire subtree, for comparision 
    int total = (int)points.size();
    if (divided) {
        total += NE->count();
        total += NW->count();
        total += SE->count();
        total += SW->count();
    }
    return total;
}

void Quadtree::getAllBoundaries(std::vector<Rectangle>& rects) const { // collects all node boundaries, used in drawing the map
    rects.push_back(boundary);
    if (divided) {
        NE->getAllBoundaries(rects);
        NW->getAllBoundaries(rects);
        SE->getAllBoundaries(rects);
        SW->getAllBoundaries(rects);
    }
}
