/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#ifndef LIGHT_FUNCTIONS_SHADER_H
#define LIGHT_FUNCTIONS_SHADER_H

#include "graphics/shaders/hlsl_util.hlsli"
#include "graphics/shaders/lights.hlsli"
#include "graphics/shaders/materials.hlsli"

#define PI 3.14159265f
#define PI_RCP 0.31830989f

struct SurfaceProperties {
	float3 DiffuseColor;
	float3 SpecularColor;
	float3 Position;
	float Roughness;
	float3 Normal;
};


// We use old school Lambert
// 
// aka:    f(l, v) = c_diff / PI
float3 Diffuse_BRDF(float3 diffuseColor) {
	return diffuseColor * PI_RCP;
}


// We use the GGX/Trowbridge-Reitz distribution
// It is defined as follows:
//
// D(h) = alpha^2 / (PI * ((NoH)^2 * (alpha^2 - 1) + 1)^2)
// alpha = roughness^2
// 
// We distribute the NoH^2 multiplication and simplify to better fit into asm instructions
//
// D(H) = alpha^2 / (PI * ((NoH * alpha2 - NoH) * NoH + 1)^2)
float Specular_Distribution(float roughness, float NoH) {
	float alpha = roughness * roughness;
	float alpha2 = alpha * alpha;
	float d = (NoH * alpha2 + -NoH) * NoH + 1;    // 2 mad

	return alpha2 / (PI * d * d);                 // 3 mul, 1 rcp
}

// We use the Schlick model, but modify k = alpha / 2, so-as to better fit the GGX distribution
//
// k = (roughness)^2 / 2
// G1(N, X) = NoX * (1 - k) + k
//
// G(N, V, H) = (G1(N, V) * G1(N, H)) / 4
float Specular_GeometricVisibility(float roughness, float NoV, float NoL) {
	float k = roughness * roughness * 0.5f;
	float G1_V = NoV * (1 - k) + k;
	float G1_L = NoL * (1 - k) + k;

	return 0.25f / (G1_V * G1_L);
}

// We use the Schlick model
float3 Specular_Fresnel(float3 specularColor, float VoH ) {
	// Anything less than 2% is physically impossible and is instead considered to be shadowing 
	return specularColor + (saturate(50.0f * specularColor.g) - specularColor) * exp2((-5.55473 * VoH - 6.98316) * VoH);
}


float SpotAttenuation(float3 lightVector, float3 spotDirection, float cosOuterConeAngle, float invCosConeDifference) {
	return Square(saturate((dot(normalize(lightVector), -spotDirection) - cosOuterConeAngle) * invCosConeDifference));
}


void AccumulateCookTorranceDirectionalLight(DirectionalLight light, SurfaceProperties surfProps, float3 toEye, inout float4 outColor) {
	float3 L = float3(light.Direction.x, -light.Direction.y, -light.Direction.z);
	float3 H = normalize(toEye + L);
	float3 N = surfProps.Normal;

	// Compute / define all the needed parameters
	float NoL = saturate(dot(N, L));
	float NoV = saturate(dot(N, toEye));
	float NoH = saturate(dot(N, H));
	float VoH = saturate(dot(toEye, H));

	float roughness = surfProps.Roughness;

	// Compute the diffuse BRDF value
	float3 diffuseBRDFValue = Diffuse_BRDF(surfProps.DiffuseColor);

	// Compute the specular BRDF value
	float specularDistribution = Specular_Distribution(roughness, NoH);
	float specularGeomVisibility = Specular_GeometricVisibility(roughness, NoV, NoL);
	float3 specularFresnel = Specular_Fresnel(surfProps.SpecularColor, VoH);

	float3 specularBRDFValue = (specularDistribution * specularGeomVisibility) * specularFresnel;

	// Composite with the final color
	outColor += float4(light.Irradiance, 1.0f) * (NoL * (float4(diffuseBRDFValue, 1.0f) + float4(specularBRDFValue, 1.0f)));
}

void AccumulateCookTorrancePointLight(PointLight light, SurfaceProperties surfProps, float3 toEye, inout float4 outColor) {
	// The vector from the surface to the light
	float3 L = normalize(light.Position - surfProps.Position);
	float sqrDistance = dot(L, L);

	float3 H = normalize(toEye + L);
	float3 N = surfProps.Normal;

	// Compute / define all the needed parameters
	float NoL = saturate(dot(N, L));
	float NoV = saturate(dot(N, toEye));
	float NoH = saturate(dot(N, H));
	float VoH = saturate(dot(toEye, H));

	float roughness = surfProps.Roughness;

	// Compute the diffuse BRDF value
	float3 diffuseBRDFValue = Diffuse_BRDF(surfProps.DiffuseColor);

	// Compute the specular BRDF value
	float specularDistribution = Specular_Distribution(roughness, NoH);
	float specularGeomVisibility = Specular_GeometricVisibility(roughness, NoV, NoL);
	float3 specularFresnel = Specular_Fresnel(surfProps.SpecularColor, VoH);

	float3 specularBRDFValue = (specularDistribution * specularGeomVisibility) * specularFresnel;

	// True attenuation is just 1/(d^2) , but the +1 prevents the equation from getting really huge when
	// the light is really close to a surface
	float attenuation = 1 / (sqrDistance + 1);

	// This term gracefully clamps the radiance to zero at the extent of the light
	// This is not physical, but necessary for real-time
	float lightRadiusMask = Square(saturate(1 - Square(sqrDistance * Square(light.InvRange))));

	// Composite with the final color
	outColor += float4(light.Irradiance, 1.0f) * ((attenuation * lightRadiusMask * NoL) * (float4(diffuseBRDFValue, 1.0f) + float4(specularBRDFValue, 1.0f)));
}

void AccumulateCookTorranceSpotLight(SpotLight light, SurfaceProperties surfProps, float3 toEye, inout float4 outColor) {
	// The vector from the surface to the light
	float3 L = normalize(light.Position - surfProps.Position);
	float sqrDistance = dot(L, L);

	float3 H = normalize(toEye + L);
	float3 N = surfProps.Normal;

	// Compute / define all the needed parameters
	float NoL = saturate(dot(N, L));
	float NoV = saturate(dot(N, toEye));
	float NoH = saturate(dot(N, H));
	float VoH = saturate(dot(toEye, H));

	float roughness = surfProps.Roughness;

	// Compute the diffuse BRDF value
	float3 diffuseBRDFValue = Diffuse_BRDF(surfProps.DiffuseColor);

	// Compute the specular BRDF value
	float specularDistribution = Specular_Distribution(roughness, NoH);
	float specularGeomVisibility = Specular_GeometricVisibility(roughness, NoV, NoL);
	float3 specularFresnel = Specular_Fresnel(surfProps.SpecularColor, VoH);

	float3 specularBRDFValue = (specularDistribution * specularGeomVisibility) * specularFresnel;

	// True attenuation is just 1/(d^2) , but the +1 prevents the equation from getting really huge when
	// the light is really close to a surface
	float attenuation = 1 / (sqrDistance + 1);

	// This term gracefully clamps the radiance to zero at the extent of the light
	// This is not physical, but necessary for real-time
	float lightRadiusMask = Square(saturate(1 - Square(sqrDistance * Square(light.InvRange))));

	// The attenuation of the spot
	float spotAttenuation = SpotAttenuation(L, light.Direction, light.CosOuterConeAngle, light.InvCosConeDifference);

	// Composite with the final color
	outColor += float4(light.Irradiance, 1.0f) * ((attenuation * lightRadiusMask * spotAttenuation * NoL) * (float4(diffuseBRDFValue, 1.0f) + float4(specularBRDFValue, 1.0f)));
}





//void AccumulateBlinnPhongDirectionalLight(DirectionalLight light, SurfaceProperties surfProps, float3 toEye, inout float4 diffuse, inout float4 spec) {
//	float diffuseFactor = dot(-light.Direction, surfProps.normal);
//
//	[flatten]
//	if (diffuseFactor > 0.0f) {
//		diffuse += diffuseFactor * (surfProps.diffuseAlbedo * light.Diffuse);
//
//		float3 v = reflect(light.Direction, surfProps.normal);
//		float specFactor = pow(max(dot(v, toEye), 0.0f), surfProps.specAlbedoAndPower.w);
//
//		spec += specFactor * (float4(surfProps.specAlbedoAndPower.xyz, 1.0f) * light.Specular);
//	}
//}
//
//void AccumulateBlinnPhongPointLight(PointLight light, SurfaceProperties surfProps, float3 toEye, inout float4 diffuse, inout float4 spec) {
//	// The vector from the surface to the light
//	float3 lightVector = light.Position - surfProps.position;
//	float distance = length(lightVector);
//
//	if (distance > light.Range)
//		return;
//
//	// Normalize the light vector
//	lightVector /= distance;
//
//	float diffuseFactor = dot(lightVector, surfProps.normal);
//
//	[flatten]
//	if (diffuseFactor > 0.0f) {
//		float attenuation = 1.0f - smoothstep(light.Range * light.AttenuationDistanceUNorm, light.Range, distance);
//
//		diffuse += (attenuation * diffuseFactor) * (surfProps.diffuseAlbedo * light.Diffuse);
//
//		float3 v = reflect(-lightVector, surfProps.normal);
//		float specFactor = pow(max(dot(v, toEye), 0.0f), surfProps.specAlbedoAndPower.w);
//
//		spec += (attenuation * specFactor) * (float4(surfProps.specAlbedoAndPower.xyz, 1.0f) * light.Specular);
//	}
//}
//
//void AccumulateBlinnPhongSpotLight(SpotLight light, SurfaceProperties surfProps, float3 toEye, inout float4 diffuse, inout float4 spec) {
//	// The vector from the surface to the light
//	float3 lightVector = light.Position - surfProps.position;
//	float distance = length(lightVector);
//
//	if (distance > light.Range)
//		return;
//
//	// Normalize the light vector
//	lightVector /= distance;
//
//	float diffuseFactor = dot(lightVector, surfProps.normal);
//
//	[flatten]
//	if (diffuseFactor > 0.0f) {
//		float currentAngle = dot(-lightVector, light.Direction);
//		float spot = 1.0f - smoothstep(light.CosInnerConeAngle, light.CosOuterConeAngle, currentAngle);
//		float attenuation = spot * (1.0f - smoothstep(light.Range * light.AttenuationDistanceUNorm, light.Range, distance));
//
//		diffuse += (attenuation * diffuseFactor) * (surfProps.diffuseAlbedo * light.Diffuse);
//
//		float3 v = reflect(-lightVector, surfProps.normal);
//		float specFactor = pow(max(dot(v, toEye), 0.0f), surfProps.specAlbedoAndPower.w);
//		
//		spec += (attenuation * specFactor) * (float4(surfProps.specAlbedoAndPower.xyz, 1.0f) * light.Specular);
//	}
//}

#endif