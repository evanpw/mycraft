#include "player.hpp"
#include "block_library.hpp"
#include <algorithm>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

Player::Player(const ChunkManager& chunkManager, const glm::vec3& initialPosition)
: m_chunkManager(chunkManager)
{
	m_camera.eye = initialPosition;
}

void Player::step(Direction direction)
{
	glm::mat4 rotation = glm::rotate(glm::mat4(1.0), m_camera.horizontalAngle, glm::vec3(0.0f, 1.0f, 0.0f));
	glm::vec3 facing = glm::mat3(rotation) * glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 right = glm::cross(facing, glm::vec3(0.0f, 1.0f, 0.0f));

	switch(direction)
	{
	case FORWARD:
		m_step += WALKING_SPEED * facing;
		break;

	case BACKWARD:
		m_step -= WALKING_SPEED * facing;
		break;

	case RIGHT:
		m_step += WALKING_SPEED * right;
		break;

	case LEFT:
		m_step -= WALKING_SPEED * right;
		break;

	default:
		assert(false);
	}
}

void Player::jump()
{
	if (!inAir())
		m_velocity.y += JUMP_VELOCITY;
}

void Player::turnRight(float angle)
{
	m_camera.horizontalAngle -= angle;
}

void Player::tiltUp(float angle)
{
	m_camera.verticalAngle -= angle;

	// Keep from doing somersaults or backflips
	if (m_camera.verticalAngle < -90.0)
		m_camera.verticalAngle = -90.0;

	if (m_camera.verticalAngle > 90.0)
		m_camera.verticalAngle = 90.0;
}

// Is the same as static_cast<int>(floor(x)) unless x is the range
// (n - 1e-3, n] for some integer n, in which case it returns n
int fuzzyFloor(float x)
{
	int lower = static_cast<int>(floor(x));
	int upper = static_cast<int>(ceil(x));

	if (fabs(x - upper) < 1e-3)
	{
		return upper;
	}
	else
	{
		return lower;
	}
}

bool Player::inAir() const
{
	float feetY = m_camera.eye.y - EYE_HEIGHT;
	int blockY = fuzzyFloor(feetY);
	if (fabs(feetY - blockY) > 1e-3)
		return true;

	// We have to check all the blocks below the player, not just directly below the eye
	for (int x = floor(m_camera.eye.x - 0.3f); x <= floor(m_camera.eye.x + 0.3f); ++x)
	{
		for (int z = floor(m_camera.eye.z - 0.3f); z <= floor(m_camera.eye.z + 0.3f); ++z)
		{
			Coordinate blockBelow(x, blockY - 1, z);
			if (m_chunkManager.isSolid(blockBelow))
				return false;
		}
	}

	return true;
}

std::vector<Coordinate> Player::potentialIntersections() const
{
	return potentialIntersections(m_camera.eye);
}

std::vector<Coordinate> Player::potentialIntersections(const glm::vec3& eye) const
{
	glm::vec3 playerMin = eye - glm::vec3(0.3f, EYE_HEIGHT, 0.3f);
	glm::vec3 playerMax = eye + glm::vec3(0.3f, PLAYER_HEIGHT - EYE_HEIGHT, 0.3f);

	std::vector<Coordinate> result;
	for (int x = floor(playerMin.x); x <= floor(playerMax.x); ++x)
	{
		for (int y = floor(playerMin.y); y <= floor(playerMax.y); ++y)
		{
			for (int z = floor(playerMin.z); z <= floor(playerMax.z); ++z)
			{
				result.emplace_back(x, y, z);
			}
		}
	}

	return result;
}

void Player::resolveCollisions(glm::vec3& delta)
{
	glm::vec3 oldPlayerMin = m_camera.eye - glm::vec3(0.3f, EYE_HEIGHT, 0.3f);
	glm::vec3 oldPlayerMax = m_camera.eye + glm::vec3(0.3f, PLAYER_HEIGHT - EYE_HEIGHT, 0.3f);

	glm::vec3 playerMin = oldPlayerMin + delta;
	glm::vec3 playerMax = oldPlayerMax + delta;

	// For each axis, how long can we move in the direction of delta without hitting
	// any block
	glm::vec3 bestT(1.0f);
	for (const Coordinate& location : potentialIntersections(m_camera.eye + delta))
	{
		if (!m_chunkManager.isSolid(location))
			continue;

		glm::vec3 blockMin = location.vec3();
		glm::vec3 blockMax = location.vec3() + glm::vec3(1.0f);

		// Intersections of less than a milliblock are just rounding errors. (This prevents an
		// infinite loop).
		glm::vec3 penetration = glm::min(playerMax, blockMax) - glm::max(playerMin, blockMin);
		if (penetration.x < 1e-3 || penetration.y < 1e-3 || penetration.z < 1e-3)
			continue;

		// For each axis, find the maximum t such that moving by t * delta does not intersect the
		// block.
		glm::vec3 t1 = (blockMax - oldPlayerMin) / delta;
		glm::vec3 t2 = (blockMin - oldPlayerMax) / delta;
		glm::vec3 t = glm::min(t1, t2);

		// TODO: Check for either the block or the player containing the other

		// This would mean that the block and the new player bounding box do not intersect, so we shouldn't
		// haven't gotten this far.
		assert(t.x <= 1.0f && t.y <= 1.0f && t.z <= 1.0f);

		// This would mean that the player was _currently_ intersecting the block, and we can't resolve that
		//assert(t.x >= 0.0f && t.y >= 0.0f && t.z >= 0.0f);

		int minAxis;
		float* tArray = glm::value_ptr(t);
		if (t.x < 0.0f && t.y < 0.0f && t.z < 0.0f)
		{
			// This means that the player is _currently_ intersecting the block, and is heading further in. In
			// this case, we head backwards. This should only happen because of rounding error, and the overlap
			// should be small.
			minAxis = std::max_element(tArray, tArray + 3) - tArray;

			// If the intersection is more than a milliblock, then it is a bug.
			glm::vec3 currentPenetration = glm::min(oldPlayerMax, blockMax) - glm::max(oldPlayerMin, blockMin);
			assert(currentPenetration.x < 1e-3 || currentPenetration.y < 1e-3 || currentPenetration.z < 1e-3);
		}
		else
		{
			// The component of t with the smallest nonnegative value will be the first to intersect. The player will
			// then slide along in the other directions, so this is the only component for which this block restricts motion.
			for (size_t i = 0; i <= 2; ++i)
				if (t[i] < 0.0f) t[i] = 1.0f;

			minAxis = std::min_element(tArray, tArray + 3) - tArray;
		}

		bestT[minAxis] = std::min(bestT[minAxis], t[minAxis]);
	}

	// For each direction with a collision, halt at the intersection,
	// and also stop future movement in that direction
	for (size_t i = 0; i < 3; ++i)
	{
		if (bestT[i] < 1.0f)
		{
			delta[i] = bestT[i] * delta[i];
			m_velocity[i] = 0.0f;
		}
	}
}

void Player::update(float elapsed)
{
	// If in the air, fall.
	if (inAir())
	{
		m_velocity -= elapsed * GRAVITY * glm::vec3(0.0f, 1.0f, 0.0f);
		m_velocity *= (1 - AIR_RESISTANCE * elapsed);
	}

	m_step += m_velocity;

	glm::vec3 delta = m_step * elapsed;
	if (delta != glm::vec3(0.0f))
		resolveCollisions(delta);

	m_camera.eye += delta;
	m_step = glm::vec3(0.0f);
}

bool Player::isUnderwater() const
{
	Coordinate currentBlock = m_camera.eye;
	const Block* block = m_chunkManager.getBlock(currentBlock);

	return (block && block->blockType == BlockLibrary::WATER);
}