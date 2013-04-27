#ifndef RAY_CASTER_HPP
#define RAY_CASTER_HPP

class Camera;
class ChunkManager;
class Coordinate;

// Determine the block that the camera is looking directly at
bool castRay(const Camera& camera, const ChunkManager& chunkManager, Coordinate& result);

#endif