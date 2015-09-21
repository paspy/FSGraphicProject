#pragma once
#include "../D3DApp/D3DSturcture.h"
#include "../D3DApp/D3DUtils.h"
#include "../D3DApp/DirectXTK/DDSTextureLoader.h"
#include "../D3DApp/Waves.h"

using namespace D3DSturcture;

class Camera;

class WaveMesh {
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

	WaveMesh() {}
	~WaveMesh();
	void Init(ID3D11Device *_d3dDevice, LPCWSTR _shaderFilename);
	void BuildBuffer(ID3D11Device * _d3dDevice);

	void Update(double _dt, double _tt, ID3D11DeviceContext *_context);

	void Render(ID3D11DeviceContext * _context, const Camera &_camera, ID3D11RasterizerState *_rs, ID3D11BlendState* _bs, float *_bf);

	CBuffer								cbBuffer;
	ID3D11VertexShader					*vertexShader = nullptr;
	ID3D11PixelShader					*pixelShader = nullptr;
	ID3D11InputLayout					*inputLayout = nullptr;
	ID3D11ShaderResourceView*			shaderResView = nullptr;
	ID3D11ShaderResourceView*			normalShaderResView = nullptr;
	ID3D11Buffer						*vertBuffer = nullptr;
	ID3D11Buffer						*indexBuffer = nullptr;
	ID3D11Buffer						*constBuffer = nullptr;
	ID3D11SamplerState					*texSamplerState = nullptr;
	XMMATRIX							worldMat = XMMatrixIdentity();
	XMMATRIX							waterTexTransform = XMMatrixIdentity();
	UINT								stride = sizeof(Vertex3D);
	UINT								offset = 0;
	Waves								waves;
	XMFLOAT2							waterTexOffset = XMFLOAT2(0.0f, 0.0f);

	D3D11_INPUT_ELEMENT_DESC			vertexLayout[4] =
	{
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT",	0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
};