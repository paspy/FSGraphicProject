#pragma once
#include "../D3DApp/D3DSturcture.h"
#include "../D3DApp/D3DUtils.h"
#include "../D3DApp/DirectXTK/DDSTextureLoader.h"
#include "../D3DApp/GeoGen.h"

using namespace D3DSturcture;

class MirrorMesh {
public:
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

	MirrorMesh() {}
	~MirrorMesh();
	void Init(ID3D11Device *_d3dDevice, LPCWSTR _shaderFilename);
	void BuildBuffer(ID3D11Device * _d3dDevice);
	void Update();
	void Render(ID3D11DeviceContext *_d3dImmediateContext, XMMATRIX _camView, XMMATRIX _camProj, ID3D11RasterizerState *_rs);

	CBuffer								cbObject;
	Material							mtObject;
	Material							mtWallAndFloor;
	Material							mtMirror;
	Material							mtShadow;

	ID3D11VertexShader					*vertexShader = nullptr;
	ID3D11PixelShader					*pixelShader = nullptr;
	ID3D11InputLayout					*inputLayout = nullptr;
	ID3D11ShaderResourceView			*shaderResViews[3] = { nullptr, nullptr, nullptr };
	ID3D11ShaderResourceView			*normalShaderResViews[3] = { nullptr, nullptr, nullptr };
	ID3D11Buffer						*vertBuffer[2] = { nullptr, nullptr };
	ID3D11Buffer						*indexBuffer = nullptr;
	ID3D11Buffer						*constBuffer = nullptr;
	ID3D11SamplerState					*texSamplerState = nullptr;
	XMMATRIX							worldMat = XMMatrixIdentity();
	XMMATRIX							wallFloorMat = XMMatrixIdentity();

	UINT								stride = sizeof(Vertex3D);
	UINT								offset = 0;
	UINT								indicesCount;
	D3D11_INPUT_ELEMENT_DESC			vertexLayout[4] =
	{
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT",	0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
};