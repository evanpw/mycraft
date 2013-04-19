#ifndef NOISE_HPP
#define NOISE_HPP

#include <cstdint>
#include <cstdlib>
#include <vector>

// Generate noise via the midpoint displacement algorithm
// http://www.lighthouse3d.com/opengl/terrain/index.php3?mpd2
class Noise
{
public:
	Noise(uint8_t k, float range = 16)
	: m_size((1 << k) + 1)
	{
		m_grid.resize(m_size);
		for (size_t i = 0; i < m_size; ++i)
		{
			m_grid[i].resize(m_size);
		}

		// Fill in the corners first
		unsigned int scale = 1 << k;
		float d = range / 4;
		m_grid[0][0] = fuzz(d);
		m_grid[0][scale] = fuzz(d);
		m_grid[scale][0] = fuzz(d);
		m_grid[scale][scale] = fuzz(d);

		while (scale > 1)
		{
			scale /= 2;

			// Center of squares
			for (size_t i = scale; i < m_size; i += scale * 2)
				for (size_t j = scale; j < m_size; j += scale * 2)
					m_grid[i][j] = averageCorners(i, j, scale) + fuzz(d);

			// Sides of squares
			for (size_t i = 0; i < m_size; i += scale * 2)
				for (size_t j = scale; j < m_size; j += scale * 2)
					m_grid[i][j] = averageNeighbors(i, j, scale) + fuzz(d);

			for (size_t i = scale; i < m_size; i += scale * 2)
				for (size_t j = 0; j < m_size; j += scale * 2)
					m_grid[i][j] = averageNeighbors(i, j, scale) + fuzz(d);

			d /= 2;
		}

		// Shift upward to yield positive numbers
		for (size_t i = 0; i < m_size; ++i)
		{
			for (size_t j = 0; j < m_size; ++j)
			{
				m_grid[i][j] += (range / 2);
				assert(m_grid[i][j] >= 0 && m_grid[i][j] < range);
			}
		}
	}

	int operator()(size_t i, size_t j)
	{
		return m_grid[i][j];
	}

	const std::vector<std::vector<float>> grid() const
	{
		return m_grid;
	}

	float fuzz(float range)
	{
		return (rand() / static_cast<float>(RAND_MAX)) * 2 * range - range;
	}

	float averageCorners(size_t i, size_t j, unsigned int scale)
	{
		return (m_grid[i - scale][j - scale] +
				m_grid[i + scale][j - scale] +
				m_grid[i - scale][j + scale] +
				m_grid[i + scale][j - scale]) / 4;
	}

	float averageNeighbors(size_t i, size_t j, unsigned int scale)
	{
		float total = 0;
		int count = 0;

		if (i >= scale)
		{
			total += m_grid[i - scale][j];
			++count;
		}

		if (i + scale < m_size)
		{
			total += m_grid[i + scale][j];
			++count;
		}

		if (j >= scale)
		{
			total += m_grid[i][j - scale];
			++count;
		}

		if (j + scale < m_size)
		{
			total += m_grid[i][j + scale];
			++count;
		}

		return total / count;
	}

private:
	size_t m_size;
	std::vector<std::vector<float>> m_grid;
};

#endif