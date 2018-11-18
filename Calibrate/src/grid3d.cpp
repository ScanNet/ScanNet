#include "stdafx.h"
#include "grid3d.h"

Grid3D::
Grid3D()
{
	m_xRes = -1;
	m_yRes = -1;
	m_zRes = -1;
	m_maxDist = -1;

	m_data = nullptr;
}

Grid3D::
Grid3D(int xRes, int yRes, int zRes, float maxDist)
{
	m_xRes = xRes;
	m_yRes = yRes;
	m_zRes = zRes;
	m_maxDist = maxDist;

	int nElements = NElements();
	m_data = new float[nElements];
	for (int i = 0; i < nElements; ++i)
	{
		m_data[i] = 0.0f;
	}
}

Grid3D::
Grid3D(const Grid3D &other)
{
	m_xRes = other.XRes();
	m_yRes = other.YRes();
	m_zRes = other.ZRes();
	m_maxDist = other.MaxDist();

	int nElements = NElements();
	m_data = new float[nElements];
	for (int i = 0; i < nElements; ++i)
	{
		m_data[i] = other.GetValue(i);
	}
}

Grid3D::
Grid3D(const std::string &filename)
{
	ReadFile(filename);
}

Grid3D::
~Grid3D()
{
	if (m_data != nullptr)
	{
		delete [] m_data;
		m_data = nullptr;
	}
	m_xRes = -1;
	m_yRes = -1;
	m_zRes = -1;
	m_maxDist = -1;
}

int Grid3D::
ToIndex(int x, int y, int z) const
{
	return (z * m_xRes * m_yRes) + (y * m_xRes) + x;
}

int Grid3D::
XRes() const
{
	return m_xRes;
}

int Grid3D::
YRes() const
{
	return m_yRes;
}

int Grid3D::
ZRes() const
{
	return m_zRes;
}

int Grid3D::
NElements() const
{
	return m_xRes * m_yRes * m_zRes;
}

float Grid3D::
MaxDist() const
{
	return m_maxDist;
}

float Grid3D::
GetValue(int i) const
{
	assert(i >= 0 && i < NElements());
	return m_data[i];
}

float Grid3D::
GetValue(int x, int y, int z) const
{
	assert(x >= 0 && x < m_xRes);
	assert(y >= 0 && y < m_yRes);
	assert(z >= 0 && z < m_zRes);
	return m_data[ToIndex(x, y, z)];
}

float Grid3D::
GetValue(float x, float y, float z) const
{
	assert(x >= 0 && x < m_xRes);
	assert(y >= 0 && y < m_yRes);
	assert(z >= 0 && z < m_zRes);

	int x1 = (int) x;
	int y1 = (int) y;
	int z1 = (int) z;
	int x2 = x1 + 1;
	int y2 = y1 + 1;
	int z2 = z1 + 1;
	if (x2 >= m_xRes) x2 = x1;
	if (y2 >= m_yRes) y2 = y1;
	if (z2 >= m_zRes) z2 = z1;
	float dx = x - x1;
	float dy = y - y1;
	float dz = z - z1;

	float value = 0.0f;
	value += GetValue(x1, y1, z1) * (1.0f - dx) * (1.0f - dy) * (1.0f - dz);
	value += GetValue(x1, y1, z2) * (1.0f - dx) * (1.0f - dy) * dz;
	value += GetValue(x1, y2, z1) * (1.0f - dx) * dy * (1.0f - dz);
	value += GetValue(x1, y2, z2) * (1.0f - dx) * dy * dz;

	value += GetValue(x2, y1, z1) * dx * (1.0f - dy) * (1.0f - dz);
	value += GetValue(x2, y1, z2) * dx * (1.0f - dy) * dz;
	value += GetValue(x2, y2, z1) * dx * dy * (1.0f - dz);
	value += GetValue(x2, y2, z2) * dx * dy * dz;

	return value;
}

void Grid3D::
SetValue(int i, float val)
{
	assert(i >= 0 && i < NElements());

	m_data[i] = val;
}

void Grid3D::
SetValue(int x, int y, int z, float val)
{
	assert(x >= 0 && x < m_xRes);
	assert(y >= 0 && y < m_yRes);
	assert(z >= 0 && z < m_zRes);

	m_data[ToIndex(x, y, z)] = val;
}

void Grid3D::
Add(const float val)
{
	int nElements = NElements();
	for (int i = 0; i < nElements; ++i)
	{
		if (m_data[i] != GRID3D_UNKNOWN_VALUE)
			m_data[i] += val;
	}
}

void Grid3D::
Add(int x, int y, int z, float val)
{
	assert(x >= 0 && x < m_xRes);
	assert(y >= 0 && y < m_yRes);
	assert(z >= 0 && z < m_zRes);
	int i = ToIndex(x, y, z);
	if (m_data[i] != GRID3D_UNKNOWN_VALUE)
	{
		m_data[i] += val;
	}
}

void Grid3D::
Add(const Grid3D &other)
{
	assert(other.XRes() == m_xRes);
	assert(other.YRes() == m_yRes);
	assert(other.ZRes() == m_zRes);

	int nElements = NElements();
	for (int i = 0; i < nElements; ++i)
	{
		if (m_data[i] != GRID3D_UNKNOWN_VALUE &&
			other.GetValue(i) != GRID3D_UNKNOWN_VALUE)
		{
			m_data[i] += other.GetValue(i);
		}
	}
}

void Grid3D::
Multiply(const float val)
{
	int nElements = NElements();
	for (int i = 0; i < nElements; ++i)
	{
		if (m_data[i] != GRID3D_UNKNOWN_VALUE)
		{
			m_data[i] *= val;
		}
	}
}

void Grid3D::
Multiply(int x, int y, int z, float val)
{
	assert(x >= 0 && x < m_xRes);
	assert(y >= 0 && y < m_yRes);
	assert(z >= 0 && z < m_zRes);
	int i = ToIndex(x, y, z);
	if (m_data[i] != GRID3D_UNKNOWN_VALUE)
	{
		m_data[i] *= val;
	}
}

void Grid3D::
Multiply(const Grid3D &other)
{
	assert(other.XRes() == m_xRes);
	assert(other.YRes() == m_yRes);
	assert(other.ZRes() == m_zRes);

	int nElements = NElements();
	for (int i = 0; i < nElements; ++i)
	{
		if (m_data[i] != GRID3D_UNKNOWN_VALUE &&
			other.GetValue(i) != GRID3D_UNKNOWN_VALUE)
		{
			m_data[i] *= other.GetValue(i);
		}
	}
}

void Grid3D::
Divide(const float val)
{
	if (val == 0.0f) return;

	int nElements = NElements();
	for (int i = 0; i < nElements; ++i)
	{
		if (m_data[i] != GRID3D_UNKNOWN_VALUE)
		{
			m_data[i] /= val;
		}
	}
}

void Grid3D::
Divide(int x, int y, int z, float val)
{
	if (val == 0.0f) return;
	assert(x >= 0 && x < m_xRes);
	assert(y >= 0 && y < m_yRes);
	assert(z >= 0 && z < m_zRes);

	int i = ToIndex(x, y, z);
	if (m_data[i] != GRID3D_UNKNOWN_VALUE)
	{
		m_data[i] /= val;
	}
}

void Grid3D::
Divide(const Grid3D &other)
{
	assert(other.XRes() == m_xRes);
	assert(other.YRes() == m_yRes);
	assert(other.ZRes() == m_zRes);

	int nElements = NElements();
	for (int i = 0; i < nElements; ++i)
	{
		if (other.GetValue(i) == 0.0f) // dont divide by zero.
		{
			m_data[i] = GRID3D_UNKNOWN_VALUE;
			continue;
		}

		if (m_data[i] != GRID3D_UNKNOWN_VALUE &&
			other.GetValue(i) != GRID3D_UNKNOWN_VALUE)
		{
			m_data[i] /= other.GetValue(i);
		}
	}
}

void Grid3D::
InvertElements()
{
	int nElements = NElements();
	for (int i = 0; i < nElements; ++i)
	{
		if (m_data[i] == 0.0f) // dont divide by zero.
		{
			m_data[i] = GRID3D_UNKNOWN_VALUE;
			continue;
		}

		if (m_data[i] != GRID3D_UNKNOWN_VALUE)
		{
			m_data[i] = 1.0f / m_data[i];
		}
	}
}

float Grid3D::
Min() const
{
	float min = 1e9;
	int nElements = NElements();
	for (int i = 0; i < nElements; ++i)
	{
		if (m_data[i] != GRID3D_UNKNOWN_VALUE &&
			m_data[i] < min)
		{
			min = m_data[i];
		}
	}
	return min;
}

float Grid3D::
Max() const
{
	float max = -1e9;
	int nElements = NElements();
	for (int i = 0; i < nElements; ++i)
	{
		if (m_data[i] != GRID3D_UNKNOWN_VALUE &&
			m_data[i] > max)
		{
			max = m_data[i];
		}
	}
	return max;
}

int Grid3D::
ReadFile(const std::string &filename)
{
	// clear old data
	if (m_data != nullptr)
	{
		delete [] m_data;
		m_data = nullptr;
	}

	// open file
	FILE * fp = fopen(filename.c_str(), "rb");
	if (!fp)
	{
		printf("Could not open file %s for reading!\n", filename.c_str());
		fflush(stdout);
		return 0;
	}

	// read in header
	int res[3];
	if (fread(res, sizeof(int), 3, fp) != 3) {
		printf("Unable to read resolution from file %s\n", filename.c_str());
		return 0;
	}
	m_xRes = res[0];
	m_yRes = res[1];
	m_zRes = res[2];

	if (fread(&m_maxDist, sizeof(float), 1, fp) != 1) {
		printf("Unable to read max. distance from file %s\n", filename.c_str());
		return 0;
	}

	// allocate and read data
	int nElements = NElements();
	m_data = new float[nElements];
	if (fread(&(m_data[0]), sizeof(float), nElements, fp) != nElements) {
		printf("Unable to read grid values from file %s\n", filename.c_str());
		return 0;
	}

	return -1;
}

int Grid3D::
WriteFile(const std::string &filename) const
{
	FILE * fp = fopen(filename.c_str(), "wb");
	if (!fp)
	{
		printf("Could not open file %s for writing!\n", filename.c_str());
		return 0;
	}

	if (fwrite(&m_xRes, sizeof(int), 3, fp) != 3) {
		printf("Unable to write resolution to file %s\n", filename.c_str());
		return 0;
	}

	if (fwrite(&m_maxDist, sizeof(float), 1, fp) != 1) {
		printf("Unable to write maximum distance to file %s\n", filename.c_str());
		return 0;
	}

	const int nElements = NElements();
	int test = (int)fwrite(&(m_data[0]), sizeof(float), nElements, fp);
	if (test != nElements) {
		printf("Unable to write grid values to file %s\n", filename.c_str());
		return 0;
	}

	fclose(fp);

	return 1;
}