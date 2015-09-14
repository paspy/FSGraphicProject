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

	static void CreateBox(float width, float height, float depth, MeshData& meshData);
	static void CreateSphere(float radius, UINT sliceCount, UINT stackCount, MeshData& meshData);
	static void CreateGeosphere(float radius, UINT numSubdivisions, MeshData& meshData);
	static void CreateCylinder(float bottomRadius, float topRadius, float height, UINT sliceCount, UINT stackCount, MeshData& meshData);
	static void CreateGrid(float width, float depth, UINT m, UINT n, MeshData& meshData);
	static void CreateFullscreenQuad(MeshData& meshData);
	static void Subdivide(MeshData& meshData);
	static XMFLOAT3 GetHillNormal(float x, float z);
	static float GetHillHeight(float x, float z);

private:
	static void BuildCylinderTopCap(float bottomRadius, float topRadius, float height, UINT sliceCount, UINT stackCount, MeshData& meshData);
	static void BuildCylinderBottomCap(float bottomRadius, float topRadius, float height, UINT sliceCount, UINT stackCount, MeshData& meshData);
};
