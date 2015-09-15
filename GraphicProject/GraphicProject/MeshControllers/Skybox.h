#pragma once
#include "../D3DApp/D3DSturcture.h"
#include "../D3DApp/D3DUtils.h"
#include "../D3DApp/DirectXTK/DDSTextureLoader.h"

using namespace D3DSturcture;



class Skybox {
public:
	struct CBuffer {
		XMMATRIX WorldViewProj;
	};
	Skybox() {}
	~Skybox();
	void Init(ID3D11Device *_d3dDevice);
	void LoadStuff(ID3D11Device * _d3dDevice);
	void Render(ID3D11DeviceContext *_d3dImmediateContext, XMMATRIX _camView, XMMATRIX _camProj, ID3D11RasterizerState *_rs);

	CBuffer								cBuffer;
	ID3D11VertexShader					*vertexShader = nullptr;
	ID3D11PixelShader					*pixelShader = nullptr;
	ID3D11InputLayout					*inputLayout = nullptr;
	ID3D11ShaderResourceView			*shaderResView = nullptr;

	ID3D11Buffer						*indexBuffer = nullptr;
	ID3D11Buffer						*vertBuffer = nullptr;
	ID3D11Buffer						*constBuffer = nullptr;
	ID3D11DepthStencilState				*DSLessEqual = nullptr;
	ID3D11SamplerState					*texSamplerState = nullptr;
	int									numVertices = 0;
	int									numFaces = 0;
	XMMATRIX							worldMat = XMMatrixIdentity();
	UINT								stride = sizeof(Vertex3D);
	UINT								offset = 0;
	D3D11_INPUT_ELEMENT_DESC			vertexLayout[2] =
	{
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	void BuildSphere(
		ID3D11Device *_d3dDevice,
		int _latLines,
		int _longLines,
		ID3D11Buffer ** _vertBuffer,
		ID3D11Buffer ** _indexBuffer,
		int &_numSphereVertices,
		int &_numSphereFaces);

};