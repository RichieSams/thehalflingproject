/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#pragma once

#include "common/typedefs.h"

#include "DirectXMath.h"


namespace LightingDemo {

class WaveSimulator {
public:
	WaveSimulator();
	~WaveSimulator();

	uint RowCount() const { return m_numRows; }
	uint ColumnCount() const { return m_numCols; }
	uint VertexCount() const { return m_vertexCount; }
	uint TriangleCount() const { return m_triangleCount; }

	// Returns the solution at the ith grid point.
	const DirectX::XMFLOAT3& operator[](int i) const { return m_currSolution[i]; }

	void Init(uint m, uint n, float dx, float dt, float speed, float damping);
	void Update(float dt);
	void Disturb(uint i, uint j, float magnitude);

private:
	uint m_numRows;
	uint m_numCols;

	uint m_vertexCount;
	uint m_triangleCount;

	// Simulation constants we can precompute
	float m_k1;
	float m_k2;
	float m_k3;

	float m_timeStep;
	float m_spatialStep;

	DirectX::XMFLOAT3 *m_prevSolution;
	DirectX::XMFLOAT3 *m_currSolution;

	float m_accumulatedTime;
};

} // End of namespace LightingDemo
