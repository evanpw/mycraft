#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "camera.hpp"
#include "chunk_manager.hpp"
#include <glm/glm.hpp>

class Player
{
public:
	static constexpr float EYE_HEIGHT = 1.62;		// Height of eyes in blocks
	static constexpr float PLAYER_HEIGHT = 1.70;	// Height of player in blocks
	static constexpr float WALKING_SPEED = 4.3;		// Blocks / s
	static constexpr float FLYING_SPEED = 2.5 * WALKING_SPEED;
	static constexpr float GRAVITY = 32;			// Blocks / s^2
	static constexpr float AIR_RESISTANCE = 0.4;    // 1 / s
	static constexpr float JUMP_VELOCITY = 8.4;		// Blocks / s

	// initialPosition is of the player's eye
	Player(const ChunkManager& chunkManager, const glm::vec3& initialPosition);

	enum Direction {FORWARD, BACKWARD, LEFT, RIGHT};
	void step(Direction direction);

	// Will only actually jump if the player is not in mid-air
	void jump();

	// Negative angles are allowed. Angles are in degrees.
	void turnRight(float angle);
	void tiltUp(float angle);

	void update(float elapsed);

	const Camera& camera() const { return m_camera; }

	// Calls the private potentialIntersections with the current position
	std::vector<Coordinate> potentialIntersections() const;

	// Determines whether the player's eye is currently in a water block
	bool isUnderwater() const;

private:
	bool inAir() const;

	// Get all block coordinates which a player's bounding box with eye at the given
	// location would intersect, regardless of whether they contain a block.
	std::vector<Coordinate> potentialIntersections(const glm::vec3& eye) const;

	// This will search for all intersections with a solid block, and adjust delta
	// so that the intersection no longer occurs. It will also zero out the
	// player's velocity in the axis of collision.
	void resolveCollisions(glm::vec3& delta);

	Camera m_camera;
	glm::vec3 m_step;
	glm::vec3 m_velocity;

	const ChunkManager& m_chunkManager;
};

#endif