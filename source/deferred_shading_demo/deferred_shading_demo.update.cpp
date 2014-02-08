/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#include "deferred_shading_demo/deferred_shading_demo.h"


namespace DeferredShadingDemo {

void DeferredShadingDemo::Update() {
	m_worldViewProj.view = m_camera.CreateViewMatrix(DirectX::XMVectorZero());

	for (uint i = 0; i < m_pointLightAnimators.size(); ++i) {
		m_pointLightAnimators[i].AnimateLight(&m_pointLights[i], m_updatePeriod);
	}

	for (uint i = 0; i < m_spotLightAnimators.size(); ++i) {
		m_spotLightAnimators[i].AnimateLight(&m_spotLights[i], m_updatePeriod);
	}
}

} // End of namespace DeferredShadingDemo
