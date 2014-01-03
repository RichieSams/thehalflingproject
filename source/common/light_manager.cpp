/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#include "common/light_manager.h"


namespace Common {

LightManager::LightManager() 
	: m_needsBufferRebuild(false) {
}

uint LightManager::AddPointLight(PointLight &light) {
	m_needsBufferRebuild = true;
	m_pointLights[m_pointLightCounter] = light;

	return m_pointLightCounter++;
}

uint LightManager::AddSpotLight(SpotLight &light) {
	m_needsBufferRebuild = true;
	m_spotLights[m_spotLightCounter] = light;

	return m_spotLightCounter++;
}

bool LightManager::RemovePointLight(uint lightId) {
	return m_pointLights.erase(lightId) == 1;
}

bool LightManager::RemoveSpotLight(uint lightId) {
	return m_spotLights.erase(lightId) == 1;
}

} // End of namespace Common
