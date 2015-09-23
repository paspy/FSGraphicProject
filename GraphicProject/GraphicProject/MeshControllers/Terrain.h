#pragma once
#include "../D3DApp/Defines.h"
#include "../D3DApp/D3DSturcture.h"

using namespace D3DSturcture;

class Camera;

class Terrain {
public:
	struct Material {
		Material() { ZeroMemory(this, sizeof(this)); }
		XMFLOAT4 Ambient;
		XMFLOAT4 Diffuse;
		XMFLOAT4 Specular; // w = Specular Power
		XMFLOAT4 Reflect;
	};

	struct cbPerFrameT {
		DirectionalLight DirLight; // 64 bytes
		XMFLOAT4 CameraPos; 

		float MinDist;
		float MaxDist;
		float MinTess;
		float MaxTess;

		float TexelCellSpaceU;
		float TexelCellSpaceV;
		float WorldCellSpace;
		float Pad = 0;

		XMFLOAT4 WorldFrustumPlanes[6];

	};

	struct cbPerObjectT {
		XMMATRIX ViewProj;
		Material material;
	};

	struct VertexT {
		XMFLOAT3 Position;
		XMFLOAT2 TexCoord;
		XMFLOAT2 BoundsY;
	};

	struct TerrainInfo {
		wstring HeightMapFilename;
		wstring LayerMapFilename0;
		wstring LayerMapFilename1;
		wstring LayerMapFilename2;
		wstring LayerMapFilename3;
		wstring LayerMapFilename4;
		wstring BlendMapFilename;
		float HeightScale;
		UINT HeightmapWidth;
		UINT HeightmapHeight;
		float CellSpacing;
	};

public:
	Terrain();
	~Terrain();

	float GetWidth()const;
	float GetDepth()const;
	float GetHeight(float x, float z)const;

	XMMATRIX GetWorld()const;
	void SetWorld(XMMATRIX M);

	void Init(ID3D11Device* _d3dDevice, ID3D11DeviceContext* _context);
	void Update();
	void Render(ID3D11DeviceContext* _context, const Camera& _camera, D3DSturcture::DirectionalLight _light);

private:
	void LoadHeightmap();
	void Smooth();
	bool InBounds(int i, int j);
	float Average(int i, int j);
	void CalcAllPatchBoundsY();
	void CalcPatchBoundsY(UINT i, UINT j);
	void BuildQuadPatchVB(ID3D11Device* _d3dDevice);
	void BuildQuadPatchIB(ID3D11Device* _d3dDevice);
	void BuildHeightmapSRV(ID3D11Device* _d3dDevice);

private:

	// Divide heightmap into patches such that each patch has CellsPerPatch cells
	// and CellsPerPatch+1 vertices.  Use 64 so that if we tessellate all the way 
	// to 64, we use all the data from the heightmap.
	static const int CellsPerPatch = 64;

	ID3D11Buffer				*m_quadPatchVertBuffer;
	ID3D11Buffer				*m_quadPatchIndexBuffer;
	ID3D11VertexShader			*m_vertexShader;
	ID3D11PixelShader			*m_pixelShader;
	ID3D11HullShader			*m_hullShader;
	ID3D11DomainShader			*m_domainShader;
	ID3D11Buffer				*m_cbPerFrame;
	ID3D11Buffer				*m_cbPerObject;

	ID3D11SamplerState			*m_linerSamplerState;
	ID3D11SamplerState			*m_heighMapSamplerState;
	ID3D11InputLayout			*m_inputLayout;

	//ID3D11ShaderResourceView	*m_layerMapArraySRV;
	vector<ID3D11ShaderResourceView*> m_layerMapArraySRV;
	ID3D11ShaderResourceView	*m_blendMapSRV;
	ID3D11ShaderResourceView	*m_heightMapSRV;

	TerrainInfo m_info;

	cbPerFrameT	 cbPerFrame;
	cbPerObjectT cbPerObj;

	UINT m_numPatchVertices;
	UINT m_numPatchQuadFaces;

	UINT m_numPatchVertRows;
	UINT m_numPatchVertCols;

	XMMATRIX m_world;

	Material m_material;

	vector<XMFLOAT2> m_patchBoundsY;
	vector<float> m_heightmap;

	const D3D11_INPUT_ELEMENT_DESC vertexLayout[3] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	
	bool m_wireFrameRS;
};
