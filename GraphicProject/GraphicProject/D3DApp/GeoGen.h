#pragma once
#include "Defines.h"
#include "D3DSturcture.h"
#include "Waves.h"
#include "D3DUtils.h"

using namespace D3DSturcture;

class GeoGen {
public:
	struct MeshData {
		vector<Vertex3D> Vertices;
		vector<UINT> Indices;
	};

	void CreateLandBuffer(ID3D11Device *_d3dDevice, ID3D11Buffer ** _vertBuffer, ID3D11Buffer ** _indexBuffer);

private:
	void CreateBox(float width, float height, float depth, MeshData& meshData);
	void CreateSphere(float radius, UINT sliceCount, UINT stackCount, MeshData& meshData);
	void CreateGeosphere(float radius, UINT numSubdivisions, MeshData& meshData);
	void CreateCylinder(float bottomRadius, float topRadius, float height, UINT sliceCount, UINT stackCount, MeshData& meshData);
	void CreateGrid(float width, float depth, UINT m, UINT n, MeshData& meshData);
	void CreateFullscreenQuad(MeshData& meshData);
	void Subdivide(MeshData& meshData);
	void BuildCylinderTopCap(float bottomRadius, float topRadius, float height, UINT sliceCount, UINT stackCount, MeshData& meshData);
	void BuildCylinderBottomCap(float bottomRadius, float topRadius, float height, UINT sliceCount, UINT stackCount, MeshData& meshData);
	XMFLOAT3 GetHillNormal(float x, float z) const;
	float GetHillHeight(float x, float z) const;
};
