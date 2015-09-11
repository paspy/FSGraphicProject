#pragma once
#include "Defines.h"
// d3d structures

typedef struct Vertex3D {
	Vertex3D() { 
		Position = XMFLOAT3(0, 0, 0); 
		Normal = XMFLOAT3(0, 0, 0);
		TangentU = XMFLOAT3(0, 0, 0);
		TexCoord = XMFLOAT2(0, 0);
	}
	Vertex3D(XMFLOAT3 _pos, XMFLOAT2 _tex) : Position(_pos), TexCoord(_tex) { }
	Vertex3D(XMFLOAT3 _pos, XMFLOAT2 _tex, XMFLOAT3 _norm) : Position(_pos), TexCoord(_tex), Normal(_norm) {}
	Vertex3D(const XMFLOAT3& p, const XMFLOAT3& n, const XMFLOAT3& t, const XMFLOAT2& uv) : Position(p), Normal(n), TangentU(t), TexCoord(uv) {}
	Vertex3D(
		float px, float py, float pz,
		float nx, float ny, float nz,
		float tx, float ty, float tz,
		float u, float v
	) : Position(px, py, pz), Normal(nx, ny, nz), TangentU(tx, ty, tz), TexCoord(u, v) {}

	XMFLOAT3 Position;
	XMFLOAT2 TexCoord;
	XMFLOAT3 Normal;
	// bump Normal mapping
	XMFLOAT3 TangentU;
}*Vertex3D_ptr;

// constant buffer structures
struct DirectionalLight {
	DirectionalLight() { ZeroMemory(this, sizeof(this)); }
	XMFLOAT4 Ambient;
	XMFLOAT4 Diffuse;
	XMFLOAT4 Specular;

	XMFLOAT3 Direction;
	float Pad;
};

struct PointLight {
	PointLight() { ZeroMemory(this, sizeof(this)); }
	XMFLOAT4 Ambient;
	XMFLOAT4 Diffuse;
	XMFLOAT4 Specular;

	XMFLOAT3 Position;
	float Range;

	XMFLOAT3 Att;
	float Pad;
};

struct SpotLight {
	SpotLight() { ZeroMemory(this, sizeof(this)); }
	XMFLOAT4 Ambient;
	XMFLOAT4 Diffuse;
	XMFLOAT4 Specular;

	XMFLOAT3 Position;
	float Range;
	
	XMFLOAT3 Direction;
	float Spot;
	
	XMFLOAT3 Att;
	float Pad;
};

struct cbPerFrame {
	DirectionalLight directionalLight;
	PointLight pointLight;
	SpotLight spotLight;
	XMFLOAT4 cameraPos;

};

class Skybox {
public:
	struct CBuffer {
		XMMATRIX WorldViewProj;
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
		//Set the WorldViewProj matrix and send it to the constant buffer in shader file
		cBuffer.WorldViewProj = XMMatrixTranspose(worldMat * _camView * _camProj);
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

		// hard coded matrtial setting.
		cbBuffer.material.Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
		cbBuffer.material.Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 16.0f);
	}

	void Render(ID3D11DeviceContext *_d3dImmediateContext, XMVECTOR _camPos, XMMATRIX _camView, XMMATRIX _camProj) {
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
			//Set the stuff to the constant buffer to the hlsl file
			cbBuffer.World = XMMatrixTranspose(worldMat);
			cbBuffer.WorldViewProj = XMMatrixTranspose(worldMat * _camView * _camProj);
			
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

	static float AngleFromXY(float x, float y) {
		float theta = 0.0f;
		// Quadrant I or IV
		if ( x >= 0.0f ) {
			// If x = 0, then atanf(y/x) = +pi/2 if y > 0
			//                atanf(y/x) = -pi/2 if y < 0
			theta = atanf(y / x); // in [-pi/2, +pi/2]

			if ( theta < 0.0f )
				theta += 2.0f*XM_PI; // in [0, 2*pi).
		}

		// Quadrant II or III
		else
			theta = atanf(y / x) + XM_PI; // in [0, 2*pi).

		return theta;
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

	static bool CreateModelFromObjFileKaiNi(
		ID3D11Device *_d3dDevice,
		IDXGISwapChain *_swapChain,
		string _filename,
		ID3D11Buffer **_vertBuff,
		ID3D11Buffer **_indexBuff
		
		);

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
