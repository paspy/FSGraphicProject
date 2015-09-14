#pragma once
#include "../D3DApp/D3DSturcture.h"
#include "../D3DApp/D3DUtils.h"
#include "../D3DApp/DirectXTK/DDSTextureLoader.h"

using namespace D3DSturcture;

class ObjMesh {
public:

	struct SurfaceMaterial {
		SurfaceMaterial() {}
		wstring matName;
		XMFLOAT4 difColor;
		int texArrayIndex;
		int normMapTexArrayIndex;
	};

	struct Material {
		Material() { ZeroMemory(this, sizeof(this)); }
		XMFLOAT4 Ambient;
		XMFLOAT4 Diffuse;
		XMFLOAT4 Specular; // w = Specular Power
		XMFLOAT4 Reflect;
	};

	struct CBuffer {
		CBuffer() {}
		XMMATRIX World;
		XMMATRIX WorldInvTranspose;
		XMMATRIX WorldViewProj;
		XMMATRIX TexTransform;
		Material material;
	};

	ObjMesh() {}
	~ObjMesh();

	void Init(ID3D11Device *_d3dDevice, IDXGISwapChain *_swapChain, wstring _filename, bool _isRHCoordSys, bool _computeNormals, LPCWSTR _shaderFilename);

	void Render(ID3D11DeviceContext *_d3dImmediateContext, XMMATRIX _camView, XMMATRIX _camProj, ID3D11RasterizerState *_rasterState);

	CBuffer								cbBuffer;
	ID3D11VertexShader					*vertexShader = nullptr;
	ID3D11PixelShader					*pixelShader = nullptr;
	ID3D11InputLayout					*inputLayout = nullptr;
	vector<ID3D11ShaderResourceView*>	shaderResView;
	ID3D11Buffer						*vertBuffer = nullptr;
	ID3D11Buffer						*indexBuffer = nullptr;
	ID3D11Buffer						*constBuffer = nullptr;
	ID3D11SamplerState					*texSamplerState = nullptr;
	XMMATRIX							worldMat = XMMatrixIdentity();
	XMMATRIX							objTexTransform = XMMatrixIdentity();
	UINT								stride = sizeof(Vertex3D);
	UINT								offset = 0;
	int									subsets = 0;
	vector<int>							subsetIndexStart;
	vector<int>							subsetTexture;
	vector<SurfaceMaterial>				materials;
	vector<wstring>						textureNameArray;
	D3D11_INPUT_ELEMENT_DESC			vertexLayout[4] =
	{
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT",	0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

private:
	bool CreateModelFromObjFile(
		ID3D11Device *_d3dDevice,
		IDXGISwapChain *_swapChain,
		wstring _filename,											// .obj filename
		ID3D11Buffer **_vertBuff,									// mesh vertex buffer
		ID3D11Buffer **_indexBuff,									// mesh index buffer
		vector<wstring> &_textureNameArray,							// texture names
		vector<ID3D11ShaderResourceView*> &_meshShaderResView,		// mesh shader res view
		vector<int>& _subsetIndexStart,								// start index of each subset
		vector<int>& _subsetMaterialArray,							// index value of material for each subset
		vector<SurfaceMaterial>& _materials,						// vector of material structures
		int& _subsetCount,											// Number of subsets in mesh
		bool _isRHCoordSys,											// true if model was created in right hand coord system
		bool _computeNormals);										// true to compute the normals, false to use the files normals
};