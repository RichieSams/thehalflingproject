/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#include "types.hlsli"

cbuffer MatrixBuffer
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

VertexOut VS(VertexIn vin) {
	VertexOut vout;

	// Change the position vector to be 4 units for proper matrix calculations.
    vin.position.w = 1.0f;

	matrix worldViewProj = mul(worldMatrix, viewMatrix);
	worldViewProj = mul(worldViewProj, projectionMatrix);
	vout.position = mul(vin.position, worldViewProj);

    //// Transform to homogeneous clip space.
    //vout.position = mul(vin.position, worldMatrix);
    //vout.position = mul(vout.position, viewMatrix);
    //vout.position = mul(vout.position, projectionMatrix);

	vout.color = vin.color;

	return vout;
}