#pragma once

#ifndef _CUDA_SIMPLE_MATRIX_UTIL_
#define _CUDA_SIMPLE_MATRIX_UTIL_

#define MINF __int_as_float(0xff800000)
#define INF  __int_as_float(0x7f800000)

#include <iostream>
#include "cudaUtil.h"

//////////////////////////////
// float2x2
//////////////////////////////

class float2x2
{
public:

	inline __device__ __host__ float2x2()
	{
	}

	inline __device__ __host__ float2x2(const float values[4])
	{
		m11 = values[0];	m12 = values[1];
		m21 = values[2];	m22 = values[3];
	}

	inline __device__ __host__ float2x2(const float2x2& other)
	{
		m11 = other.m11;	m12 = other.m12;
		m21 = other.m21; 	m22 = other.m22;
	}

	inline __device__ __host__ void setZero()
	{
		m11 = 0.0f;	m12 = 0.0f;
		m21 = 0.0f; m22 = 0.0f;
	}

	static inline __device__ __host__ float2x2 getIdentity()
	{
		float2x2 res;
		res.setZero();
		res.m11 = res.m22 = 1.0f;
		return res;
	}

	inline __device__ __host__ float2x2& operator=(const float2x2 &other)
	{
		m11 = other.m11;	m12 = other.m12;
		m21 = other.m21;	m22 = other.m22;
		return *this;
	}

	inline __device__ __host__ float2x2 getInverse()
	{
		float2x2 res;
		res.m11 =  m22; res.m12 = -m12;
		res.m21 = -m21; res.m22 =  m11;

		return res*(1.0f/det());
	}

	inline __device__ __host__ float det()
	{
		return m11*m22-m21*m12;
	}

	inline __device__ __host__ float2 operator*(const float2& v) const
	{
		return make_float2(m11*v.x + m12*v.y, m21*v.x + m22*v.y);
	}

	//! matrix scalar multiplication
	inline __device__ __host__ float2x2 operator*(const float t) const
	{
		float2x2 res;
		res.m11 = m11 * t;	res.m12 = m12 * t;
		res.m21 = m21 * t;	res.m22 = m22 * t;
		return res;
	}

	//! matrix matrix multiplication
	inline __device__ __host__ float2x2 operator*(const float2x2& other) const
	{
		float2x2 res;
		res.m11 = m11 * other.m11 + m12 * other.m21;
		res.m12 = m11 * other.m12 + m12 * other.m22;
		res.m21 = m21 * other.m11 + m22 * other.m21;
		res.m22 = m21 * other.m12 + m22 * other.m22;
		return res;
	}

	//! matrix matrix addition
	inline __device__ __host__ float2x2 operator+(const float2x2& other) const
	{
		float2x2 res;
		res.m11 = m11 + other.m11;
		res.m12 = m12 + other.m12;
		res.m21 = m21 + other.m21;
		res.m22 = m22 + other.m22;
		return res;
	}
	
	inline __device__ __host__ float& operator()(int i, int j)
	{
		return entries2[i][j];
	}

	inline __device__ __host__ float operator()(int i, int j) const
	{
		return entries2[i][j];
	}

	inline __device__ __host__ const float* ptr() const {
		return entries;
	}
	inline __device__ __host__ float* ptr() {
		return entries;
	}

	union
	{
		struct
		{
			float m11; float m12;
			float m21; float m22;
		};

		float entries[4];
		float entries2[2][2];
	};
};

//////////////////////////////
// float2x3
//////////////////////////////

class float2x3
{
public:

	inline __device__ __host__ float2x3()
	{
	}

	inline __device__ __host__ float2x3(const float values[6])
	{
		m11 = values[0];	m12 = values[1];	m13 = values[2];
		m21 = values[3];	m22 = values[4];	m23 = values[5];
	}

	inline __device__ __host__ float2x3(const float2x3& other)
	{
		m11 = other.m11;	m12 = other.m12;	m13 = other.m13;
		m21 = other.m21; 	m22 = other.m22;	m23 = other.m23;
	}

	inline __device__ __host__ float2x3& operator=(const float2x3 &other)
	{
		m11 = other.m11;	m12 = other.m12; m13 = other.m13;
		m21 = other.m21;	m22 = other.m22; m23 = other.m23;
		return *this;
	}

	inline __device__ __host__ float2 operator*(const float3 &v) const
	{
		return make_float2(m11*v.x + m12*v.y + m13*v.z, m21*v.x + m22*v.y + m23*v.z);
	}

	//! matrix scalar multiplication
	inline __device__ __host__ float2x3 operator*(const float t) const
	{
		float2x3 res;
		res.m11 = m11 * t;	res.m12 = m12 * t;	res.m13 = m13 * t;
		res.m21 = m21 * t;	res.m22 = m22 * t;	res.m23 = m23 * t;
		return res;
	}

	//! matrix scalar division
	inline __device__ __host__ float2x3 operator/(const float t) const
	{
		float2x3 res;
		res.m11 = m11 / t;	res.m12 = m12 / t;	res.m13 = m13 / t;
		res.m21 = m21 / t;	res.m22 = m22 / t;	res.m23 = m23 / t;
		return res;
	}
	
	inline __device__ __host__ float& operator()(int i, int j)
	{
		return entries2[i][j];
	}

	inline __device__ __host__ float operator()(int i, int j) const
	{
		return entries2[i][j];
	}

	inline __device__ __host__ const float* ptr() const {
		return entries;
	}
	inline __device__ __host__ float* ptr() {
		return entries;
	}

	union
	{
		struct
		{
			float m11; float m12; float m13;
			float m21; float m22; float m23;
		};

		float entries[6];
		float entries2[3][2];
	};
};

//////////////////////////////
// float3x2
//////////////////////////////

class float3x2
{
public:

	inline __device__ __host__ float3x2()
	{
	}

	inline __device__ __host__ float3x2(const float values[6])
	{
		m11 = values[0];	m12 = values[1];
		m21 = values[2];	m22 = values[3];
		m31 = values[4];	m32 = values[5];
	}

	inline __device__ __host__ float3x2& operator=(const float3x2& other)
	{
		m11 = other.m11;	m12 = other.m12;
		m21 = other.m21;	m22 = other.m22;
		m31 = other.m31;	m32 = other.m32;
		return *this;
	}

	inline __device__ __host__ float3 operator*(const float2& v) const
	{
		return make_float3(m11*v.x + m12*v.y, m21*v.x + m22*v.y, m31*v.x + m32*v.y);
	}

	inline __device__ __host__ float3x2 operator*(const float t) const
	{
		float3x2 res;
		res.m11 = m11 * t;	res.m12 = m12 * t;
		res.m21 = m21 * t;	res.m22 = m22 * t;
		res.m31 = m31 * t;	res.m32 = m32 * t;
		return res;
	}

	inline __device__ __host__ float& operator()(int i, int j)
	{
		return entries2[i][j];
	}

	inline __device__ __host__ float operator()(int i, int j) const
	{
		return entries2[i][j];
	}

	inline __device__ __host__ float2x3 getTranspose()
	{
		float2x3 res;
		res.m11 = m11; res.m12 = m21; res.m13 = m31;
		res.m21 = m12; res.m22 = m22; res.m23 = m32;
		return res;
	}

	inline __device__ __host__ const float* ptr() const {
		return entries;
	}
	inline __device__ __host__ float* ptr() {
		return entries;
	}

	union
	{
		struct
		{
			float m11; float m12;
			float m21; float m22;
			float m31; float m32;
		};

		float entries[6];
		float entries2[3][2];
	};
};

inline __device__ __host__ float2x2 matMul(const float2x3& m0, const float3x2& m1)
{
	float2x2 res;
	res.m11 = m0.m11*m1.m11+m0.m12*m1.m21+m0.m13*m1.m31;
	res.m12 = m0.m11*m1.m12+m0.m12*m1.m22+m0.m13*m1.m32;
	res.m21 = m0.m21*m1.m11+m0.m22*m1.m21+m0.m23*m1.m31;
	res.m22 = m0.m21*m1.m12+m0.m22*m1.m22+m0.m23*m1.m32;
	return res;
}

class float3x3 {
public:
	inline __device__ __host__ float3x3() {

	}
	inline __device__ __host__ float3x3(const float values[9]) {
		m11 = values[0];	m12 = values[1];	m13 = values[2];
		m21 = values[3];	m22 = values[4];	m23 = values[5];
		m31 = values[6];	m32 = values[7];	m33 = values[8];
	}

	inline __device__ __host__ float3x3(const float3x3& other) {
		m11 = other.m11;	m12 = other.m12;	m13 = other.m13;
		m21 = other.m21;	m22 = other.m22;	m23 = other.m23;
		m31 = other.m31;	m32 = other.m32;	m33 = other.m33;
	}

	explicit inline __device__ __host__ float3x3(const float2x2& other) {
		m11 = other.m11;	m12 = other.m12;	m13 = 0.0;
		m21 = other.m21;	m22 = other.m22;	m23 = 0.0;
		m31 = 0.0;			m32 = 0.0;			m33 = 0.0;
	}

	inline __device__ __host__ float3x3& operator=(const float3x3 &other) {
		m11 = other.m11;	m12 = other.m12;	m13 = other.m13;
		m21 = other.m21;	m22 = other.m22;	m23 = other.m23;
		m31 = other.m31;	m32 = other.m32;	m33 = other.m33;
		return *this;
	}

	inline __device__ __host__ float& operator()(int i, int j) {
		return entries2[i][j];
	}

	inline __device__ __host__ float operator()(int i, int j) const {
		return entries2[i][j];
	}


	static inline __device__ __host__  void swap(float& v0, float& v1) {
		float tmp = v0;
		v0 = v1;
		v1 = tmp;
	}

	inline __device__ __host__ void transpose() {
		swap(m12, m21);
		swap(m13, m31);
		swap(m23, m32);
	}
	inline __device__ __host__ float3x3 getTranspose() const {
		float3x3 ret = *this;
		ret.transpose();
		return ret;
	}

	//! inverts the matrix
	inline __device__ __host__ void invert() {
		*this = getInverse();
	}

	//! computes the inverse of the matrix; the result is returned
	inline __device__ __host__ float3x3 getInverse() const {
		float3x3 res;
		res.entries[0] = entries[4]*entries[8] - entries[5]*entries[7];
		res.entries[1] = -entries[1]*entries[8] + entries[2]*entries[7];
		res.entries[2] = entries[1]*entries[5] - entries[2]*entries[4];

		res.entries[3] = -entries[3]*entries[8] + entries[5]*entries[6];
		res.entries[4] = entries[0]*entries[8] - entries[2]*entries[6];
		res.entries[5] = -entries[0]*entries[5] + entries[2]*entries[3];

		res.entries[6] = entries[3]*entries[7] - entries[4]*entries[6];
		res.entries[7] = -entries[0]*entries[7] + entries[1]*entries[6];
		res.entries[8] = entries[0]*entries[4] - entries[1]*entries[3];
		float nom = 1.0f/det();
		return res * nom;
	}

	inline __device__ __host__ void setZero(float value = 0.0f) {
		m11 = m12 = m13 = value;
		m21 = m22 = m23 = value;
		m31 = m32 = m33 = value;
	}

	inline __device__ __host__ float det() const {
		return
			+ m11*m22*m33
			+ m12*m23*m31
			+ m13*m21*m32
			- m31*m22*m13
			- m32*m23*m11
			- m33*m21*m12;
	}

	inline __device__ __host__ float3 getRow(unsigned int i) {
		return make_float3(entries[3*i+0], entries[3*i+1], entries[3*i+2]);
	}

	inline __device__ __host__ void setRow(unsigned int i, float3& r) {
		entries[3*i+0] = r.x;
		entries[3*i+1] = r.y;
		entries[3*i+2] = r.z;
	}

	inline __device__ __host__ void normalizeRows()
	{
		//#pragma unroll 3
		for(unsigned int i = 0; i<3; i++)
		{
			float3 r = getRow(i);
			r/=length(r);
			setRow(i, r);
		}
	}

	//! computes the product of two matrices (result stored in this)
	inline __device__ __host__ void mult(const float3x3 &other) {
		float3x3 res;
		res.m11 = m11 * other.m11 + m12 * other.m21 + m13 * other.m31;
		res.m12 = m11 * other.m12 + m12 * other.m22 + m13 * other.m32;
		res.m13 = m11 * other.m13 + m12 * other.m23 + m13 * other.m33;

		res.m21 = m21 * other.m11 + m22 * other.m21 + m23 * other.m31;
		res.m22 = m21 * other.m12 + m22 * other.m22 + m23 * other.m32;
		res.m23 = m21 * other.m13 + m22 * other.m23 + m23 * other.m33;

		res.m31 = m21 * other.m11 + m32 * other.m21 + m33 * other.m31;
		res.m32 = m21 * other.m12 + m32 * other.m22 + m33 * other.m32;
		res.m33 = m21 * other.m13 + m32 * other.m23 + m33 * other.m33;
		*this = res;
	}

	//! computes the sum of two matrices (result stored in this)
	inline __device__ __host__ void add(const float3x3 &other) {
		m11 += other.m11;	m12 += other.m12;	m13 += other.m13;
		m21 += other.m21;	m22 += other.m22;	m23 += other.m23;
		m31 += other.m31;	m32 += other.m32;	m33 += other.m33;
	}

	//! standard matrix matrix multiplication
	inline __device__ __host__ float3x3 operator*(const float3x3 &other) const {
		float3x3 res;
		res.m11 = m11 * other.m11 + m12 * other.m21 + m13 * other.m31;
		res.m12 = m11 * other.m12 + m12 * other.m22 + m13 * other.m32;
		res.m13 = m11 * other.m13 + m12 * other.m23 + m13 * other.m33;

		res.m21 = m21 * other.m11 + m22 * other.m21 + m23 * other.m31;
		res.m22 = m21 * other.m12 + m22 * other.m22 + m23 * other.m32;
		res.m23 = m21 * other.m13 + m22 * other.m23 + m23 * other.m33;

		res.m31 = m31 * other.m11 + m32 * other.m21 + m33 * other.m31;
		res.m32 = m31 * other.m12 + m32 * other.m22 + m33 * other.m32;
		res.m33 = m31 * other.m13 + m32 * other.m23 + m33 * other.m33;
		return res;
	}

	//! standard matrix matrix multiplication
	inline __device__ __host__ float3x2 operator*(const float3x2 &other) const {
		float3x2 res;
		res.m11 = m11 * other.m11 + m12 * other.m21 + m13 * other.m31;
		res.m12 = m11 * other.m12 + m12 * other.m22 + m13 * other.m32;

		res.m21 = m21 * other.m11 + m22 * other.m21 + m23 * other.m31;
		res.m22 = m21 * other.m12 + m22 * other.m22 + m23 * other.m32;

		res.m31 = m31 * other.m11 + m32 * other.m21 + m33 * other.m31;
		res.m32 = m31 * other.m12 + m32 * other.m22 + m33 * other.m32;
		return res;
	}

	inline __device__ __host__ float3 operator*(const float3 &v) const {
		return make_float3(
			m11*v.x + m12*v.y + m13*v.z,
			m21*v.x + m22*v.y + m23*v.z,
			m31*v.x + m32*v.y + m33*v.z
			);
	}

	inline __device__ __host__ float3x3 operator*(const float t) const {
		float3x3 res;
		res.m11 = m11 * t;		res.m12 = m12 * t;		res.m13 = m13 * t;
		res.m21 = m21 * t;		res.m22 = m22 * t;		res.m23 = m23 * t;
		res.m31 = m31 * t;		res.m32 = m32 * t;		res.m33 = m33 * t;
		return res;
	}


	inline __device__ __host__ float3x3 operator+(const float3x3 &other) const {
		float3x3 res;
		res.m11 = m11 + other.m11;	res.m12 = m12 + other.m12;	res.m13 = m13 + other.m13;
		res.m21 = m21 + other.m21;	res.m22 = m22 + other.m22;	res.m23 = m23 + other.m23;
		res.m31 = m31 + other.m31;	res.m32 = m32 + other.m32;	res.m33 = m33 + other.m33;
		return res;
	}

	inline __device__ __host__ float3x3 operator-(const float3x3 &other) const {
		float3x3 res;
		res.m11 = m11 - other.m11;	res.m12 = m12 - other.m12;	res.m13 = m13 - other.m13;
		res.m21 = m21 - other.m21;	res.m22 = m22 - other.m22;	res.m23 = m23 - other.m23;
		res.m31 = m31 - other.m31;	res.m32 = m32 - other.m32;	res.m33 = m33 - other.m33;
		return res;
	}

	static inline __device__ __host__ float3x3 getIdentity() {
		float3x3 res;
		res.setZero();
		res.m11 = res.m22 = res.m33 = 1.0f;
		return res;
	}

	static inline __device__ __host__ float3x3 getZeroMatrix() {
		float3x3 res;
		res.setZero();
		return res;
	}

	static inline __device__ __host__ float3x3 getDiagonalMatrix(float diag = 1.0f) {
		float3x3 res;
		res.m11 = diag;		res.m12 = 0.0f;		res.m13 = 0.0f;
		res.m21 = 0.0f;		res.m22 = diag;		res.m23 = 0.0f;
		res.m31 = 0.0f;		res.m32 = 0.0f;		res.m33 = diag;
		return res;
	}

	static inline __device__ __host__  float3x3 tensorProduct(const float3 &v, const float3 &vt) {
		float3x3 res;
		res.m11 = v.x * vt.x;	res.m12 = v.x * vt.y;	res.m13 = v.x * vt.z;
		res.m21 = v.y * vt.x;	res.m22 = v.y * vt.y;	res.m23 = v.y * vt.z;
		res.m31 = v.z * vt.x;	res.m32 = v.z * vt.y;	res.m33 = v.z * vt.z;
		return res;
	}

	inline __device__ __host__ const float* ptr() const {
		return entries;
	}
	inline __device__ __host__ float* ptr() {
		return entries;
	}

	union {
		struct {
			float m11; float m12; float m13;
			float m21; float m22; float m23;
			float m31; float m32; float m33;
		};
		float entries[9];
		float entries2[3][3];
	};
};


inline __device__ __host__ float2x3 matMul(const float2x3& m0, const float3x3& m1)
{
	float2x3 res;
	res.m11 = m0.m11*m1.m11+m0.m12*m1.m21+m0.m13*m1.m31;
	res.m12 = m0.m11*m1.m12+m0.m12*m1.m22+m0.m13*m1.m32;
	res.m13 = m0.m11*m1.m13+m0.m12*m1.m23+m0.m13*m1.m33;

	res.m21 = m0.m21*m1.m11+m0.m22*m1.m21+m0.m23*m1.m31;
	res.m22 = m0.m21*m1.m12+m0.m22*m1.m22+m0.m23*m1.m32;
	res.m23 = m0.m21*m1.m13+m0.m22*m1.m23+m0.m23*m1.m33;
	return res;
}

// (1x2) row matrix as float2
inline __device__ __host__ float3 matMul(const float2& m0, const float2x3& m1)
{
	float3 res;
	res.x = m0.x*m1.m11+m0.y*m1.m21;
	res.y = m0.x*m1.m12+m0.y*m1.m22;
	res.z = m0.x*m1.m13+m0.y*m1.m23;

	return res;
}


class float3x4 {
public:
	inline __device__ __host__ float3x4() {

	}
	inline __device__ __host__ float3x4(const float values[12]) {
		m11 = values[0];	m12 = values[1];	m13 = values[2];	m14 = values[3];
		m21 = values[4];	m22 = values[5];	m23 = values[6];	m24 = values[7];
		m31 = values[8];	m32 = values[9];	m33 = values[10];	m34 = values[11];
	}

	inline __device__ __host__ float3x4(const float3x4& other) {
		m11 = other.m11;	m12 = other.m12;	m13 = other.m13;	m14 = other.m14;
		m21 = other.m21;	m22 = other.m22;	m23 = other.m23;	m24 = other.m24;
		m31 = other.m31;	m32 = other.m32;	m33 = other.m33;	m34 = other.m34;
	}

	inline __device__ __host__ float3x4(const float3x3& other) {
		m11 = other.m11;	m12 = other.m12;	m13 = other.m13;	m14 = 0.0f;
		m21 = other.m21;	m22 = other.m22;	m23 = other.m23;	m24 = 0.0f;
		m31 = other.m31;	m32 = other.m32;	m33 = other.m33;	m34 = 0.0f;
	}

	inline __device__ __host__ float3x4 operator=(const float3x4 &other) {
		m11 = other.m11;	m12 = other.m12;	m13 = other.m13;	m14 = other.m14;
		m21 = other.m21;	m22 = other.m22;	m23 = other.m23;	m24 = other.m24;
		m31 = other.m31;	m32 = other.m32;	m33 = other.m33;	m34 = other.m34;
		return *this;
	}

	inline __device__ __host__ float3x4& operator=(const float3x3 &other) {
		m11 = other.m11;	m12 = other.m12;	m13 = other.m13;	m14 = 0.0f;
		m21 = other.m21;	m22 = other.m22;	m23 = other.m23;	m24 = 0.0f;
		m31 = other.m31;	m32 = other.m32;	m33 = other.m33;	m34 = 0.0f;
		return *this;
	}

	//! assumes the last line of the matrix implicitly to be (0,0,0,1)
	inline __device__ __host__ float4 operator*(const float4 &v) const {
		return make_float4(
			m11*v.x + m12*v.y + m13*v.z + m14*v.w,
			m21*v.x + m22*v.y + m23*v.z + m24*v.w,
			m31*v.x + m32*v.y + m33*v.z + m34*v.w,
			v.w
			);
	}

	//! assumes an implicit 1 in w component of the input vector
	inline __device__ __host__ float3 operator*(const float3 &v) const {
		return make_float3(
			m11*v.x + m12*v.y + m13*v.z + m14,
			m21*v.x + m22*v.y + m23*v.z + m24,
			m31*v.x + m32*v.y + m33*v.z + m34
			);
	}

	//! matrix scalar multiplication
	inline __device__ __host__ float3x4 operator*(const float t) const {
		float3x4 res;
		res.m11 = m11 * t;		res.m12 = m12 * t;		res.m13 = m13 * t;		res.m14 = m14 * t;
		res.m21 = m21 * t;		res.m22 = m22 * t;		res.m23 = m23 * t;		res.m24 = m24 * t;
		res.m31 = m31 * t;		res.m32 = m32 * t;		res.m33 = m33 * t;		res.m34 = m34 * t;
		return res;
	}
	inline __device__ __host__ float3x4& operator*=(const float t) {
		*this = *this * t;
		return *this;
	}

	//! matrix scalar division
	inline __device__ __host__ float3x4 operator/(const float t) const {
		float3x4 res;
		res.m11 = m11 / t;		res.m12 = m12 / t;		res.m13 = m13 / t;		res.m14 = m14 / t;
		res.m21 = m21 / t;		res.m22 = m22 / t;		res.m23 = m23 / t;		res.m24 = m24 / t;
		res.m31 = m31 / t;		res.m32 = m32 / t;		res.m33 = m33 / t;		res.m34 = m34 / t;
		return res;
	}
	inline __device__ __host__ float3x4& operator/=(const float t) {
		*this = *this / t;
		return *this;
	}

	//! assumes the last line of the matrix implicitly to be (0,0,0,1)
	inline __device__ __host__ float3x4 operator*(const float3x4 &other) const {
		float3x4 res;
		res.m11 = m11*other.m11 + m12*other.m21 + m13*other.m31;  
		res.m12 = m11*other.m12 + m12*other.m22 + m13*other.m32;  
		res.m13 = m11*other.m13 + m12*other.m23 + m13*other.m33; 
		res.m14 = m11*other.m14 + m12*other.m24 + m13*other.m34 + m14;
		
		res.m21 = m21*other.m11 + m22*other.m21 + m23*other.m31;  
		res.m22 = m21*other.m12 + m22*other.m22 + m23*other.m32;  
		res.m23 = m21*other.m13 + m22*other.m23 + m23*other.m33; 
		res.m24 = m21*other.m14 + m22*other.m24 + m23*other.m34 + m24;

		res.m31 = m31*other.m11 + m32*other.m21 + m33*other.m31;  
		res.m32 = m31*other.m12 + m32*other.m22 + m33*other.m32;  
		res.m33 = m31*other.m13 + m32*other.m23 + m33*other.m33; 
		res.m34 = m31*other.m14 + m32*other.m24 + m33*other.m34 + m34;

		//res.m41 = m41*other.m11 + m42*other.m21 + m43*other.m31 + m44*other.m41;  
		//res.m42 = m41*other.m12 + m42*other.m22 + m43*other.m32 + m44*other.m42;  
		//res.m43 = m41*other.m13 + m42*other.m23 + m43*other.m33 + m44*other.m43; 
		//res.m44 = m41*other.m14 + m42*other.m24 + m43*other.m34 + m44*other.m44;
		
		return res;
	}

	//! assumes the last line of the matrix implicitly to be (0,0,0,1); and a (0,0,0) translation of other
	inline __device__ __host__ float3x4 operator*(const float3x3 &other) const {
		float3x4 res;
		res.m11 = m11*other.m11 + m12*other.m21 + m13*other.m31;  
		res.m12 = m11*other.m12 + m12*other.m22 + m13*other.m32;  
		res.m13 = m11*other.m13 + m12*other.m23 + m13*other.m33; 
		res.m14 = m14;

		res.m21 = m21*other.m11 + m22*other.m21 + m23*other.m31;  
		res.m22 = m21*other.m12 + m22*other.m22 + m23*other.m32;  
		res.m23 = m21*other.m13 + m22*other.m23 + m23*other.m33; 
		res.m24 = m24;

		res.m31 = m31*other.m11 + m32*other.m21 + m33*other.m31;  
		res.m32 = m31*other.m12 + m32*other.m22 + m33*other.m32;  
		res.m33 = m31*other.m13 + m32*other.m23 + m33*other.m33; 
		res.m34 = m34;

		return res;
	}



	inline __device__ __host__ float& operator()(int i, int j) {
		return entries2[i][j];
	}

	inline __device__ __host__ float operator()(int i, int j) const {
		return entries2[i][j];
	}

	//! returns the translation part of the matrix
	inline __device__ __host__ float3 getTranslation() {
		return make_float3(m14, m24, m34);
	}

	//! sets only the translation part of the matrix (other values remain unchanged)
	inline __device__ __host__ void setTranslation(const float3 &t) {
		m14 = t.x;
		m24 = t.y;
		m34 = t.z;
	}

	//! returns the 3x3 part of the matrix
	inline __device__ __host__ float3x3 getFloat3x3() {
		float3x3 ret;
		ret.m11 = m11;	ret.m12 = m12;	ret.m13 = m13;
		ret.m21 = m21;	ret.m22 = m22;	ret.m23 = m23;
		ret.m31 = m31;	ret.m32 = m32;	ret.m33 = m33;
		return ret;
	}

	//! sets the 3x3 part of the matrix (other values remain unchanged)
	inline __device__ __host__ void setFloat3x3(const float3x3 &other) {
		m11 = other.m11;	m12 = other.m12;	m13 = other.m13;
		m21 = other.m21;	m22 = other.m22;	m23 = other.m23;
		m31 = other.m31;	m32 = other.m32;	m33 = other.m33;
	}

	//! inverts the matrix
	inline __device__ __host__ void inverse() {
		*this = getInverse();
	}

	//! computes the inverse of the matrix
	inline __device__ __host__ float3x4 getInverse() {
		float3x3 A = getFloat3x3();
		A.invert();
		float3 t = getTranslation();
		t = A*t;

		float3x4 ret;
		ret.setFloat3x3(A);
		ret.setTranslation(make_float3(-t.x, -t.y, -t.z));	//float3 doesn't have unary '-'... thank you cuda
		return ret;
	}

	//! prints the matrix; only host	
	__host__ void print() {
		std::cout <<
			m11 << " " << m12 << " " << m13 << " " << m14 << std::endl <<
			m21 << " " << m22 << " " << m23 << " " << m24 << std::endl <<
			m31 << " " << m32 << " " << m33 << " " << m34 << std::endl <<
			std::endl;
	}

	inline __device__ __host__ const float* ptr() const {
		return entries;
	}
	inline __device__ __host__ float* ptr() {
		return entries;
	}

	union {
		struct {
			float m11; float m12; float m13; float m14;
			float m21; float m22; float m23; float m24;
			float m31; float m32; float m33; float m34;
		};
		float entries[9];
		float entries2[3][4];
	};
};



class float4x4 {
public:
	inline __device__ __host__ float4x4() {

	}
	inline __device__ __host__ float4x4(const float values[16]) {
		m11 = values[0];	m12 = values[1];	m13 = values[2];	m14 = values[3];
		m21 = values[4];	m22 = values[5];	m23 = values[6];	m24 = values[7];
		m31 = values[8];	m32 = values[9];	m33 = values[10];	m34 = values[11];
		m41 = values[12];	m42 = values[13];	m43 = values[14];	m44 = values[15];
	}

	inline __device__ __host__ float4x4(const float4x4& other) {
		m11 = other.m11;	m12 = other.m12;	m13 = other.m13;	m14 = other.m14;
		m21 = other.m21;	m22 = other.m22;	m23 = other.m23;	m24 = other.m24;
		m31 = other.m31;	m32 = other.m32;	m33 = other.m33;	m34 = other.m34;
		m41 = other.m41;	m42 = other.m42;	m43 = other.m43;	m44 = other.m44;
	}

	//implicitly assumes last line to (0,0,0,1)
	inline __device__ __host__ float4x4(const float3x4& other) {
		m11 = other.m11;	m12 = other.m12;	m13 = other.m13;	m14 = other.m14;
		m21 = other.m21;	m22 = other.m22;	m23 = other.m23;	m24 = other.m24;
		m31 = other.m31;	m32 = other.m32;	m33 = other.m33;	m34 = other.m34;
		m41 = 0.0f;			m42 = 0.0f;			m43 = 0.0f;			m44 = 1.0f;
	}

	inline __device__ __host__ float4x4(const float3x3& other) {
		m11 = other.m11;	m12 = other.m12;	m13 = other.m13;	m14 = 0.0f;
		m21 = other.m21;	m22 = other.m22;	m23 = other.m23;	m24 = 0.0f;
		m31 = other.m31;	m32 = other.m32;	m33 = other.m33;	m34 = 0.0f;
		m41 = 0.0f;			m42 = 0.0f;			m43 = 0.0f;			m44 = 1.0f;
	}

	inline __device__ __host__ float4x4 operator=(const float4x4 &other) {
		m11 = other.m11;	m12 = other.m12;	m13 = other.m13;	m14 = other.m14;
		m21 = other.m21;	m22 = other.m22;	m23 = other.m23;	m24 = other.m24;
		m31 = other.m31;	m32 = other.m32;	m33 = other.m33;	m34 = other.m34;
		m41 = other.m41;	m42 = other.m42;	m43 = other.m43;	m44 = other.m44;
		return *this;
	}

	inline __device__ __host__ float4x4 operator=(const float3x4 &other) {
		m11 = other.m11;	m12 = other.m12;	m13 = other.m13;	m14 = other.m14;
		m21 = other.m21;	m22 = other.m22;	m23 = other.m23;	m24 = other.m24;
		m31 = other.m31;	m32 = other.m32;	m33 = other.m33;	m34 = other.m34;
		m41 = 0.0f;			m42 = 0.0f;			m43 = 0.0f;			m44 = 1.0f;
		return *this;
	}

	inline __device__ __host__ float4x4& operator=(const float3x3 &other) {
		m11 = other.m11;	m12 = other.m12;	m13 = other.m13;	m14 = 0.0f;
		m21 = other.m21;	m22 = other.m22;	m23 = other.m23;	m24 = 0.0f;
		m31 = other.m31;	m32 = other.m32;	m33 = other.m33;	m34 = 0.0f;
		m41 = 0.0f;			m42 = 0.0f;			m43 = 0.0f;			m44 = 1.0f;
		return *this;
	}


	//! not tested
	inline __device__ __host__ float4x4 operator*(const float4x4 &other) const {
		float4x4 res;
		res.m11 = m11*other.m11 + m12*other.m21 + m13*other.m31 + m14*other.m41;  
		res.m12 = m11*other.m12 + m12*other.m22 + m13*other.m32 + m14*other.m42;  
		res.m13 = m11*other.m13 + m12*other.m23 + m13*other.m33 + m14*other.m43; 
		res.m14 = m11*other.m14 + m12*other.m24 + m13*other.m34 + m14*other.m44;

		res.m21 = m21*other.m11 + m22*other.m21 + m23*other.m31 + m24*other.m41;  
		res.m22 = m21*other.m12 + m22*other.m22 + m23*other.m32 + m24*other.m42;  
		res.m23 = m21*other.m13 + m22*other.m23 + m23*other.m33 + m24*other.m43; 
		res.m24 = m21*other.m14 + m22*other.m24 + m23*other.m34 + m24*other.m44;

		res.m31 = m31*other.m11 + m32*other.m21 + m33*other.m31 + m34*other.m41;  
		res.m32 = m31*other.m12 + m32*other.m22 + m33*other.m32 + m34*other.m42;  
		res.m33 = m31*other.m13 + m32*other.m23 + m33*other.m33 + m34*other.m43; 
		res.m34 = m31*other.m14 + m32*other.m24 + m33*other.m34 + m34*other.m44;

		res.m41 = m41*other.m11 + m42*other.m21 + m43*other.m31 + m44*other.m41;  
		res.m42 = m41*other.m12 + m42*other.m22 + m43*other.m32 + m44*other.m42;  
		res.m43 = m41*other.m13 + m42*other.m23 + m43*other.m33 + m44*other.m43; 
		res.m44 = m41*other.m14 + m42*other.m24 + m43*other.m34 + m44*other.m44;

		return res;
	}

	// untested
	inline __device__ __host__ float4 operator*(const float4& v) const
	{
		return make_float4(
			m11*v.x + m12*v.y + m13*v.z + m14*v.w,
			m21*v.x + m22*v.y + m23*v.z + m24*v.w,
			m31*v.x + m32*v.y + m33*v.z + m34*v.w,
			m41*v.x + m42*v.y + m43*v.z + m44*v.w
			);
	}

	// untested
	//implicitly assumes w to be 1
	inline __device__ __host__ float3 operator*(const float3& v) const
	{
		return make_float3(
			m11*v.x + m12*v.y + m13*v.z + m14*1.0f,
			m21*v.x + m22*v.y + m23*v.z + m24*1.0f,
			m31*v.x + m32*v.y + m33*v.z + m34*1.0f
			);
	}
	
	inline __device__ __host__ float& operator()(int i, int j) {
		return entries2[i][j];
	}

	inline __device__ __host__ float operator()(int i, int j) const {
		return entries2[i][j];
	}


	static inline __device__ __host__  void swap(float& v0, float& v1) {
		float tmp = v0;
		v0 = v1;
		v1 = tmp;
	}

	inline __device__ __host__ void transpose() {
		swap(m12, m21);
		swap(m13, m31);
		swap(m23, m32);
		swap(m41, m14);
		swap(m42, m24);
		swap(m43, m34);
	}
	inline __device__ __host__ float4x4 getTranspose() const {
		float4x4 ret = *this;
		ret.transpose();
		return ret;
	}


	inline __device__ __host__ void invert() {
		*this = getInverse();
	}

	//! return the inverse matrix; but does not change the current matrix
	inline __device__ __host__ float4x4 getInverse() const {
		float inv[16];

		inv[0] = entries[5]  * entries[10] * entries[15] - 
			entries[5]  * entries[11] * entries[14] - 
			entries[9]  * entries[6]  * entries[15] + 
			entries[9]  * entries[7]  * entries[14] +
			entries[13] * entries[6]  * entries[11] - 
			entries[13] * entries[7]  * entries[10];

		inv[4] = -entries[4]  * entries[10] * entries[15] + 
			entries[4]  * entries[11] * entries[14] + 
			entries[8]  * entries[6]  * entries[15] - 
			entries[8]  * entries[7]  * entries[14] - 
			entries[12] * entries[6]  * entries[11] + 
			entries[12] * entries[7]  * entries[10];

		inv[8] = entries[4]  * entries[9] * entries[15] - 
			entries[4]  * entries[11] * entries[13] - 
			entries[8]  * entries[5] * entries[15] + 
			entries[8]  * entries[7] * entries[13] + 
			entries[12] * entries[5] * entries[11] - 
			entries[12] * entries[7] * entries[9];

		inv[12] = -entries[4]  * entries[9] * entries[14] + 
			entries[4]  * entries[10] * entries[13] +
			entries[8]  * entries[5] * entries[14] - 
			entries[8]  * entries[6] * entries[13] - 
			entries[12] * entries[5] * entries[10] + 
			entries[12] * entries[6] * entries[9];

		inv[1] = -entries[1]  * entries[10] * entries[15] + 
			entries[1]  * entries[11] * entries[14] + 
			entries[9]  * entries[2] * entries[15] - 
			entries[9]  * entries[3] * entries[14] - 
			entries[13] * entries[2] * entries[11] + 
			entries[13] * entries[3] * entries[10];

		inv[5] = entries[0]  * entries[10] * entries[15] - 
			entries[0]  * entries[11] * entries[14] - 
			entries[8]  * entries[2] * entries[15] + 
			entries[8]  * entries[3] * entries[14] + 
			entries[12] * entries[2] * entries[11] - 
			entries[12] * entries[3] * entries[10];

		inv[9] = -entries[0]  * entries[9] * entries[15] + 
			entries[0]  * entries[11] * entries[13] + 
			entries[8]  * entries[1] * entries[15] - 
			entries[8]  * entries[3] * entries[13] - 
			entries[12] * entries[1] * entries[11] + 
			entries[12] * entries[3] * entries[9];

		inv[13] = entries[0]  * entries[9] * entries[14] - 
			entries[0]  * entries[10] * entries[13] - 
			entries[8]  * entries[1] * entries[14] + 
			entries[8]  * entries[2] * entries[13] + 
			entries[12] * entries[1] * entries[10] - 
			entries[12] * entries[2] * entries[9];

		inv[2] = entries[1]  * entries[6] * entries[15] - 
			entries[1]  * entries[7] * entries[14] - 
			entries[5]  * entries[2] * entries[15] + 
			entries[5]  * entries[3] * entries[14] + 
			entries[13] * entries[2] * entries[7] - 
			entries[13] * entries[3] * entries[6];

		inv[6] = -entries[0]  * entries[6] * entries[15] + 
			entries[0]  * entries[7] * entries[14] + 
			entries[4]  * entries[2] * entries[15] - 
			entries[4]  * entries[3] * entries[14] - 
			entries[12] * entries[2] * entries[7] + 
			entries[12] * entries[3] * entries[6];

		inv[10] = entries[0]  * entries[5] * entries[15] - 
			entries[0]  * entries[7] * entries[13] - 
			entries[4]  * entries[1] * entries[15] + 
			entries[4]  * entries[3] * entries[13] + 
			entries[12] * entries[1] * entries[7] - 
			entries[12] * entries[3] * entries[5];

		inv[14] = -entries[0]  * entries[5] * entries[14] + 
			entries[0]  * entries[6] * entries[13] + 
			entries[4]  * entries[1] * entries[14] - 
			entries[4]  * entries[2] * entries[13] - 
			entries[12] * entries[1] * entries[6] + 
			entries[12] * entries[2] * entries[5];

		inv[3] = -entries[1] * entries[6] * entries[11] + 
			entries[1] * entries[7] * entries[10] + 
			entries[5] * entries[2] * entries[11] - 
			entries[5] * entries[3] * entries[10] - 
			entries[9] * entries[2] * entries[7] + 
			entries[9] * entries[3] * entries[6];

		inv[7] = entries[0] * entries[6] * entries[11] - 
			entries[0] * entries[7] * entries[10] - 
			entries[4] * entries[2] * entries[11] + 
			entries[4] * entries[3] * entries[10] + 
			entries[8] * entries[2] * entries[7] - 
			entries[8] * entries[3] * entries[6];

		inv[11] = -entries[0] * entries[5] * entries[11] + 
			entries[0] * entries[7] * entries[9] + 
			entries[4] * entries[1] * entries[11] - 
			entries[4] * entries[3] * entries[9] - 
			entries[8] * entries[1] * entries[7] + 
			entries[8] * entries[3] * entries[5];

		inv[15] = entries[0] * entries[5] * entries[10] - 
			entries[0] * entries[6] * entries[9] - 
			entries[4] * entries[1] * entries[10] + 
			entries[4] * entries[2] * entries[9] + 
			entries[8] * entries[1] * entries[6] - 
			entries[8] * entries[2] * entries[5];

		float matrixDet = entries[0] * inv[0] + entries[1] * inv[4] + entries[2] * inv[8] + entries[3] * inv[12];

		float matrixDetr = 1.0f / matrixDet;

		float4x4 res;
		for (unsigned int i = 0; i < 16; i++) {
			res.entries[i] = inv[i] * matrixDetr;
		}
		return res;

	}





	//! returns the 3x3 part of the matrix
	inline __device__ __host__ float3x3 getFloat3x3() {
		float3x3 ret;
		ret.m11 = m11;	ret.m12 = m12;	ret.m13 = m13;
		ret.m21 = m21;	ret.m22 = m22;	ret.m23 = m23;
		ret.m31 = m31;	ret.m32 = m32;	ret.m33 = m33;
		return ret;
	}

	//! sets the 3x3 part of the matrix (other values remain unchanged)
	inline __device__ __host__ void setFloat3x3(const float3x3 &other) {
		m11 = other.m11;	m12 = other.m12;	m13 = other.m13;
		m21 = other.m21;	m22 = other.m22;	m23 = other.m23;
		m31 = other.m31;	m32 = other.m32;	m33 = other.m33;
	}

	//! sets the 4x4 part of the matrix to identity
	inline __device__ __host__ void setIdentity()
	{
		m11 = 1.0f;	m12 = 0.0f;	m13 = 0.0f;	m14 = 0.0f;
		m21 = 0.0f;	m22 = 1.0f;	m23 = 0.0f;	m24 = 0.0f;
		m31 = 0.0f;	m32 = 0.0f;	m33 = 1.0f;	m34 = 0.0f;
		m41 = 0.0f;	m42 = 0.0f;	m43 = 0.0f;	m44 = 1.0f;
	}

	//! sets the 4x4 part of the matrix to identity
	inline __device__ __host__ void setValue(float v)
	{
		m11 = v;	m12 = v;	m13 = v;	m14 = v;
		m21 = v;	m22 = v;	m23 = v;	m24 = v;
		m31 = v;	m32 = v;	m33 = v;	m34 = v;
		m41 = v;	m42 = v;	m43 = v;	m44 = v;
	}

	//! returns the 3x4 part of the matrix
	inline __device__ __host__ float3x4 getFloat3x4() {
		float3x4 ret;
		ret.m11 = m11;	ret.m12 = m12;	ret.m13 = m13;	ret.m14 = m14;
		ret.m21 = m21;	ret.m22 = m22;	ret.m23 = m23;	ret.m24 = m24;
		ret.m31 = m31;	ret.m32 = m32;	ret.m33 = m33;	ret.m34 = m34;
		return ret;
	}

	//! sets the 3x4 part of the matrix (other values remain unchanged)
	inline __device__ __host__ void setFloat3x4(const float3x4 &other) {
		m11 = other.m11;	m12 = other.m12;	m13 = other.m13;	m14 = other.m14;
		m21 = other.m21;	m22 = other.m22;	m23 = other.m23;	m24 = other.m24;
		m31 = other.m31;	m32 = other.m32;	m33 = other.m33;	m34 = other.m34;
	}




	inline __device__ __host__ const float* ptr() const {
		return entries;
	}
	inline __device__ __host__ float* ptr() {
		return entries;
	}

	union {
		struct {
			float m11; float m12; float m13; float m14;
			float m21; float m22; float m23; float m24;
			float m31; float m32; float m33; float m34;
			float m41; float m42; float m43; float m44;
		};
		float entries[16];
		float entries2[4][4];
	};
};



//////////////////////////////
// matNxM
//////////////////////////////

template<unsigned int N, unsigned int M>
class matNxM
{
	public:

		//////////////////////////////
		// Initialization
		//////////////////////////////
		inline __device__ __host__ matNxM()
		{
		}

		inline __device__ __host__ matNxM(float* values)
		{
			__CONDITIONAL_UNROLL__
			for(unsigned int i = 0; i<N*M; i++) entries[i] = values[i];
		}

		inline __device__ __host__ matNxM(const float* values)
		{
			__CONDITIONAL_UNROLL__
			for(unsigned int i = 0; i<N*M; i++) entries[i] = values[i];
		}

		inline __device__ __host__ matNxM(const matNxM& other)
		{
			(*this) = other;
		}

		inline __device__ __host__ matNxM<N,M>& operator=(const matNxM<N,M>& other)
		{
			__CONDITIONAL_UNROLL__
			for(unsigned int i = 0; i<N*M; i++) entries[i] = other.entries[i];
			return *this;
		}
		
		inline __device__ __host__ void setZero()
		{
			__CONDITIONAL_UNROLL__
			for(unsigned int i = 0; i<N*M; i++) entries[i] = 0.0f;
		}

		inline __device__ __host__ void setIdentity()
		{
			setZero();
			__CONDITIONAL_UNROLL__
			for(unsigned int i = 0; i<min(N, M); i++) entries2D[i][i] = 1.0f;
		}

		static inline __device__ __host__ matNxM<N, M> getIdentity()
		{
			matNxM<N, M> R; R.setIdentity();
			return R;
		}

		//////////////////////////////
		// Conversion
		//////////////////////////////

		// declare generic constructors for compile time checking of matrix size
		template<class B>
		explicit inline __device__ __host__  matNxM(const B& other);

		template<class B>
		explicit inline __device__ __host__  matNxM(const B& other0, const B& other1);

		// declare generic casts for compile time checking of matrix size
		inline __device__ __host__ operator float();
		inline __device__ __host__ operator float2();
		inline __device__ __host__ operator float3();
		inline __device__ __host__ operator float4();

		inline __device__ __host__ operator float2x2();
		inline __device__ __host__ operator float3x3();
		inline __device__ __host__ operator float4x4();

		//////////////////////////////
		// Matrix - Matrix Multiplication
		//////////////////////////////
		template<unsigned int NOther, unsigned int MOther>
		inline __device__ __host__ matNxM<N,MOther> operator*(const matNxM<NOther,MOther>& other) const
		{
			cudaAssert(M == NOther);
			matNxM<N,MOther> res;

			__CONDITIONAL_UNROLL__
			for(unsigned int i = 0; i<N; i++)
			{
				__CONDITIONAL_UNROLL__
				for(unsigned int j = 0; j<MOther; j++)
				{
					float sum = 0.0f;
					__CONDITIONAL_UNROLL__
					for(unsigned int k = 0; k<M; k++)
					{
						sum += (*this)(i, k)*other(k, j);
					}

					res(i, j) = sum;
				}
			}

			return res;
		}

		//////////////////////////////
		// Matrix - Inversion
		//////////////////////////////

		inline __device__ __host__ float det() const;
		inline __device__ __host__  matNxM<N, M> getInverse() const;

		//////////////////////////////
		// Matrix - Transpose
		//////////////////////////////
		inline __device__ __host__ matNxM<M,N> getTranspose() const
		{
			matNxM<M,N> res;

			__CONDITIONAL_UNROLL__
			for(unsigned int i = 0; i<M; i++)
			{
				__CONDITIONAL_UNROLL__
				for(unsigned int j = 0; j<N; j++)
				{
					res(i, j) = (*this)(j, i);
				}
			}

			return res;
		}

		inline __device__ void printCUDA() const
		{
			__CONDITIONAL_UNROLL__
			for(unsigned int i = 0; i<N; i++)
			{
				__CONDITIONAL_UNROLL__
				for(unsigned int j = 0; j<M; j++)
				{
					printf("%f ", (*this)(i, j));
				}
				printf("\n");
			}
		}

		inline __device__ bool checkMINF() const
		{
			__CONDITIONAL_UNROLL__
			for(unsigned int i = 0; i<N; i++)
			{
				__CONDITIONAL_UNROLL__
				for(unsigned int j = 0; j<M; j++)
				{
					if((*this)(i, j) == MINF) return true;
				}
			}

			return false;
		}

		inline __device__ bool checkINF() const
		{
			__CONDITIONAL_UNROLL__
			for(unsigned int i = 0; i<N; i++)
			{
				__CONDITIONAL_UNROLL__
				for(unsigned int j = 0; j<M; j++)
				{
					if((*this)(i, j) == INF) return true;
				}
			}

			return false;
		}

		inline __device__ bool checkQNAN() const
		{
			__CONDITIONAL_UNROLL__
			for(unsigned int i = 0; i<N; i++)
			{
				__CONDITIONAL_UNROLL__
				for(unsigned int j = 0; j<M; j++)
				{
					if((*this)(i, j) != (*this)(i, j)) return true;
				}
			}

			return false;
		}

		//////////////////////////////
		// Matrix - Matrix Addition
		//////////////////////////////
		inline __device__ __host__ matNxM<N,M> operator+(const matNxM<N,M>& other) const
		{
			matNxM<N,M> res = (*this);
			res+=other;
			return res;
		}

		inline __device__ __host__ matNxM<N,M>& operator+=(const matNxM<N,M>& other)
		{
			__CONDITIONAL_UNROLL__
			for(unsigned int i = 0; i<N*M; i++) entries[i] += other.entries[i];
			return (*this);
		}

		//////////////////////////////
		// Matrix - Negation
		//////////////////////////////
		inline __device__ __host__ matNxM<N,M> operator-() const
		{
			matNxM<N,M> res = (*this)*(-1.0f);
			return res;
		}

		//////////////////////////////
		// Matrix - Matrix Subtraction
		//////////////////////////////
		inline __device__ __host__ matNxM<N,M> operator-(const matNxM<N,M>& other) const
		{
			matNxM<N,M> res = (*this);
			res-=other;
			return res;
		}

		inline __device__ __host__ matNxM<N,M>& operator-=(const matNxM<N,M>& other)
		{
			__CONDITIONAL_UNROLL__
			for(unsigned int i = 0; i<N*M; i++) entries[i] -= other.entries[i];
			return (*this);
		}

		//////////////////////////////
		// Matrix - Scalar Multiplication
		//////////////////////////////
		inline __device__ __host__ matNxM<N,M> operator*(const float t) const
		{
			matNxM<N,M> res = (*this);
			res*=t;
			return res;
		}

		inline __device__ __host__ matNxM<N, M>& operator*=(const float t)
		{
			__CONDITIONAL_UNROLL__
			for(unsigned int i = 0; i<N*M; i++) entries[i] *= t;
			return (*this);
		}

		//////////////////////////////
		// Matrix - Scalar Division
		//////////////////////////////
		inline __device__ __host__ matNxM<N, M> operator/(const float t) const
		{
			matNxM<N, M> res = (*this);
			res/=t;
			return res;
		}

		inline __device__ __host__ matNxM<N, M>& operator/=(const float t)
		{
			__CONDITIONAL_UNROLL__
			for(unsigned int i = 0; i<N*M; i++) entries[i] /= t;
			return (*this);
		}

		//////////////////////////
		// Element Access
		//////////////////////////
		inline __device__ __host__ unsigned int nRows()
		{
			return N;
		}

		inline __device__ __host__ unsigned int nCols()
		{
			return M;
		}

		inline __device__ __host__ float& operator()(unsigned int i, unsigned int j)
		{
			cudaAssert(i<N && j<M);
			return entries2D[i][j];
		}

		inline __device__ __host__ float operator()(unsigned int i, unsigned int j) const
		{
			cudaAssert(i<N && j<M);
			return entries2D[i][j];
		}

		inline __device__ __host__ float& operator()(unsigned int i)
		{
			cudaAssert(i<N*M);
			return entries[i];
		}

		inline __device__ __host__ float operator()(unsigned int i) const
		{
			cudaAssert(i<N*M);
			return entries[i];
		}

		template<unsigned int NOther, unsigned int MOther>
		inline __device__ __host__ void getBlock(unsigned int xStart, unsigned int yStart, matNxM<NOther, MOther>& res) const
		{
			cudaAssert(xStart+NOther <= N && yStart+MOther <= M);
			
			__CONDITIONAL_UNROLL__
			for(unsigned int i = 0; i<NOther; i++)
			{
				__CONDITIONAL_UNROLL__
				for(unsigned int j = 0; j<MOther; j++)
				{
					res(i, j) = (*this)(xStart+i, yStart+j);
				}
			}
		}

		template<unsigned int NOther, unsigned int MOther>
		inline __device__ __host__ void setBlock(matNxM<NOther, MOther>& input, unsigned int xStart, unsigned int yStart)
		{
			cudaAssert(xStart+NOther <= N && yStart+MOther <= M);
			
			__CONDITIONAL_UNROLL__
			for(unsigned int i = 0; i<NOther; i++)
			{
				__CONDITIONAL_UNROLL__
				for(unsigned int j = 0; j<MOther; j++)
				{
					(*this)(xStart+i, yStart+j) = input(i, j);
				}
			}
		}

		inline __device__ __host__ const float* ptr() const {
			return entries;
		}
		inline __device__ __host__ float* ptr() {
			return entries;
		}

		// Operators

		inline __device__ __host__ float norm1DSquared() const
		{
			cudaAssert(M==1 || N==1);

			float sum = 0.0f;
			for(unsigned int i = 0; i<max(N, M); i++) sum += entries[i]*entries[i];

			return sum;
		}

		inline __device__ __host__ float norm1D() const
		{
			return sqrt(norm1DSquared());
		}

	private:

		union
		{
			float entries[N*M];
			float entries2D[N][M];
		};
};

//////////////////////////////
// Scalar - Matrix Multiplication
//////////////////////////////
template<unsigned int N, unsigned int M>
inline __device__ __host__ matNxM<N,M> operator*(const float t, const matNxM<N, M>& mat)
{
	matNxM<N,M> res = mat;
	res*=t;
	return res;
}

//////////////////////////////
// Matrix Inversion
//////////////////////////////

template<>
inline __device__ __host__ float  matNxM<3, 3>::det() const
{
	const float& m11 = entries2D[0][0];
	const float& m12 = entries2D[0][1];
	const float& m13 = entries2D[0][2];

	const float& m21 = entries2D[1][0];
	const float& m22 = entries2D[1][1];
	const float& m23 = entries2D[1][2];

	const float& m31 = entries2D[2][0];
	const float& m32 = entries2D[2][1];
	const float& m33 = entries2D[2][2];

	return m11*m22*m33 + m12*m23*m31 + m13*m21*m32 - m31*m22*m13 - m32*m23*m11 - m33*m21*m12;
}

template<>
inline __device__ __host__ matNxM<3, 3> matNxM<3, 3>::getInverse() const
{
	matNxM<3, 3> res;
	res.entries[0] = entries[4]*entries[8] - entries[5]*entries[7];
	res.entries[1] = -entries[1]*entries[8] + entries[2]*entries[7];
	res.entries[2] = entries[1]*entries[5] - entries[2]*entries[4];

	res.entries[3] = -entries[3]*entries[8] + entries[5]*entries[6];
	res.entries[4] = entries[0]*entries[8] - entries[2]*entries[6];
	res.entries[5] = -entries[0]*entries[5] + entries[2]*entries[3];

	res.entries[6] = entries[3]*entries[7] - entries[4]*entries[6];
	res.entries[7] = -entries[0]*entries[7] + entries[1]*entries[6];
	res.entries[8] = entries[0]*entries[4] - entries[1]*entries[3];
	return res*(1.0f/det());
}

template<>
inline __device__ __host__ float matNxM<2, 2>::det() const
{
	return (*this)(0, 0)*(*this)(1, 1)-(*this)(1, 0)*(*this)(0, 1);
}

template<>
inline __device__ __host__ matNxM<2, 2> matNxM<2, 2>::getInverse() const
{
	matNxM<2, 2> res;
	res(0, 0) =  (*this)(1, 1); res(0, 1) = -(*this)(0, 1);
	res(1, 0) = -(*this)(1, 0); res(1, 1) =  (*this)(0, 0);

	return res*(1.0f/det());
}

//////////////////////////////
// Conversion
//////////////////////////////

// To Matrix from floatNxN
template<>
template<>
inline __device__ __host__  matNxM<1, 1>::matNxM(const float& other)
{
	entries[0] = other;
}

// To Matrix from floatNxN
template<>
template<>
inline __device__ __host__  matNxM<2, 2>::matNxM(const float2x2& other)
{
	__CONDITIONAL_UNROLL__
	for(unsigned int i = 0; i<4; i++) entries[i] = other.entries[i];
}

template<>
template<>
inline __device__ __host__  matNxM<3, 3>::matNxM(const float3x3& other)
{
	__CONDITIONAL_UNROLL__
	for(unsigned int i = 0; i<9; i++) entries[i] = other.entries[i];
}

template<>
template<>
inline __device__ __host__  matNxM<4, 4>::matNxM(const float4x4& other)
{
	__CONDITIONAL_UNROLL__
	for(unsigned int i = 0; i<16; i++) entries[i] = other.entries[i];
}

template<>
template<>
inline __device__ __host__ matNxM<3, 2>::matNxM(const float3& col0, const float3& col1)
{
	entries2D[0][0] = col0.x; entries2D[0][1] = col1.x;
	entries2D[1][0] = col0.y; entries2D[1][1] = col1.y;
	entries2D[2][0] = col0.z; entries2D[2][1] = col1.z;
}

// To floatNxM from Matrix
template<>
inline __device__ __host__ matNxM<4, 4>::operator float4x4()
{
	float4x4 res;
	__CONDITIONAL_UNROLL__
	for(unsigned int i = 0; i<16; i++) res.entries[i] = entries[i];
	return res;
}

template<>
inline __device__ __host__ matNxM<3, 3>::operator float3x3()
{
	float3x3 res;
	__CONDITIONAL_UNROLL__
	for(unsigned int i = 0; i<9; i++) res.entries[i] = entries[i];
	return res;
}

template<>
inline __device__ __host__ matNxM<2, 2>::operator float2x2()
{
	float2x2 res;
	__CONDITIONAL_UNROLL__
	for(unsigned int i = 0; i<4; i++) res.entries[i] = entries[i];
	return res;
}

// To Matrix from floatN
template<>
template<>
inline __device__ __host__ matNxM<2, 1>::matNxM(const float2& other)
{
	entries[0] = other.x;
	entries[1] = other.y;
}

template<>
template<>
inline __device__ __host__ matNxM<3, 1>::matNxM(const float3& other)
{
	entries[0] = other.x;
	entries[1] = other.y;
	entries[2] = other.z;
}

template<>
template<>
inline __device__ __host__ matNxM<4, 1>::matNxM(const float4& other)
{
	entries[0] = other.x;
	entries[1] = other.y;
	entries[2] = other.z;
	entries[3] = other.w;
}

// To floatN from Matrix
template<>
inline __device__ __host__ matNxM<1, 1>::operator float()
{
	return entries[0];
}

template<>
inline __device__ __host__ matNxM<2, 1>::operator float2()
{
	return make_float2(entries[0],  entries[1]);
}

template<>
inline __device__ __host__ matNxM<3, 1>::operator float3()
{
	return make_float3(entries[0], entries[1], entries[2]);
}

inline __device__ __host__ matNxM<4, 1>::operator float4()
{
	return make_float4(entries[0],  entries[1], entries[2], entries[3]);
}

//////////////////////////////
// Typedefs
//////////////////////////////

typedef matNxM<9, 3> mat9x3;
typedef matNxM<3, 9> mat3x9;

typedef matNxM<9, 1> mat9x1;
typedef matNxM<1, 9> mat1x9;

typedef matNxM<6, 6> mat6x6;

typedef matNxM<6, 1> mat6x1;
typedef matNxM<1, 6> mat1x6;

typedef matNxM<3, 6> mat3x6;
typedef matNxM<6, 3> mat6x3;

typedef matNxM<4, 4> mat4x4;

typedef matNxM<4, 1> mat4x1;
typedef matNxM<1, 4> mat1x4;

typedef matNxM<3, 3> mat3x3;

typedef matNxM<2, 3> mat2x3;
typedef matNxM<3, 2> mat3x2;

typedef matNxM<2, 2> mat2x2;

typedef matNxM<1, 2> mat1x2;
typedef matNxM<2, 1> mat2x1;

typedef matNxM<1, 3> mat1x3;
typedef matNxM<3, 1> mat3x1;

typedef matNxM<1, 1> mat1x1;


#endif
