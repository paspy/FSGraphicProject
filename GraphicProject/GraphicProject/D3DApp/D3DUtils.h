#pragma once
#include <Windows.h>
#include <Windowsx.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include <DirectXColors.h>
#include <D3Dcompiler.h>
#include <vector>
#include <sstream>
#include <iostream>
#include <string>
#include <fstream>
#include "DirectXTK/DDSTextureLoader.h"

using namespace DirectX;
using namespace std;

// Convenience macro for releasing objects.
#define SafeRelease(x) { if(x){ x->Release(); x = nullptr; } }

// Convenience macro for deleting objects.
#define SafeDelete(x) { delete x; x = nullptr; }

// d3d error checker
#if defined(DEBUG) | defined(_DEBUG)
#ifndef HR
#define HR(x) {												\
		HRESULT hr = (x);									\
		if(FAILED(hr)) {									\
			LPWSTR output;									\
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM |		\
				FORMAT_MESSAGE_IGNORE_INSERTS 	 |			\
				FORMAT_MESSAGE_ALLOCATE_BUFFER,				\
				NULL,										\
				hr,											\
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),	\
				(LPTSTR) &output,							\
				0,											\
				NULL);										\
			MessageBox(NULL, output, L"Error", MB_OK);		\
		}													\
	}
#endif
#else
#ifndef HR
#define HR(x) (x)
#endif
#endif 

#define BACKBUFFER_WIDTH	1366
#define BACKBUFFER_HEIGHT	768

#define IDR_MENU1                       101
#define IDD_DIALOG1                     102
#define IDR_ACCELERATOR1                105
#define IDI_ICON1                       106
#define IDC_RENDER_WINDOW               1001
#define IDC_TAB1                        1006
#define IDC_SYSLINK1                    1007
#define IDC_TOOL_DIVIDER                1009
#define IDC_BUTTON2                     1011
#define IDC_BUTTON1                     1012
#define IDC_BUTTON3                     1014
#define IDC_LIST1                       1016
#define IDC_STATIC_FPS                  1017
#define ID_FILE_OPEN_MENU               40001
#define ID_FILE_EXIT                    40002
#define ID_ACCELERATOR_OPEN             40005

#define VK_LW 0x57
#define VK_LS 0x53
#define VK_LA 0x41
#define VK_LD 0x44

// d3d structures

typedef struct Vertex3D {
	Vertex3D() { 
		pos = XMFLOAT3(0, 0, 0); 
		normal = XMFLOAT3(0, 0, 0);
		tangent = XMFLOAT3(0, 0, 0);
		biTangent = XMFLOAT3(0, 0, 0);
		texCoord = XMFLOAT2(0, 0);
	}
	Vertex3D(XMFLOAT3 _pos, XMFLOAT2 _tex) : pos(_pos), texCoord(_tex) { }
	Vertex3D(XMFLOAT3 _pos, XMFLOAT2 _tex, XMFLOAT3 _norm) : pos(_pos), texCoord(_tex), normal(_norm) {}

	XMFLOAT3 pos;
	XMFLOAT2 texCoord;
	XMFLOAT3 normal;
	// bump normal mapping
	XMFLOAT3 tangent;
	XMFLOAT3 biTangent;
}*Vertex3D_ptr;

// constant buffer structures
struct DirectionLight {
	DirectionLight() { ZeroMemory(this, sizeof(DirectionLight)); }
	XMFLOAT3 direction;
	float pad;
	XMFLOAT4 ambient;
	XMFLOAT4 diffuse;
};

struct PointLight {
	PointLight() { ZeroMemory(this, sizeof(PointLight)); }
	XMFLOAT3 position;
	float range;
	XMFLOAT3 attenuation; float pad;
	XMFLOAT4 diffuse;
};

struct SpotLight {
	SpotLight() { ZeroMemory(this, sizeof(SpotLight)); }
	XMFLOAT3 direction; float pad;
	XMFLOAT3 position;
	float range;
	float cone;
	XMFLOAT3 attenuation;
	XMFLOAT4 diffuse;
};

struct ConstPerFrame {
	DirectionLight directionLight;
	PointLight pointLight;
	SpotLight spotLight;
};

class Skybox {
public:
	struct CBuffer {
		XMMATRIX WVP;
	};
	Skybox() {}
	~Skybox() {
		SafeRelease(inputLayout);
		SafeRelease(indexBuffer);
		SafeRelease(vertBuffer);
		SafeRelease(constBuffer);
		SafeRelease(vertexShader);
		SafeRelease(pixelShader);
		SafeRelease(shaderResView);
		SafeRelease(DSLessEqual);
		SafeRelease(rasterState);
		SafeRelease(texSamplerState);
	}
	void Init(ID3D11Device *_d3dDevice) {
		D3D11_BUFFER_DESC cbbd;
		ZeroMemory(&cbbd, sizeof(D3D11_BUFFER_DESC));
		cbbd.Usage = D3D11_USAGE_DEFAULT;
		cbbd.ByteWidth = sizeof(CBuffer);
		cbbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbbd.CPUAccessFlags = 0;
		cbbd.MiscFlags = 0;
		HR(_d3dDevice->CreateBuffer(&cbbd, NULL, &constBuffer));

		D3D11_SAMPLER_DESC sampDesc;
		ZeroMemory(&sampDesc, sizeof(sampDesc));
		sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		sampDesc.MinLOD = 0;
		sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
		HR(_d3dDevice->CreateSamplerState(&sampDesc, &texSamplerState));

		D3D11_RASTERIZER_DESC cmdesc;
		ZeroMemory(&cmdesc, sizeof(D3D11_RASTERIZER_DESC));
		cmdesc.FillMode = D3D11_FILL_SOLID;
		cmdesc.CullMode = D3D11_CULL_NONE;

		HR(_d3dDevice->CreateRasterizerState(&cmdesc, &rasterState));

		D3D11_DEPTH_STENCIL_DESC dssDesc;
		ZeroMemory(&dssDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
		dssDesc.DepthEnable = true;
		dssDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		dssDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

		HR(_d3dDevice->CreateDepthStencilState(&dssDesc, &DSLessEqual));

	}
	void Render(ID3D11DeviceContext *_d3dImmediateContext, XMMATRIX _camView, XMMATRIX _camProj) {
		//Set the proper VS and PS shaders, and layout
		_d3dImmediateContext->VSSetShader(vertexShader, 0, 0);
		_d3dImmediateContext->PSSetShader(pixelShader, 0, 0);
		_d3dImmediateContext->IASetInputLayout(inputLayout);
		//Set the spheres index buffer
		_d3dImmediateContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
		//Set the spheres vertex buffer
		_d3dImmediateContext->IASetVertexBuffers(0, 1, &vertBuffer, &stride, &offset);
		//Set the WVP matrix and send it to the constant buffer in shader file
		cBuffer.WVP = XMMatrixTranspose(worldMat * _camView * _camProj);
		_d3dImmediateContext->UpdateSubresource(constBuffer, 0, NULL, &cBuffer, 0, 0);
		_d3dImmediateContext->VSSetConstantBuffers(0, 1, &constBuffer);
		//Send our skymap resource view to pixel shader
		_d3dImmediateContext->PSSetShaderResources(0, 1, &shaderResView);
		_d3dImmediateContext->PSSetSamplers(0, 1, &texSamplerState);
		//Set the new depth/stencil and RS states
		_d3dImmediateContext->OMSetDepthStencilState(DSLessEqual, 0);
		_d3dImmediateContext->RSSetState(rasterState);
		_d3dImmediateContext->DrawIndexed(numFaces * 3, 0, 0);
	}

	CBuffer								cBuffer;
	ID3D11VertexShader					*vertexShader = nullptr;
	ID3D11PixelShader					*pixelShader = nullptr;
	ID3D11InputLayout					*inputLayout = nullptr;
	ID3D11ShaderResourceView			*shaderResView = nullptr;

	ID3D11Buffer						*indexBuffer = nullptr;
	ID3D11Buffer						*vertBuffer = nullptr;
	ID3D11Buffer						*constBuffer = nullptr;
	ID3D11DepthStencilState				*DSLessEqual = nullptr;
	ID3D11RasterizerState				*rasterState = nullptr;
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
};

struct SurfaceMaterial {
	SurfaceMaterial() {}
	wstring matName;
	XMFLOAT4 difColor;
	int texArrayIndex;
	int normMapTexArrayIndex;
};

class ObjMesh {
public:
	struct CBuffer {
		CBuffer() {}
		XMMATRIX WVP;
		XMMATRIX World;
		XMFLOAT4 difColor;
	};
	
	ObjMesh() {}
	~ObjMesh() {
		SafeRelease(inputLayout);
		SafeRelease(indexBuffer);
		SafeRelease(vertBuffer);
		SafeRelease(constBuffer);
		SafeRelease(vertexShader);
		SafeRelease(pixelShader);
		for (size_t i = 0; i < shaderResView.size(); i++) {
			SafeRelease(shaderResView[i]);
		}
		SafeRelease(rasterState);
		SafeRelease(texSamplerState);
	}
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
	}
	void Render(ID3D11DeviceContext *_d3dImmediateContext, XMMATRIX _camView, XMMATRIX _camProj) {
		// Set the default VS shader and depth/stencil state and layout
		_d3dImmediateContext->VSSetShader(vertexShader, NULL, 0);
		_d3dImmediateContext->PSSetShader(pixelShader, NULL, 0);
		_d3dImmediateContext->IASetInputLayout(inputLayout);
		_d3dImmediateContext->OMSetDepthStencilState(NULL, 0);

		for (int i = 0; i < subsets; i++) {
			//Set the grounds index buffer
			_d3dImmediateContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
			//Set the grounds vertex buffer
			_d3dImmediateContext->IASetVertexBuffers(0, 1, &vertBuffer, &stride, &offset);
			//Set the WVP matrix and send it to the constant buffer in effect file
			cbBuffer.WVP = XMMatrixTranspose(worldMat * _camView * _camProj);
			cbBuffer.World = XMMatrixTranspose(worldMat);
			cbBuffer.difColor = materials[subsetTexture[i]].difColor;
			_d3dImmediateContext->UpdateSubresource(constBuffer, 0, NULL, &cbBuffer, 0, 0);
			_d3dImmediateContext->VSSetConstantBuffers(0, 1, &constBuffer);
			_d3dImmediateContext->PSSetConstantBuffers(1, 1, &constBuffer);
			_d3dImmediateContext->PSSetShaderResources(0, 1, &shaderResView[materials[subsetTexture[i]].texArrayIndex]);
			_d3dImmediateContext->PSSetShaderResources(1, 1, &shaderResView[materials[subsetTexture[i]].normMapTexArrayIndex]);
			_d3dImmediateContext->PSSetSamplers(0, 1, &texSamplerState);
			_d3dImmediateContext->RSSetState(rasterState);
			int indexStart = subsetIndexStart[i];
			int indexDrawAmount = subsetIndexStart[i + 1] - subsetIndexStart[i];
			_d3dImmediateContext->DrawIndexed(indexDrawAmount, indexStart, 0);
		}
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
};


class D3DUtils {

public:
	D3DUtils() {}

	template<typename T>
	static T Clamp(const T& _x, const T& _low, const T& _high) {
		return _x < _low ? _low : (_x > _high ? _high : _x);
	}

	static float DegreesToradians(float _degree) {
		return (_degree * XM_PI / 180.0f);
	}


	static bool loadOBJ( string _filePath, vector<Vertex3D> & _verts);

	static void BuildSphere(
		ID3D11Device *_d3dDevice,
		int _latLines,
		int _longLines,
		ID3D11Buffer ** _vertBuffer,
		ID3D11Buffer ** _indexBuffer,
		int &_numSphereVertices,
		int &_numSphereFaces);

	static bool CreateModelFromObjFile(
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

	static HRESULT CreateShaderAndLayoutFromFile(
		ID3D11Device *_d3dDevice,
		const LPCWSTR _fileName,
		const D3D11_INPUT_ELEMENT_DESC *_inputElemDesc,
		const UINT _elemNum,
		ID3D11VertexShader **_vertexShader,
		ID3D11PixelShader ** _pixelShader,
		ID3D11InputLayout **_inputLayout
		);

};
