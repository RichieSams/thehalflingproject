/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#include "common/math.h"


namespace Common {

float2 float2::operator+(const float2 &rhs) {
	X += rhs.X;
	Y += rhs.Y;

	return *this;
}

float2 float2::operator-(const float2 &rhs) {
	X -= rhs.X;
	Y -= rhs.Y;

	return *this;
}

float2 float2::operator*(const float2 &rhs) {
	X *= rhs.X;
	Y *= rhs.Y;

	return *this;
}

float2 float2::operator/(const float2 &rhs) {
	X /= rhs.X;
	Y /= rhs.Y;

	return *this;
}

float2 float2::operator+(float rhs) {
	X += rhs;
	Y += rhs;

	return *this;
}

float2 float2::operator-(float rhs) {
	X -= rhs;
	Y -= rhs;

	return *this;
}

float2 float2::operator*(float rhs) {
	X *= rhs;
	Y *= rhs;

	return *this;
}

float2 float2::operator/(float rhs) {
	X /= rhs;
	Y /= rhs;

	return *this;
}

float2 operator+(float lhs, const float2 &rhs) {
	return float2(lhs + rhs.X, lhs + rhs.Y);
}

float2 operator-(float lhs, const float2 &rhs) {
	return float2(lhs - rhs.X, lhs - rhs.Y);
}

float2 operator*(float lhs, const float2 &rhs) {
	return float2(lhs * rhs.X, lhs * rhs.Y);
}

float2 operator/(float lhs, const float2 &rhs) {
	return float2(lhs / rhs.X, lhs / rhs.Y);
}


float3 float3::operator+(const float3 &rhs) {
	X += rhs.X;
	Y += rhs.Y;
	Z += rhs.Z;

	return *this;
}

float3 float3::operator-(const float3 &rhs) {
	X -= rhs.X;
	Y -= rhs.Y;
	Z -= rhs.Z;

	return *this;
}

float3 float3::operator*(const float3 &rhs) {
	X *= rhs.X;
	Y *= rhs.Y;
	Z *= rhs.Z;

	return *this;
}

float3 float3::operator/(const float3 &rhs) {
	X /= rhs.X;
	Y /= rhs.Y;
	Z /= rhs.Z;

	return *this;
}

float3 float3::operator+(float rhs) {
	X += rhs;
	Y += rhs;
	Z += rhs;

	return *this;
}

float3 float3::operator-(float rhs) {
	X -= rhs;
	Y -= rhs;
	Z -= rhs;

	return *this;
}

float3 float3::operator*(float rhs) {
	X *= rhs;
	Y *= rhs;
	Z *= rhs;

	return *this;
}

float3 float3::operator/(float rhs) {
	X /= rhs;
	Y /= rhs;
	Z /= rhs;

	return *this;
}

float3 operator+(float lhs, const float3 &rhs) {
	return float3(lhs + rhs.X, lhs + rhs.Y, lhs + rhs.Z);
}

float3 operator-(float lhs, const float3 &rhs) {
	return float3(lhs - rhs.X, lhs - rhs.Y, lhs - rhs.Z);
}

float3 operator*(float lhs, const float3 &rhs) {
	return float3(lhs * rhs.X, lhs * rhs.Y, lhs * rhs.Z);
}

float3 operator/(float lhs, const float3 &rhs) {
	return float3(lhs / rhs.X, lhs / rhs.Y, lhs / rhs.Z);
}


float4 float4::operator+(const float4 &rhs) {
	X += rhs.X;
	Y += rhs.Y;
	Z += rhs.Z;
	W += rhs.W;

	return *this;
}

float4 float4::operator-(const float4 &rhs) {
	X -= rhs.X;
	Y -= rhs.Y;
	Z -= rhs.Z;
	W -= rhs.W;

	return *this;
}

float4 float4::operator*(const float4 &rhs) {
	X *= rhs.X;
	Y *= rhs.Y;
	Z *= rhs.Z;
	W *= rhs.W;

	return *this;
}

float4 float4::operator/(const float4 &rhs) {
	X /= rhs.X;
	Y /= rhs.Y;
	Z /= rhs.Z;
	W /= rhs.W;

	return *this;
}

float4 float4::operator+(float rhs) {
	X += rhs;
	Y += rhs;
	Z += rhs;
	W += rhs;

	return *this;
}

float4 float4::operator-(float rhs) {
	X -= rhs;
	Y -= rhs;
	Z -= rhs;
	W -= rhs;

	return *this;
}

float4 float4::operator*(float rhs) {
	X *= rhs;
	Y *= rhs;
	Z *= rhs;
	W *= rhs;

	return *this;
}

float4 float4::operator/(float rhs) {
	X /= rhs;
	Y /= rhs;
	Z /= rhs;
	W /= rhs;

	return *this;
}

float4 operator+(float lhs, const float4 &rhs) {
	return float4(lhs + rhs.X, lhs + rhs.Y, lhs + rhs.Z, lhs + rhs.W);
}

float4 operator-(float lhs, const float4 &rhs) {
	return float4(lhs - rhs.X, lhs - rhs.Y, lhs - rhs.Z, lhs - rhs.W);
}

float4 operator*(float lhs, const float4 &rhs) {
	return float4(lhs * rhs.X, lhs * rhs.Y, lhs * rhs.Z, lhs * rhs.W);
}

float4 operator/(float lhs, const float4 &rhs) {
	return float4(lhs / rhs.X, lhs / rhs.Y, lhs / rhs.Z, lhs / rhs.W);
}

} // End of namespace Common
