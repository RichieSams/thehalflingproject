/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#ifndef MATERIALS_SHADER_H
#define MATERIALS_SHADER_H

struct BlinnPhongMaterial {
	float4 Ambient; // w = UseSpecular
	float4 Diffuse;
	float4 Specular; // w = SpecPower
};

#endif
