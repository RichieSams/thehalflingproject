/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#pragma once

#include "common/halfling_sys.h"


namespace Common {

class float2 {
public:
	float2(float x, float y) : X(x), Y(y) {}
	float2() : X(0.0f), Y(0.0f) {}

public:
	float X;
	float Y;

public:
	float2 operator+(const float2 &rhs);
	float2 operator-(const float2 &rhs);
	float2 operator*(const float2 &rhs);
	float2 operator/(const float2 &rhs);

	float2 operator+(float rhs);
	float2 operator-(float rhs);
	float2 operator*(float rhs);
	float2 operator/(float rhs);
};

float2 operator+(float lhs, const float2 &rhs);
float2 operator-(float lhs, const float2 &rhs);
float2 operator*(float lhs, const float2 &rhs);
float2 operator/(float lhs, const float2 &rhs);


class float3 {
public:
	float3(float x, float y, float z) : X(x), Y(y), Z(z) {}
	float3() : X(0.0f), Y(0.0f), Z(0.0f) {}

public:
	float X;
	float Y;
	float Z;

public:
	float3 operator+(const float3 &rhs);
	float3 operator-(const float3 &rhs);
	float3 operator*(const float3 &rhs);
	float3 operator/(const float3 &rhs);

	float3 operator+(float rhs);
	float3 operator-(float rhs);
	float3 operator*(float rhs);
	float3 operator/(float rhs);
};

float3 operator+(float lhs, const float3 &rhs);
float3 operator-(float lhs, const float3 &rhs);
float3 operator*(float lhs, const float3 &rhs);
float3 operator/(float lhs, const float3 &rhs);


class float4 {
public:
	float4(float x, float y, float z, float w) : X(x), Y(y), Z(z), W(w) {}
	float4() : X(0.0f), Y(0.0f), Z(0.0f), W(0.0f) {}

public:
	float X;
	float Y;
	float Z;
	float W;

public:
	float4 operator+(const float4 &rhs);
	float4 operator-(const float4 &rhs);
	float4 operator*(const float4 &rhs);
	float4 operator/(const float4 &rhs);

	float4 operator+(float rhs);
	float4 operator-(float rhs);
	float4 operator*(float rhs);
	float4 operator/(float rhs);
};

float4 operator+(float lhs, const float4 &rhs);
float4 operator-(float lhs, const float4 &rhs);
float4 operator*(float lhs, const float4 &rhs);
float4 operator/(float lhs, const float4 &rhs);


template<typename T>
inline T Lerp(const T &a, const T &b, float t) {
	return a + (b - a) * t;
}

template<typename T>
inline T Clamp(const T &x, const T &low, const T &high) {
	return x < low ? low : (x > high ? high : x);
}

// Returns random float in [0, 1).
static float RandF() {
	return (float)(rand()) / (float)RAND_MAX;
}

// Returns random float in [a, b).
static float RandF(float a, float b) {
	return a + RandF() * (b - a);
}

} // End of namespace Common
