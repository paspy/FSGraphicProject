#pragma once
#include "../D3DApp/D3DSturcture.h"

using namespace D3DSturcture;

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
		XMMATRIX WorldViewProj;
		Material material;
	};

	WaveMesh() {}
	~WaveMesh();
	void Init(ID3D11Device *_d3dDevice) {
		D3D11_BUFFER_DESC cbbd;
		ZeroMemory(&cbbd, sizeof(D3D11_BUFFER_DESC));
		cbbd.Usage = D3D11_USAGE_DEFAULT;
		cbbd.ByteWidth = sizeof(CBuffer);
		cbbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbbd.CPUAccessFlags = 0;
		cbbd.MiscFlags = 0;
		HR(_d3dDevice->CreateBuffer(&cbbd, NULL, &constBuffer));

		D3D11_RASTERIZER_DESC cmdesc;
		ZeroMemory(&cmdesc, sizeof(D3D11_RASTERIZER_DESC));
		cmdesc.FillMode = D3D11_FILL_SOLID;
		cmdesc.CullMode = D3D11_CULL_NONE;

		HR(_d3dDevice->CreateRasterizerState(&cmdesc, &rasterState));

		D3D11_SAMPLER_DESC sampDesc;
		ZeroMemory(&sampDesc, sizeof(sampDesc));
		sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		sampDesc.MinLOD = 0;
		sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
		_d3dDevice->CreateSamplerState(&sampDesc, &texSamplerState);

		// hard coded matrtial setting - NOT GOOD
		cbBuffer.material.Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
		cbBuffer.material.Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 16.0f);

		// init the wave
		waves.Init(320, 320, 1.0f, 0.03f, 3.25f, 0.4f);

	}

	void Update(double _dt) {

	}

	void Render(mutex *_mutex, ID3D11DeviceContext *_d3dImmediateContext, XMMATRIX _camView, XMMATRIX _camProj) {

		// Set the default VS shader and depth/stencil state and layout
		_d3dImmediateContext->VSSetShader(vertexShader, NULL, 0);
		_d3dImmediateContext->PSSetShader(pixelShader, NULL, 0);
		_d3dImmediateContext->IASetInputLayout(inputLayout);
		_d3dImmediateContext->OMSetDepthStencilState(NULL, 0);


	}

	CBuffer								cbBuffer;
	ID3D11VertexShader					*vertexShader = nullptr;
	ID3D11PixelShader					*pixelShader = nullptr;
	ID3D11InputLayout					*inputLayout = nullptr;
	vector<ID3D11ShaderResourceView*>	shaderResView;
	ID3D11Buffer						*vertBuffer = nullptr;
	ID3D11Buffer						*indexBuffer = nullptr;
	ID3D11Buffer						*constBuffer = nullptr;
	ID3D11RasterizerState				*rasterState = nullptr;
	ID3D11SamplerState					*texSamplerState = nullptr;
	XMMATRIX							worldMat = XMMatrixIdentity();
	UINT								stride = sizeof(Vertex3D);
	UINT								offset = 0;
	Waves								waves;

	D3D11_INPUT_ELEMENT_DESC			vertexLayout[4] =
	{
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT",	0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
};