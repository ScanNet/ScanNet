#pragma once

#define GRID3D_UNKNOWN_VALUE -321

class Grid3D
{
public:
	Grid3D();
	Grid3D(int xRes, int yRes, int zRes, float maxDist);
	Grid3D(const Grid3D &other);
	Grid3D(const std::string &filename);
	~Grid3D();

	int XRes() const;
	int YRes() const;
	int ZRes() const;
	int NElements() const;
	float MaxDist() const;

	float GetValue(int i) const;
	float GetValue(int x, int y, int z) const;
	float GetValue(float x, float y, float z) const;
	void SetValue(int i, float val);
	void SetValue(int x, int y, int z, float val);

	void Add(const float val);
	void Add(int x, int y, int z, float val);
	void Add(const Grid3D &other);

	void Multiply(const float val);
	void Multiply(int x, int y, int z, float val);
	void Multiply(const Grid3D &other);

	void Divide(const float val);
	void Divide(int x, int y, int z, float val);
	void Divide(const Grid3D &other);

	float Min() const;
	float Max() const;

	void InvertElements();

	int ReadFile(const std::string &filename);
	int WriteFile(const std::string &filename) const;

private:
	int m_xRes;
	int m_yRes;
	int m_zRes;
	float m_maxDist;

	float * m_data;

	int ToIndex(int x, int y, int z) const;
};
