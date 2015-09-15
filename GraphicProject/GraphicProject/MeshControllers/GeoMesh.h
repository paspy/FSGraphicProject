#pragma once
#include "../D3DApp/D3DSturcture.h"
#include "../D3DApp/D3DUtils.h"
#include "../D3DApp/DirectXTK/DDSTextureLoader.h"
#include "../D3DApp/GeoGen.h"

using namespace D3DSturcture;

class GeoMesh {
public:

	enum GeoType {
		Box, Cylinder, Sphere, Grid
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
		XMMATRIX WorldInvTranspose;
		XMMATRIX View;
		XMMATRIX Proj;
		XMMATRIX TexTransform;
		Material material;
	};

	GeoMesh() {}
	~GeoMesh();
	void Init(ID3D11Device *_d3dDevice, LPCWSTR _shaderFilename, GeoType _genType, LPCWSTR _diffuseFile, LPCWSTR _normalFile);
	void BuildBuffer(ID3D11Device * _d3dDevice, GeoType _genType);
	void BuildInstancedBuffer(ID3D11Device * _d3dDevice);

	float GetDistanceFromCamera(XMFLOAT4X4 _m4x4, XMVECTOR _camPosition);

	void Update(ID3D11DeviceContext * _d3dImmediateContext, XMVECTOR _camPosition);
	void Render(ID3D11DeviceContext *_d3dImmediateContext, XMMATRIX _camView, XMMATRIX _camProj, ID3D11BlendState* _bs, float *_bf);

	CBuffer								cbBuffer;
	ID3D11VertexShader					*vertexShader = nullptr;
	ID3D11PixelShader					*pixelShader = nullptr;
	ID3D11InputLayout					*inputLayout = nullptr;
	ID3D11ShaderResourceView*			shaderResView = nullptr;
	ID3D11ShaderResourceView*			normalShaderResView = nullptr;
	ID3D11Buffer						*vertBuffer = nullptr;
	ID3D11Buffer						*indexBuffer = nullptr;
	ID3D11Buffer						*instancedBuffer = nullptr;
	ID3D11Buffer						*constBuffer = nullptr;
	ID3D11SamplerState					*texSamplerState = nullptr;
	ID3D11RasterizerState				*CCWcullMode = nullptr;
	ID3D11RasterizerState				*CWcullMode = nullptr;
	XMMATRIX							worldMat = XMMatrixIdentity();
	XMMATRIX							geoTexTransform = XMMatrixIdentity();
	vector<InstancedData>				instancedData;

	UINT								stride[2] = { sizeof(Vertex3D), sizeof(InstancedData) };
	UINT								offset[2] = { 0, 0 };
	UINT								indicesCount = 0;
	UINT								visibleObjectCount = 0;

	D3D11_INPUT_ELEMENT_DESC			vertexLayout[8] =
	{
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA,	0 },
		{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA,	0 },
		{ "NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA,	0 },
		{ "TANGENT",	0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA,	0 },
		// instanced
		{ "WORLD",		0, DXGI_FORMAT_R32G32B32A32_FLOAT,	1, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_INSTANCE_DATA,	1 },
		{ "WORLD",		1, DXGI_FORMAT_R32G32B32A32_FLOAT,	1, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_INSTANCE_DATA,	1 },
		{ "WORLD",		2, DXGI_FORMAT_R32G32B32A32_FLOAT,	1, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_INSTANCE_DATA,	1 },
		{ "WORLD",		3, DXGI_FORMAT_R32G32B32A32_FLOAT,	1, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_INSTANCE_DATA,	1 },

	};
};