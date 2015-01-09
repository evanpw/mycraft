#ifndef RAY_CASTER_HPP
#define RAY_CASTER_HPP

struct Camera;
class ChunkManager;
struct Coordinate;

// Determine the block that the camera is looking directly at
bool castRay(const Camera& camera, const ChunkManager& chunkManager, Coordinate& result, Coordinate& lastOpen);

#endif
