/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#include "common/shader.h"


namespace Common {

void SetConstants(ID3D11DeviceContext *context, ID3D11Buffer *buffer, void *data, size_t dataSize, uint slotNumber) {
	// Make sure the buffer even exists
	assert(buffer != nullptr);

	// Map the data
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	// Lock the constant buffer so it can be written to.
	HR(context->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
	memcpy(mappedResource.pData, data, dataSize);
	context->Unmap(buffer, 0);
}

} // End of namespace Common
