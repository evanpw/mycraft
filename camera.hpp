#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <glm/glm.hpp>

struct Camera
{
	Camera() : horizontalAngle(0), verticalAngle(0) {}

	// Camera location in world coordinates
	glm::vec3 eye;

	// Camera rotation about the y-axis
	float horizontalAngle;

	// Camera rotation about the x-axis
	float verticalAngle;
};

#endif