/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#ifndef COMMON_LIGHT_MANAGER_H
#define COMMON_LIGHT_MANAGER_H

#include "common/lights.h"

#include <unordered_map>


namespace Common {

class LightManager {
public:
	LightManager();

private:
	DirectionalLight m_directionalLight;
	std::unordered_map<uint, PointLight> m_pointLights;
	std::unordered_map<uint, SpotLight> m_spotLights;

	uint m_pointLightCounter;
	uint m_spotLightCounter;

	bool m_needsBufferRebuild;
	
public:
	uint AddPointLight(PointLight &light);
	uint AddSpotLight(SpotLight &light);

	bool RemovePointLight(uint lightId);
	bool RemoveSpotLight(uint lightId);

	inline DirectionalLight *GetDirectionalLight() { return &m_directionalLight; }
	inline const std::unordered_map<uint, PointLight> *GetPointLights() { return &m_pointLights; }
	inline const std::unordered_map<uint, SpotLight> *GetSpotLights() { return &m_spotLights; }

	inline bool NeedsBufferRebuild() { return m_needsBufferRebuild; }
	void ResetBufferRebuildState() { m_needsBufferRebuild = false; }
};


} // End of namespace Common

#endif