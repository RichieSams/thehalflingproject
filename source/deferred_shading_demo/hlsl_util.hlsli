/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#ifndef DEFERRED_SHADING_DEMO_HLSL_UTIL_H
#define DEFERRED_SHADING_DEMO_HLSL_UTIL_H


// Converts a normalized cartesian direction vector
// to spherical coordinates.
float2 CartesianToSpherical(float3 cartesian) {
	float2 spherical;

	spherical.x = atan2(cartesian.y, cartesian.x);
	spherical.y = acos(cartesian.z);

	return spherical;
}

// Converts a spherical coordinate to a normalized
// cartesian direction vector.
float3 SphericalToCartesian(float2 spherical) {
	float2 sinCosTheta, sinCosPhi;

	sincos(spherical.x, sinCosTheta.x, sinCosTheta.y);
	sincos(spherical.y, sinCosPhi.x, sinCosPhi.y);

	return float3(sinCosTheta.y * sinCosPhi.x, sinCosTheta.x * sinCosPhi.x, sinCosPhi.y);
}

// Converts a z-buffer depth to linear depth
float LinearDepth(in float zw, in float4x4 projectionMatrix) {
    return projectionMatrix._43 / (zw - projectionMatrix._33);
}

// Calculates position from a depth value + pixel coordinate
float3 PositionFromDepth(in float zw, in uint2 pixelCoord, in float2 displaySize, in float4x4 invViewProjection) {
    float2 cpos = (pixelCoord + 0.5f) / displaySize;
    cpos *= 2.0f;
    cpos -= 1.0f;
    cpos.y *= -1.0f;
    float4 positionWS = mul(float4(cpos, zw, 1.0f), invViewProjection);
    return positionWS.xyz / positionWS.w;
}


#endif