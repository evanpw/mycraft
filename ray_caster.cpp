#include "chunk_manager.hpp"
#include "coordinate.hpp"
#include "ray_caster.hpp"

// Maximum distance at which one can target (and destroy / place) a block
const float MAX_TARGET_DISTANCE = 10.0f;

// TODO: Should this go in chunkManager?
bool castRay(const Camera& camera, const ChunkManager& chunkManager, Coordinate& result)
{
	glm::vec3 current = camera.eye;
	glm::vec3 gaze = camera.gaze();

	Coordinate currentBlock(current);
	glm::vec3 fractional = glm::fract(current);

	// The direction of travel, in block coordinates
	Coordinate step(glm::sign(gaze));

	// The distance along gaze between consecutive blocks boundary
	glm::vec3 delta = glm::length(gaze) / glm::abs(gaze);

	// The distance along gaze to the first block boundary
	glm::vec3 distance;
	distance.x = gaze.x < 0 ? fractional.x : 1 - fractional.x;
	distance.y = gaze.y < 0 ? fractional.y : 1 - fractional.y;
	distance.z = gaze.z < 0 ? fractional.z : 1 - fractional.z;
	distance *= delta;

	do
	{
		// Travel the smallest distance necessary to hit the next block

		// Intersects x-wall first
		if (distance.x <= distance.y && distance.x <= distance.z)
		{
			current += distance.x * gaze;
			currentBlock.x += step.x;
			distance -= glm::vec3(distance.x);
			distance.x = delta.x;
		}
		else if (distance.y <= distance.x && distance.y <= distance.z)
		{
			current += distance.y * gaze;
			currentBlock.y += step.y;
			distance -= glm::vec3(distance.y);
			distance.y = delta.y;
		}
		else if (distance.z <= distance.x && distance.z <= distance.y)
		{
			current += distance.z * gaze;
			currentBlock.z += step.z;
			distance -= glm::vec3(distance.z);
			distance.z = delta.z;
		}
		else
		{
			// Numerical error?
			assert(false);
		}

		// Only look for intersections within a certain range of the camera
		if (glm::length(current - camera.eye) > MAX_TARGET_DISTANCE)
			return false;

	} while (chunkManager.isTransparent(currentBlock));

	result = currentBlock;
	return true;
}