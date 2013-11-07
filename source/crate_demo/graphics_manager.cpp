/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#include "crate_demo/graphics_manager.h"

#include "common/d3d_util.h"

#include "assert.h"

namespace CrateDemo {

GraphicsManager::GraphicsManager(GameStateManager *gameStateManager)
	: Common::GraphicsManagerBase(),
	  m_gameStateManager(gameStateManager),
	  m_renderTargetView(nullptr),
}

bool CrateDemo::GraphicsManager::Initialize(int clientWidth, int clientHeight, HWND hwnd) {
	if (!Common::GraphicsManagerBase::Initialize(clientWidth, clientHeight, hwnd))
		return false;


	return true;
}

void GraphicsManager::Shutdown() {

}

void GraphicsManager::DrawFrame() {

}

void GraphicsManager::OnResize(int newClientWidth, int newClientHeight) {
	if (!m_d3dInitialized) {
		return;
	}

	// Release the render target view
	ReleaseCOM(m_renderTargetView);

	Common::GraphicsManagerBase::OnResize(newClientWidth, newClientHeight);

	// Recreate the render target view.
	ID3D11Texture2D* backBuffer;
	HR(m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer)));
	HR(m_device->CreateRenderTargetView(backBuffer, 0, &m_renderTargetView));
	ReleaseCOM(backBuffer);

	// Bind the render target view and depth/stencil view to the pipeline.
	m_immediateContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);

}

void GraphicsManager::GamePaused() {

}

void GraphicsManager::GameUnpaused() {

}

} // End of namespace CrateDemo
