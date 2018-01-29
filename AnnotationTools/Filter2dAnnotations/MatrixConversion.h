#pragma once

#include "cuda_SimpleMatrixUtil.h"

namespace MatrixConversion
{
	static mat4f toMlib(const float4x4& m) {
		return mat4f(m.ptr());
	}
	static vec4f toMlib(const float4& v) {
		return vec4f(v.x, v.y, v.z, v.w);
	}
	static vec3f toMlib(const float3& v) {
		return vec3f(v.x, v.y, v.z);
	}
	static vec4i toMlib(const int4& v) {
		return vec4i(v.x, v.y, v.z, v.w);
	}
	static vec3i toMlib(const int3& v) {
		return vec3i(v.x, v.y, v.z);
	}
	static float4x4 toCUDA(const mat4f& m) {
		return float4x4(m.getData());
	}

	static float4 toCUDA(const vec4f& v) {
		return make_float4(v.x, v.y, v.z, v.w);
	}
	static float3 toCUDA(const vec3f& v) {
		return make_float3(v.x, v.y, v.z);
	}
	static int4 toCUDA(const vec4i& v) {
		return make_int4(v.x, v.y, v.z, v.w);
	}
	static int3 toCUDA(const vec3i& v) {
		return make_int3(v.x, v.y, v.z);
	}


}
