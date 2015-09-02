#pragma once
#include <Windows.h>
#include <Windowsx.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include <DirectXColors.h>
#include <DirectXPackedVector.h>
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

#define VK_LW 0x57
#define VK_LS 0x53
#define VK_LA 0x41
#define VK_LD 0x44

// d3d structures

typedef struct Vertex3D {
	Vertex3D() {}
	Vertex3D(XMFLOAT3 _pos, XMFLOAT2 _tex) : pos(_pos), texCoord(_tex) { }
	Vertex3D(XMFLOAT3 _pos, XMFLOAT2 _tex, XMFLOAT3 _norm) : pos(_pos), texCoord(_tex), normal(_norm) {}

	XMFLOAT3 pos;
	XMFLOAT2 texCoord;
	XMFLOAT3 normal;
	// bump normal mapping
	XMFLOAT3 tangent;
	XMFLOAT3 biTangent;
}*Vertex3D_ptr;

struct BaseLight {
	BaseLight() { ZeroMemory(this, sizeof(BaseLight)); }
	XMFLOAT3 direction;
	float paddding_1;
	XMFLOAT3 position;
	float range;
	XMFLOAT3 spotLightDir;
	float cone;
	XMFLOAT3 attenuation;
	float paddding_2;
	XMFLOAT4 ambient;
	XMFLOAT4 diffuse;
};

struct SurfaceMaterial {
	SurfaceMaterial() : hasNormMap(false), hasTexture(false) {}
	wstring matName;
	XMFLOAT4 difColor;
	int texArrayIndex;
	int normMapTexArrayIndex;
	bool hasNormMap;
	bool hasTexture;
	bool transparent;
};


// constant buffer structures
struct ConstPerObject {
	ConstPerObject() : hasTexture(false), hasNormal(false) {}
	XMMATRIX WVP;
	XMMATRIX World;
	XMFLOAT4 difColor;
	// need to 4 bytes
	bool hasTexture;
	bool hasNormal;
};

struct ConstPerFrame {
	BaseLight baseLight;
};

class Skybox {
public:
	Skybox() {}
	~Skybox() {
		SafeRelease(inputLayout);
		SafeRelease(indexBuffer);
		SafeRelease(vertBuffer);
		SafeRelease(vertexShader);
		SafeRelease(pixelShader);
		SafeRelease(shaderResView);
		SafeRelease(DSLessEqual);
		SafeRelease(rasterState);
	}
	ID3D11Buffer						*indexBuffer = nullptr;
	ID3D11Buffer						*vertBuffer = nullptr;
	ID3D11VertexShader					*vertexShader = nullptr;
	ID3D11PixelShader					*pixelShader = nullptr;
	ID3D11InputLayout					*inputLayout = nullptr;
	ID3D11ShaderResourceView			*shaderResView = nullptr;
	ID3D11DepthStencilState				*DSLessEqual = nullptr;
	ID3D11RasterizerState				*rasterState = nullptr;
	int									numVertices;
	int									numFaces;
	XMMATRIX							worldMat;
	D3D11_INPUT_ELEMENT_DESC			vertexLayout[2] =
	{
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
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





