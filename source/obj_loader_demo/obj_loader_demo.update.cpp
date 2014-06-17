/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#include "obj_loader_demo/obj_loader_demo.h"


namespace ObjLoaderDemo {

void ObjLoaderDemo::Update() {
	if (m_animateLights) {
		for (auto iter = m_pointLightAnimators.begin(); iter != m_pointLightAnimators.end(); ++iter) {
			iter->AnimateLight(m_updatePeriod);
		}

		for (auto iter = m_spotLightAnimators.begin(); iter != m_spotLightAnimators.end(); ++iter) {
			iter->AnimateLight(m_updatePeriod);
		}
	}
}

} // End of namespace ObjLoaderDemo
