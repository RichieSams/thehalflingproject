/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#include "obj_loader_demo/obj_loader_demo.h"


namespace ObjLoaderDemo {

void ObjLoaderDemo::Update() {
	if (m_animateLights) {
		for (uint i = 0; i < m_pointLightAnimators.size(); ++i) {
			m_pointLightAnimators[i]->AnimateLight(m_pointLights[i], m_updatePeriod);
		}

		for (uint i = 0; i < m_spotLightAnimators.size(); ++i) {
			m_spotLightAnimators[i]->AnimateLight(m_spotLights[i], m_updatePeriod);
		}
	}
}

} // End of namespace ObjLoaderDemo
