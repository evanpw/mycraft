#include "coordinate.hpp"

std::ostream& operator<<(std::ostream& out, const Coordinate& r) {
    out << "(" << r.x << ", " << r.y << ", " << r.z << ")";
    return out;
}

bool operator<(const Coordinate& lhs, const Coordinate& rhs) {
    if (lhs.x < rhs.x)
        return true;
    else if (lhs.x > rhs.x)
        return false;

    if (lhs.y < rhs.y)
        return true;
    else if (lhs.y > rhs.y)
        return false;

    if (lhs.z < rhs.z)
        return true;
    else
        return false;
}

bool operator==(const Coordinate& lhs, const Coordinate& rhs) {
    return (lhs.x == rhs.x) && (lhs.y == rhs.y) && (lhs.z == rhs.z);
}