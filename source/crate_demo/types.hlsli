/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#ifndef CRATE_DEMO_TYPES_H
#define CRATE_DEMO_TYPES_H

struct VertexIn {
	float4 position		: POSITION;
	float4 color		: COLOR;
};

struct VertexOut {
	float4 position		: SV_POSITION;
	float4 color		: COLOR;
};

#endif
