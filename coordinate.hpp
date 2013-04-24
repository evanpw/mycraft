#ifndef COORDINATE_HPP
#define COORDINATE_HPP

#include <iostream>
#include <glm/glm.hpp>

struct Coordinate
{
	Coordinate()
	: Coordinate(0, 0, 0)
	{}

	Coordinate(int x, int y, int z)
	: x(x), y(y), z(z)
	{}

	Coordinate(const glm::vec3& v)
	{
		x = int(floor(v.x));
		y = int(floor(v.y));
		z = int(floor(v.z));
	}

	glm::vec3 vec3() const
	{
		return glm::vec3(x, y, z);
	}

	Coordinate addX(int delta) const { return Coordinate(x + delta, y, z); }
	Coordinate addY(int delta) const { return Coordinate(x, y + delta, z); }
	Coordinate addZ(int delta) const { return Coordinate(x, y, z + delta); }

	int x, y, z;
};

std::ostream& operator<<(std::ostream& out, const Coordinate& r);
bool operator<(const Coordinate& lhs, const Coordinate& rhs);
bool operator==(const Coordinate& lhs, const Coordinate& rhs);

#endif