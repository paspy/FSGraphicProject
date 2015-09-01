#include "GuineaPig.h"
// shader byte code
#include "../Base_VS.csh"
#include "../Base_PS.csh"
#include "../Skybox_VS.csh"
#include "../Skybox_PS.csh"

// texture header file
//#include "Resource/numbers_test.h"
//#include "Resource/grass_diffuse.h"
//#include "Resource/grass_normal.h"

#include <iostream>
#include <sstream>

GuineaPig::GuineaPig(HINSTANCE hinst) : D3DApp(hinst),
m_cubeVertexBuffer(nullptr),
m_groundVertexBuffer(nullptr),
m_inputLayout(nullptr),
m_vertexShader(nullptr),
m_pixelShader(nullptr) 


{


}

GuineaPig::~GuineaPig() {

	// release geometries ptr
	SafeRelease(m_cubeVertexBuffer);
	SafeRelease(m_cubeIndexBuffer);
	SafeRelease(m_groundIndexBuffer);
	SafeRelease(m_groundVertexBuffer);

	// release shader ptr
	SafeRelease(m_vertexShader);
	SafeRelease(m_pixelShader);

	// release layout ptr
	SafeRelease(m_inputLayout);
	SafeRelease(m_skyboxInputLayout);

	// release constant buffer ptr
	SafeRelease(m_cbCubeBuffer);
	SafeRelease(m_cbGroundBuffer);
	SafeRelease(m_cbPerFrameBuffer);

	// release texture ptr
	// the cube
	SafeRelease(m_cubeShaderResView);
	SafeRelease(m_cubeTexture2D);
	SafeRelease(m_baseTexSamplerState);

	// the ground
	SafeRelease(m_grassShaderResView);
	SafeRelease(m_grassTexture2D);

	// release lighting ptr
	SafeRelease(m_perFrameBuffer);

	// release render state ptr
	SafeRelease(m_antialiasedLine);
	SafeRelease(m_blendTransparency);
	SafeRelease(m_cwCullingMode);
	SafeRelease(m_ccwCullingMode);

	// release skybox ptr
	SafeRelease(m_sphereIndexBuffer);
	SafeRelease(m_sphereVertBuffer);
	SafeRelease(m_skyboxVertexShader);
	SafeRelease(m_skyboxPixelShader);
	SafeRelease(m_skyboxShaderResView);
	SafeRelease(m_skyboxDSLessEqual);
	SafeRelease(m_skyboxRasterState);

	// obj loader
	SafeRelease(m_meshVertBuff);
	SafeRelease(m_meshIndexBuff);
	for ( size_t i = 0; i < m_meshShaderResView.size(); i++ ) {
		SafeRelease(m_meshShaderResView[i]);
	}
}

bool GuineaPig::Init() {
	if ( !D3DApp::Init() )
		return false;

	BuildObjConstBuffer();
	BuildGeometryBuffers();
	BuildTextureAndState();
	BuildLighting();
	BuildVertexLayout();
	BuildShader();
	BuildRenderStates();

	return true;
}

void GuineaPig::OnResize() {
	D3DApp::OnResize();

}

void GuineaPig::BuildSphere(int _latLines, int _longLines,
	ID3D11Buffer ** _vertBuffer, ID3D11Buffer ** _indexBuffer,
	int &_numSphereVertices, int &_numSphereFaces) {
	_numSphereVertices = ((_latLines - 2) * _longLines) + 2;
	_numSphereFaces = ((_latLines - 3)*(_longLines)* 2) + (_longLines * 2);

	float sphereYaw = 0.0f;
	float spherePitch = 0.0f;
	XMMATRIX RotationX;
	XMMATRIX RotationY;
	XMMATRIX RotationZ;

	vector<Vertex3D> vertices(_numSphereVertices);

	XMVECTOR currVertPos = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

	vertices[0].pos.x = 0.0f;
	vertices[0].pos.y = 0.0f;
	vertices[0].pos.z = 1.0f;

	for ( int i = 0; i < _latLines - 2; i++ ) {
		spherePitch = (i + 1) * (3.14f / (_latLines - 1));
		RotationX = XMMatrixRotationX(spherePitch);
		for ( int j = 0; j < _longLines; j++ ) {
			sphereYaw = j * (6.28f / (_longLines));
			RotationY = XMMatrixRotationZ(sphereYaw);
			currVertPos = XMVector3TransformNormal(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), (RotationX * RotationY));
			currVertPos = XMVector3Normalize(currVertPos);
			vertices[i*_longLines + j + 1].pos.x = XMVectorGetX(currVertPos);
			vertices[i*_longLines + j + 1].pos.y = XMVectorGetY(currVertPos);
			vertices[i*_longLines + j + 1].pos.z = XMVectorGetZ(currVertPos);
		}
	}

	vertices[_numSphereVertices - 1].pos.x = 0.0f;
	vertices[_numSphereVertices - 1].pos.y = 0.0f;
	vertices[_numSphereVertices - 1].pos.z = -1.0f;

	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));

	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(Vertex3D) * _numSphereVertices;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA vertexBufferData;

	ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
	vertexBufferData.pSysMem = &vertices[0];
	HR(m_d3dDevice->CreateBuffer(&vertexBufferDesc, &vertexBufferData, _vertBuffer));

	std::vector<DWORD> indices(m_numSphereFaces * 3);

	int k = 0;
	for ( int l = 0; l < _longLines - 1; ++l ) {
		indices[k] = 0;
		indices[k + 1] = l + 1;
		indices[k + 2] = l + 2;
		k += 3;
	}

	indices[k] = 0;
	indices[k + 1] = _longLines;
	indices[k + 2] = 1;
	k += 3;

	for ( int i = 0; i < _latLines - 3; i++ ) {
		for ( int j = 0; j < _longLines - 1; j++ ) {
			indices[k] = i*_longLines + j + 1;
			indices[k + 1] = i*_longLines + j + 2;
			indices[k + 2] = (i + 1)*_longLines + j + 1;

			indices[k + 3] = (i + 1)*_longLines + j + 1;
			indices[k + 4] = i*_longLines + j + 2;
			indices[k + 5] = (i + 1)*_longLines + j + 2;

			k += 6; // next quad
		}

		indices[k] = (i*_longLines) + _longLines;
		indices[k + 1] = (i*_longLines) + 1;
		indices[k + 2] = ((i + 1)*_longLines) + _longLines;

		indices[k + 3] = ((i + 1)*_longLines) + _longLines;
		indices[k + 4] = (i*_longLines) + 1;
		indices[k + 5] = ((i + 1)*_longLines) + 1;

		k += 6;
	}

	for ( int l = 0; l < _longLines - 1; ++l ) {
		indices[k] = _numSphereVertices - 1;
		indices[k + 1] = (_numSphereVertices - 1) - (l + 1);
		indices[k + 2] = (_numSphereVertices - 1) - (l + 2);
		k += 3;
	}

	indices[k] = _numSphereVertices - 1;
	indices[k + 1] = (_numSphereVertices - 1) - _longLines;
	indices[k + 2] = _numSphereVertices - 2;

	D3D11_BUFFER_DESC indexBufferDesc;
	ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));

	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(DWORD) * m_numSphereFaces * 3;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA iinitData;

	iinitData.pSysMem = &indices[0];
	HR(m_d3dDevice->CreateBuffer(&indexBufferDesc, &iinitData, _indexBuffer));

}

void GuineaPig::BuildObjConstBuffer() {
	// objects
	D3D11_BUFFER_DESC cbbd;
	ZeroMemory(&cbbd, sizeof(D3D11_BUFFER_DESC));

	cbbd.Usage = D3D11_USAGE_DEFAULT;
	cbbd.ByteWidth = sizeof(ConstPerObject);
	cbbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbbd.CPUAccessFlags = 0;
	cbbd.MiscFlags = 0;

	HR(m_d3dDevice->CreateBuffer(&cbbd, NULL, &m_cbCubeBuffer));

	HR(m_d3dDevice->CreateBuffer(&cbbd, NULL, &m_cbGroundBuffer));
	m_cbGroundObject.texIndex = 0;

	// lightings
	ZeroMemory(&cbbd, sizeof(D3D11_BUFFER_DESC));
	cbbd.Usage = D3D11_USAGE_DEFAULT;
	cbbd.ByteWidth = sizeof(ConstPerFrame);
	cbbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbbd.CPUAccessFlags = 0;
	cbbd.MiscFlags = 0;
	HR(m_d3dDevice->CreateBuffer(&cbbd, NULL, &m_cbPerFrameBuffer));

}

void GuineaPig::BuildGeometryBuffers() {

	// Clockwise
	Vertex3D cubeVerteces[] = {
		// Front Face
		Vertex3D(XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f,  1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f)),	// A
		Vertex3D(XMFLOAT3(-1.0f,  1.0f, -1.0f), XMFLOAT2(0.0f,  0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f)),	// B
		Vertex3D(XMFLOAT3(+1.0f,  1.0f, -1.0f), XMFLOAT2(0.25f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f)),	// C
		Vertex3D(XMFLOAT3(+1.0f, -1.0f, -1.0f), XMFLOAT2(0.25f, 1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f)),	// D

																										// Back Face
		Vertex3D(XMFLOAT3(-1.0f, -1.0f, +1.0f), XMFLOAT2(0.25f, 1.0f), XMFLOAT3(0.0f, 0.0f, +1.0f)),
		Vertex3D(XMFLOAT3(+1.0f, -1.0f, +1.0f), XMFLOAT2(0.0f,  1.0f), XMFLOAT3(0.0f, 0.0f, +1.0f)),
		Vertex3D(XMFLOAT3(+1.0f,  1.0f, +1.0f), XMFLOAT2(0.0f,  0.0f), XMFLOAT3(0.0f, 0.0f, +1.0f)),
		Vertex3D(XMFLOAT3(-1.0f,  1.0f, +1.0f), XMFLOAT2(0.25f, 0.0f), XMFLOAT3(0.0f, 0.0f, +1.0f)),

		// Top Face
		Vertex3D(XMFLOAT3(-1.0f, +1.0f, -1.0f), XMFLOAT2(0.0f,  1.0f), XMFLOAT3(0.0f, +1.0f, 0.0f)),
		Vertex3D(XMFLOAT3(-1.0f, +1.0f,  1.0f), XMFLOAT2(0.0f,  0.0f), XMFLOAT3(0.0f, +1.0f, 0.0f)),
		Vertex3D(XMFLOAT3(+1.0f, +1.0f,  1.0f), XMFLOAT2(0.25f, 0.0f), XMFLOAT3(0.0f, +1.0f, 0.0f)),
		Vertex3D(XMFLOAT3(+1.0f, +1.0f, -1.0f), XMFLOAT2(0.25f, 1.0f), XMFLOAT3(0.0f, +1.0f, 0.0f)),

		// Bottom Face
		Vertex3D(XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(0.25f, 1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f)),
		Vertex3D(XMFLOAT3(+1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f,  1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f)),
		Vertex3D(XMFLOAT3(+1.0f, -1.0f,  1.0f), XMFLOAT2(0.0f,  0.0f), XMFLOAT3(0.0f, -1.0f, 0.0f)),
		Vertex3D(XMFLOAT3(-1.0f, -1.0f,  1.0f), XMFLOAT2(0.25f, 0.0f), XMFLOAT3(0.0f, -1.0f, 0.0f)),

		// Left Face
		Vertex3D(XMFLOAT3(-1.0f, -1.0f,  1.0f), XMFLOAT2(0.0f,  1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f)),
		Vertex3D(XMFLOAT3(-1.0f,  1.0f,  1.0f), XMFLOAT2(0.0f,  0.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f)),
		Vertex3D(XMFLOAT3(-1.0f,  1.0f, -1.0f), XMFLOAT2(0.25f, 0.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f)),
		Vertex3D(XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(0.25f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f)),

		// Right Face
		Vertex3D(XMFLOAT3(+1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f,  1.0f), XMFLOAT3(+1.0f, 0.0f, 0.0f)),
		Vertex3D(XMFLOAT3(+1.0f,  1.0f, -1.0f), XMFLOAT2(0.0f,  0.0f), XMFLOAT3(+1.0f, 0.0f, 0.0f)),
		Vertex3D(XMFLOAT3(+1.0f,  1.0f,  1.0f), XMFLOAT2(0.25f, 0.0f), XMFLOAT3(+1.0f, 0.0f, 0.0f)),
		Vertex3D(XMFLOAT3(+1.0f, -1.0f,  1.0f), XMFLOAT2(0.25f, 1.0f), XMFLOAT3(+1.0f, 0.0f, 0.0f)),
	};

	DWORD cubeIndices[] = {
		// Front Face
		0,  1,  2,
		0,  2,  3,

		// Back Face
		4,  5,  6,
		4,  6,  7,

		// Top Face
		8,  9, 10,
		8, 10, 11,

		// Bottom Face
		12, 13, 14,
		12, 14, 15,

		// Left Face
		16, 17, 18,
		16, 18, 19,

		// Right Face
		20, 21, 22,
		20, 22, 23
	};

	D3D11_BUFFER_DESC indexBufferDesc;
	ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));

	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.ByteWidth = sizeof(DWORD) * 12 * 3;

	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = cubeIndices;
	// Create Index Buffer
	HR(m_d3dDevice->CreateBuffer(&indexBufferDesc, &iinitData, &m_cubeIndexBuffer));
	// Set indeces buffer
	m_d3dImmediateContext->IASetIndexBuffer(m_cubeIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));

	vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = NULL;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.ByteWidth = static_cast<UINT>(sizeof(Vertex3D) * 24);

	D3D11_SUBRESOURCE_DATA vinitData;
	//vinitData.pSysMem = m_vertices.data();
	vinitData.pSysMem = cubeVerteces;

	// Create Verteces Buffer
	HR(m_d3dDevice->CreateBuffer(&vertexBufferDesc, &vinitData, &m_cubeVertexBuffer));
	// Set verteces buffer	- if changed over time, need to recall every frame
	//UINT stride = sizeof(Vertex3D), offset = 0;
	//m_d3dImmediateContext->IASetVertexBuffers(0, 1, &m_cubeVertexBuffer, &stride, &offset);

	BuildSphere(10, 10, &m_sphereVertBuffer, &m_sphereIndexBuffer, m_numSphereVertices, m_numSphereFaces);

	BuildGroundBuffers();
}

void GuineaPig::BuildGroundBuffers() {

	//Create the vertex buffer
	Vertex3D groundVerts[] = {
		Vertex3D(XMFLOAT3(-1.0f, 0.0f, -1.0f), XMFLOAT2(100.0f, 100.0f), XMFLOAT3(0.0f, 1.0f, 0.0f)),
		Vertex3D(XMFLOAT3(+1.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 100.0f), XMFLOAT3(0.0f, 1.0f, 0.0f)),
		Vertex3D(XMFLOAT3(+1.0f, 0.0f, +1.0f), XMFLOAT2(0.0f,   0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f)),
		Vertex3D(XMFLOAT3(-1.0f, 0.0f, +1.0f), XMFLOAT2(100.0f,   0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f)),
	};

	DWORD gourndIndices[] = {
		0,  1,  2,
		0,  2,  3,
	};

	D3D11_BUFFER_DESC indexBufferDesc;
	ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));

	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.ByteWidth = sizeof(DWORD) * 2 * 3;

	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = gourndIndices;
	// Create Index Buffer
	HR(m_d3dDevice->CreateBuffer(&indexBufferDesc, &iinitData, &m_groundIndexBuffer));


	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));

	vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = NULL;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.ByteWidth = static_cast<UINT>(sizeof(Vertex3D) * 4);

	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = groundVerts;

	// Create Verteces Buffer
	HR(m_d3dDevice->CreateBuffer(&vertexBufferDesc, &vinitData, &m_groundVertexBuffer));

}

void GuineaPig::BuildTextureAndState() {
	// loading the texture - using dds loader
	HR(CreateDDSTextureFromFile(m_d3dDevice, L"Resources/numbers_test.dds", NULL, &m_cubeShaderResView));
	HR(CreateDDSTextureFromFile(m_d3dDevice, L"Resources/Scenes/grass_diffuse.dds", NULL, &m_grassShaderResView));
	HR(CreateDDSTextureFromFile(m_d3dDevice, L"Resources/Skybox/skymap.dds", NULL, &m_skyboxShaderResView));

	// Describe the Sample State
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));

	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// Create the Sample State
	HR(m_d3dDevice->CreateSamplerState(&sampDesc, &m_baseTexSamplerState));

}

void GuineaPig::BuildLighting() {
	// Direction light setting
	//m_baseLight.direction = XMFLOAT3(0.0f, 1.0f, 0.0f);
	//m_baseLight.ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	//m_baseLight.diffuse = XMFLOAT4(1.2f, 1.2f, 1.2f, 1.2f);

	// Point light setting
	//m_baseLight.position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	//m_baseLight.range = 50.0f;
	//m_baseLight.attenuation = XMFLOAT3(0.0f, 0.1f, 0.0f);
	//m_baseLight.ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	//m_baseLight.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	// Point light setting
	m_baseLight.position = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_baseLight.spotLightDir = XMFLOAT3(0.0f, 0.0f, 1.0f);
	m_baseLight.range = 1000.0f;
	m_baseLight.cone = 20.0f;
	m_baseLight.attenuation = XMFLOAT3(0.4f, 0.02f, 0.0f);
	m_baseLight.ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	m_baseLight.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
}

void GuineaPig::BuildShader() {
	// normal shader
	HR(m_d3dDevice->CreateVertexShader(Base_VS, sizeof(Base_VS), NULL, &m_vertexShader));
	HR(m_d3dDevice->CreatePixelShader(Base_PS, sizeof(Base_PS), NULL, &m_pixelShader));

	// skybox shader
	HR(m_d3dDevice->CreateVertexShader(Skybox_VS, sizeof(Skybox_VS), NULL, &m_skyboxVertexShader));
	HR(m_d3dDevice->CreatePixelShader(Skybox_PS, sizeof(Skybox_PS), NULL, &m_skyboxPixelShader));

	//m_d3dImmediateContext->VSSetShader(m_vertexShader, NULL, 0);
	//m_d3dImmediateContext->PSSetShader(m_pixelShader, NULL, 0);
}

void GuineaPig::BuildVertexLayout() {

	D3D11_INPUT_ELEMENT_DESC vertLayout[] = {
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",		0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT",	0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	// Create the input layout
	HR(m_d3dDevice->CreateInputLayout(vertLayout, 5, Base_VS, sizeof(Base_VS), &m_inputLayout));

	D3D11_INPUT_ELEMENT_DESC skyboxVertLayout[] = {
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	// Create the input layout
	HR(m_d3dDevice->CreateInputLayout(skyboxVertLayout, 2, Skybox_VS, sizeof(Skybox_VS), &m_skyboxInputLayout));

}

void GuineaPig::BuildRenderStates() {

	// Raster Description	
	D3D11_RASTERIZER_DESC rasterDesc;
	ZeroMemory(&rasterDesc, sizeof(D3D11_RASTERIZER_DESC));

	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.CullMode = D3D11_CULL_NONE;
	rasterDesc.AntialiasedLineEnable = true;

	HR(m_d3dDevice->CreateRasterizerState(&rasterDesc, &m_antialiasedLine));

	// create blending description
	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(blendDesc));

	D3D11_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc;
	ZeroMemory(&renderTargetBlendDesc, sizeof(renderTargetBlendDesc));

	renderTargetBlendDesc.BlendEnable = true;
	renderTargetBlendDesc.SrcBlend = D3D11_BLEND_SRC_COLOR;
	renderTargetBlendDesc.DestBlend = D3D11_BLEND_BLEND_FACTOR;
	renderTargetBlendDesc.BlendOp = D3D11_BLEND_OP_ADD;
	renderTargetBlendDesc.SrcBlendAlpha = D3D11_BLEND_ONE;
	renderTargetBlendDesc.DestBlendAlpha = D3D11_BLEND_ZERO;
	renderTargetBlendDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	renderTargetBlendDesc.RenderTargetWriteMask = D3D10_COLOR_WRITE_ENABLE_ALL;

	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.RenderTarget[0] = renderTargetBlendDesc;

	HR(m_d3dDevice->CreateBlendState(&blendDesc, &m_blendTransparency));

	// create counter-clockwise and clockwise description
	D3D11_RASTERIZER_DESC cmdesc;
	ZeroMemory(&cmdesc, sizeof(D3D11_RASTERIZER_DESC));

	cmdesc.FillMode = D3D11_FILL_SOLID;
	cmdesc.CullMode = D3D11_CULL_BACK;

	cmdesc.FrontCounterClockwise = true;
	HR(m_d3dDevice->CreateRasterizerState(&cmdesc, &m_ccwCullingMode));

	cmdesc.FrontCounterClockwise = false;
	HR(m_d3dDevice->CreateRasterizerState(&cmdesc, &m_cwCullingMode));

	// skybox render state
	ZeroMemory(&cmdesc, sizeof(D3D11_RASTERIZER_DESC));
	cmdesc.FillMode = D3D11_FILL_SOLID;
	cmdesc.CullMode = D3D11_CULL_NONE;

	HR(m_d3dDevice->CreateRasterizerState(&cmdesc, &m_skyboxRasterState));

	D3D11_DEPTH_STENCIL_DESC dssDesc;
	ZeroMemory(&dssDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	dssDesc.DepthEnable = true;
	dssDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dssDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

	HR(m_d3dDevice->CreateDepthStencilState(&dssDesc, &m_skyboxDSLessEqual));
}

void GuineaPig::UpdateScene(double _dt) {

	// Update skybox

	//Reset sphereWorld
	m_sphereWorld = XMMatrixIdentity();

	//Define sphereWorld's world space matrix
	XMMATRIX scale = XMMatrixScaling(5.0f, 5.0f, 5.0f);
	//Make sure the sphere is always centered around camera
	XMMATRIX translation = XMMatrixTranslation(XMVectorGetX(m_camPosition), XMVectorGetY(m_camPosition), XMVectorGetZ(m_camPosition));

	//Set sphereWorld's world space using the transformations
	m_sphereWorld = scale * translation;

	// Update point light position

	//Reset Lights Position
	//XMVECTOR lightVector = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	//lightVector = XMVector3TransformCoord(lightVector, cubeWorldMat * XMMatrixTranslation(0, 5.0f, 0));
	//m_baseLight.position.x = XMVectorGetX(lightVector);
	//m_baseLight.position.y = XMVectorGetY(lightVector);
	//m_baseLight.position.z = XMVectorGetZ(lightVector);

	m_baseLight.position.x = XMVectorGetX(m_camPosition);
	m_baseLight.position.y = XMVectorGetY(m_camPosition);
	m_baseLight.position.z = XMVectorGetZ(m_camPosition);

	m_baseLight.spotLightDir.x = XMVectorGetX(m_camTarget) - m_baseLight.position.x;
	m_baseLight.spotLightDir.y = XMVectorGetY(m_camTarget) - m_baseLight.position.y;
	m_baseLight.spotLightDir.z = XMVectorGetZ(m_camTarget) - m_baseLight.position.z;

	// Update objects
	static double texIdx = 0;
	static float rot = 0.00f;

	rot += (float)_dt;

	if ( rot > 6.26f ) rot = 0.0f;

	// update cube animation (TexCoord Index)
	texIdx += _dt;
	m_cbCubeObject.texIndex = (int)texIdx;
	if ( (int)texIdx > 3 ) texIdx = 0;

	m_cbCubeObject.hasNormal = false;
	m_cbCubeObject.hasTexture = false;
	m_cbGroundObject.hasNormal = false;
	m_cbGroundObject.hasTexture = false;

	m_groundWorldMat = XMMatrixIdentity();
	m_groundWorldMat = XMMatrixScaling(500.0f, 1.0f, 500.0f)*XMMatrixTranslation(0, 0.0f, 0);

	m_cubeWorldMat = XMMatrixIdentity();
	XMVECTOR rotAxis_X = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR rotAxis_Y = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMVECTOR rotAxis_Z = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

	XMMATRIX rotationMat = XMMatrixRotationAxis(rotAxis_X, rot) * XMMatrixRotationAxis(rotAxis_Y, rot);
	XMMATRIX translationMat = XMMatrixTranslation(0.0f, 2.0f, 0.0f);

	m_cubeWorldMat = rotationMat * translationMat;
}

void GuineaPig::DrawScene() {
	assert(m_d3dImmediateContext);
	assert(m_swapChain);

	//Refresh the render target view
	m_d3dImmediateContext->ClearRenderTargetView(m_renderTargetView, reinterpret_cast<const float*>(&Colors::Black));

	//Refresh the Depth/Stencil view
	m_d3dImmediateContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	UINT stride = sizeof(Vertex3D), offset = 0;

	m_d3dImmediateContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// opaque objects drawing

	// draw skybox
	//Set the default blend state (no blending) for opaque objects
	m_d3dImmediateContext->OMSetBlendState(0, 0, 0xffffffff);
	//Set the spheres index buffer
	m_d3dImmediateContext->IASetIndexBuffer(m_sphereIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	//Set the spheres vertex buffer
	m_d3dImmediateContext->IASetVertexBuffers(0, 1, &m_sphereVertBuffer, &stride, &offset);

	//Set the WVP matrix and send it to the constant buffer in effect file
	m_cbGroundObject.WVP = XMMatrixTranspose(m_sphereWorld * m_camView * m_camProjection);
	m_cbGroundObject.World = XMMatrixTranspose(m_sphereWorld);
	m_d3dImmediateContext->UpdateSubresource(m_cbGroundBuffer, 0, NULL, &m_cbGroundObject, 0, 0);
	m_d3dImmediateContext->VSSetConstantBuffers(0, 1, &m_cbGroundBuffer);
	//Send our skymap resource view to pixel shader
	m_d3dImmediateContext->PSSetShaderResources(0, 1, &m_skyboxShaderResView);
	m_d3dImmediateContext->PSSetSamplers(0, 1, &m_baseTexSamplerState);

	//Set the proper VS and PS shaders, and layout
	m_d3dImmediateContext->VSSetShader(m_skyboxVertexShader, 0, 0);
	m_d3dImmediateContext->PSSetShader(m_skyboxPixelShader, 0, 0);
	m_d3dImmediateContext->IASetInputLayout(m_skyboxInputLayout);
	//Set the new depth/stencil and RS states
	m_d3dImmediateContext->OMSetDepthStencilState(m_skyboxDSLessEqual, 0);
	m_d3dImmediateContext->RSSetState(m_skyboxRasterState);

	m_d3dImmediateContext->DrawIndexed(m_numSphereFaces * 3, 0, 0);

	//"fine-tune" the blending equation
	float blendFactor[] = { 0.75f, 0.75f, 0.75f, 1.0f };

	//Set the default blend state (no blending) for opaque objects
	m_d3dImmediateContext->OMSetBlendState(0, 0, 0xffffffff);

	// apply lighting
	m_cbPerFrame.baseLight = m_baseLight;
	m_d3dImmediateContext->UpdateSubresource(m_cbPerFrameBuffer, 0, NULL, &m_cbPerFrame, 0, 0);
	m_d3dImmediateContext->PSSetConstantBuffers(0, 1, &m_cbPerFrameBuffer);

	// Set the default VS shader and depth/stencil state and layout
	m_d3dImmediateContext->VSSetShader(m_vertexShader, NULL, 0);
	m_d3dImmediateContext->PSSetShader(m_pixelShader, NULL, 0);
	m_d3dImmediateContext->OMSetDepthStencilState(NULL, 0);
	m_d3dImmediateContext->IASetInputLayout(m_inputLayout);



	// Set Shader Resources and Samplers
	m_d3dImmediateContext->PSSetShaderResources(0, 1, &m_grassShaderResView);
	m_d3dImmediateContext->PSSetSamplers(0, 1, &m_baseTexSamplerState);

	// Draw Ground
	m_cbGroundObject.WVP = XMMatrixTranspose(m_groundWorldMat * m_camView * m_camProjection);
	m_cbGroundObject.World = XMMatrixTranspose(m_groundWorldMat);
	m_d3dImmediateContext->UpdateSubresource(m_cbGroundBuffer, 0, NULL, &m_cbGroundObject, 0, 0);
	m_d3dImmediateContext->VSSetConstantBuffers(0, 1, &m_cbGroundBuffer);
	// Set verts buffer
	m_d3dImmediateContext->IASetVertexBuffers(0, 1, &m_groundVertexBuffer, &stride, &offset);
	// Set indeces buffer
	m_d3dImmediateContext->IASetIndexBuffer(m_groundIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	m_d3dImmediateContext->RSSetState(m_antialiasedLine);
	m_d3dImmediateContext->DrawIndexed(6, 0, 0);

	// Set Shader Resources and Samplers
	m_d3dImmediateContext->PSSetShaderResources(0, 1, &m_cubeShaderResView);
	m_d3dImmediateContext->PSSetSamplers(0, 1, &m_baseTexSamplerState);

	// Render opaque objects //

	//*****Transparency Depth Ordering*****//
	// Find which transparent object is further from the camera
	// So we can render the objects in depth order to the render target
	// Puting the objects into a vector to organize the order of distance
	// Find distance from cube to camera
	//XMVECTOR objPos = XMVectorZero();

	//objPos = XMVector3TransformCoord(objPos, cubeWorldMat);

	//float distX = XMVectorGetX(objPos) - XMVectorGetX(camPosition);
	//float distY = XMVectorGetY(objPos) - XMVectorGetY(camPosition);
	//float distZ = XMVectorGetZ(objPos) - XMVectorGetZ(camPosition);

	//float cubeDist = distX*distX + distY*distY + distZ*distZ;

	//objPos = XMVectorZero();

	//objPos = XMVector3TransformCoord(objPos, groundWorldMat);

	//distX = XMVectorGetX(objPos) - XMVectorGetX(camPosition);
	//distY = XMVectorGetY(objPos) - XMVectorGetY(camPosition);
	//distZ = XMVectorGetZ(objPos) - XMVectorGetZ(camPosition);

	//float gridDist = distX*distX + distY*distY + distZ*distZ;

	//Set the blend state for transparent objects
	//m_d3dImmediateContext->OMSetBlendState(m_blendTransparency, blendFactor, 0xffffffff);

	// draw two cubes
	m_cbCubeObject.WVP = XMMatrixTranspose(m_cubeWorldMat * m_camView * m_camProjection);
	m_cbCubeObject.World = XMMatrixTranspose(m_cubeWorldMat);
	m_d3dImmediateContext->UpdateSubresource(m_cbCubeBuffer, 0, NULL, &m_cbCubeObject, 0, 0);
	m_d3dImmediateContext->VSSetConstantBuffers(0, 1, &m_cbCubeBuffer);
	// Set verts buffer
	m_d3dImmediateContext->IASetVertexBuffers(0, 1, &m_cubeVertexBuffer, &stride, &offset);
	// Set indeces buffer
	m_d3dImmediateContext->IASetIndexBuffer(m_cubeIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Send conterclockwise culling cube first!
	m_d3dImmediateContext->RSSetState(m_ccwCullingMode);
	m_d3dImmediateContext->DrawIndexed(36, 0, 0);

	// Send clockwise culling cube following the conter-colockwise culling cube!
	m_d3dImmediateContext->RSSetState(m_cwCullingMode);
	m_d3dImmediateContext->DrawIndexed(36, 0, 0);

	//Present the backbuffer to the screen
	HR(m_swapChain->Present(0, 0));
}

void GuineaPig::UpdateKeyboardInput(double _dt) {

	if ( GetAsyncKeyState(VK_LW) ) {
		m_moveBackForward += (float)_dt * 10.0f;
	}

	if ( GetAsyncKeyState(VK_LS) ) {
		m_moveBackForward -= (float)_dt * 10.0f;
	}

	if ( GetAsyncKeyState(VK_LA) ) {
		m_moveLeftRight -= (float)_dt * 10.0f;
	}

	if ( GetAsyncKeyState(VK_LD) ) {
		m_moveLeftRight += (float)_dt * 10.0f;
	}

}

void GuineaPig::UpdateCamera() {
	D3DApp::UpdateCamera();
}

void GuineaPig::OnMouseDown(WPARAM _btnState, int _x, int _y) {
	m_lastMousePos.x = _x;
	m_lastMousePos.y = _y;

	SetCapture(m_hWindow);
}

void GuineaPig::OnMouseUp(WPARAM _btnState, int _x, int _y) {
	ReleaseCapture();
}

void GuineaPig::OnMouseMove(WPARAM _btnState, int _x, int _y) {
	if ( (MK_RBUTTON & _btnState) != 0 ) {

		m_camYaw += 0.01f*(_x - m_lastMousePos.x);
		m_camPitch += 0.01f*(_y - m_lastMousePos.y);

		m_camPitch = Mathlib::Clamp(m_camPitch, -XM_PIDIV2 + 0.01f, XM_PIDIV2 - 0.01f);
	}

	m_lastMousePos.x = _x;
	m_lastMousePos.y = _y;
}

bool GuineaPig::CreateModelFromObjFile(
	wstring _filename,
	ID3D11Buffer ** _vertBuff,
	ID3D11Buffer ** _indexBuff,
	vector<int>& _subsetIndexStart,
	vector<int>& _subsetMaterialArray,
	vector<SurfaceMaterial>& _material,
	int & _subsetCount,
	bool _isRHCoordSys,
	bool _computeNormals) {

	HRESULT hr = 0;

	wifstream fileIn(_filename.c_str());	//Open file
	wstring meshMatLib;						//String to hold our obj material library _filename

											//Arrays to store our model's information
	vector<DWORD> indices;
	vector<XMFLOAT3> vertPos;
	vector<XMFLOAT3> vertNorm;
	vector<XMFLOAT2> vertTexCoord;
	vector<std::wstring> meshMaterials;

	//Vertex definition indices
	vector<int> vertPosIndex;
	vector<int> vertNormIndex;
	vector<int> vertTCIndex;

	//Make sure we have a default if no tex coords or normals are defined
	bool hasTexCoord = false;
	bool hasNorm = false;

	//Temp variables to store into vectors
	std::wstring meshMaterialsTemp;
	int vertPosIndexTemp;
	int vertNormIndexTemp;
	int vertTCIndexTemp;

	wchar_t checkChar;		//The variable we will use to store one char from file at a time
	std::wstring face;		//Holds the string containing our face vertices
	int vIndex = 0;			//Keep track of our vertex index count
	int triangleCount = 0;	//Total Triangles
	int totalVerts = 0;
	int meshTriangles = 0;

	//Check to see if the file was opened
	if ( fileIn ) {
		while ( fileIn ) {
			checkChar = fileIn.get();	//Get next char

			switch ( checkChar ) {
			case '#':
				checkChar = fileIn.get();
				while ( checkChar != '\n' )
					checkChar = fileIn.get();
				break;
			case 'v':	//Get Vertex Descriptions
				checkChar = fileIn.get();
				if ( checkChar == ' ' )	//v - vert position
				{
					float vz, vy, vx;
					fileIn >> vx >> vy >> vz;	//Store the next three types

					if ( _isRHCoordSys )	//If model is from an RH Coord System
						vertPos.push_back(XMFLOAT3(vx, vy, vz * -1.0f));	//Invert the Z axis
					else
						vertPos.push_back(XMFLOAT3(vx, vy, vz));
				}
				if ( checkChar == 't' )	//vt - vert tex coords
				{
					float vtcu, vtcv;
					fileIn >> vtcu >> vtcv;		//Store next two types

					if ( _isRHCoordSys )	//If model is from an RH Coord System
						vertTexCoord.push_back(XMFLOAT2(vtcu, 1.0f - vtcv));	//Reverse the "v" axis
					else
						vertTexCoord.push_back(XMFLOAT2(vtcu, vtcv));

					hasTexCoord = true;	//We know the model uses texture coords
				}
				//Since we compute the normals later, we don't need to check for normals
				//In the file, but i'll do it here anyway
				if ( checkChar == 'n' )	//vn - vert normal
				{
					float vnx, vny, vnz;
					fileIn >> vnx >> vny >> vnz;	//Store next three types

					if ( _isRHCoordSys )	//If model is from an RH Coord System
						vertNorm.push_back(XMFLOAT3(vnx, vny, vnz * -1.0f));	//Invert the Z axis
					else
						vertNorm.push_back(XMFLOAT3(vnx, vny, vnz));

					hasNorm = true;	//We know the model defines normals
				}
				break;

				//New group (Subset)
			case 'g':	//g - defines a group
				checkChar = fileIn.get();
				if ( checkChar == ' ' ) {
					_subsetIndexStart.push_back(vIndex);		//Start index for this subset
					_subsetCount++;
				}
				break;

				//Get Face Index
			case 'f':	//f - defines the faces
				checkChar = fileIn.get();
				if ( checkChar == ' ' ) {
					face = L"";
					std::wstring VertDef;	//Holds one vertex definition at a time
					triangleCount = 0;

					checkChar = fileIn.get();
					while ( checkChar != '\n' ) {
						face += checkChar;			//Add the char to our face string
						checkChar = fileIn.get();	//Get the next Character
						if ( checkChar == ' ' )		//If its a space...
							triangleCount++;		//Increase our triangle count
					}

					//Check for space at the end of our face string
					if ( face[face.length() - 1] == ' ' )
						triangleCount--;	//Each space adds to our triangle count

					triangleCount -= 1;		//Ever vertex in the face AFTER the first two are new faces

					std::wstringstream ss(face);

					if ( face.length() > 0 ) {
						int firstVIndex, lastVIndex;	//Holds the first and last vertice's index

						for ( int i = 0; i < 3; ++i )		//First three vertices (first triangle)
						{
							ss >> VertDef;	//Get vertex definition (vPos/vTexCoord/vNorm)

							std::wstring vertPart;
							int whichPart = 0;		//(vPos, vTexCoord, or vNorm)

													//Parse this string
							for ( int j = 0; j < VertDef.length(); ++j ) {
								if ( VertDef[j] != '/' )	//If there is no divider "/", add a char to our vertPart
									vertPart += VertDef[j];

								//If the current char is a divider "/", or its the last character in the string
								if ( VertDef[j] == '/' || j == VertDef.length() - 1 ) {
									std::wistringstream wstringToInt(vertPart);	//Used to convert wstring to int

									if ( whichPart == 0 )	//If vPos
									{
										wstringToInt >> vertPosIndexTemp;
										vertPosIndexTemp -= 1;		//subtract one since c++ arrays start with 0, and obj start with 1

																	//Check to see if the vert pos was the only thing specified
										if ( j == VertDef.length() - 1 ) {
											vertNormIndexTemp = 0;
											vertTCIndexTemp = 0;
										}
									}

									else if ( whichPart == 1 )	//If vTexCoord
									{
										if ( vertPart != L"" )	//Check to see if there even is a tex coord
										{
											wstringToInt >> vertTCIndexTemp;
											vertTCIndexTemp -= 1;	//subtract one since c++ arrays start with 0, and obj start with 1
										} else	//If there is no tex coord, make a default
											vertTCIndexTemp = 0;

										//If the cur. char is the second to last in the string, then
										//there must be no normal, so set a default normal
										if ( j == VertDef.length() - 1 )
											vertNormIndexTemp = 0;

									} else if ( whichPart == 2 )	//If vNorm
									{
										std::wistringstream wstringToInt(vertPart);

										wstringToInt >> vertNormIndexTemp;
										vertNormIndexTemp -= 1;		//subtract one since c++ arrays start with 0, and obj start with 1
									}

									vertPart = L"";	//Get ready for next vertex part
									whichPart++;	//Move on to next vertex part					
								}
							}

							//Check to make sure there is at least one subset
							if ( _subsetCount == 0 ) {
								_subsetIndexStart.push_back(vIndex);		//Start index for this subset
								_subsetCount++;
							}

							//Avoid duplicate vertices
							bool vertAlreadyExists = false;
							if ( totalVerts >= 3 )	//Make sure we at least have one triangle to check
							{
								//Loop through all the vertices
								for ( int iCheck = 0; iCheck < totalVerts; ++iCheck ) {
									//If the vertex position and texture coordinate in memory are the same
									//As the vertex position and texture coordinate we just now got out
									//of the obj file, we will set this faces vertex index to the vertex's
									//index value in memory. This makes sure we don't create duplicate vertices
									if ( vertPosIndexTemp == vertPosIndex[iCheck] && !vertAlreadyExists ) {
										if ( vertTCIndexTemp == vertTCIndex[iCheck] ) {
											indices.push_back(iCheck);		//Set index for this vertex
											vertAlreadyExists = true;		//If we've made it here, the vertex already exists
										}
									}
								}
							}

							//If this vertex is not already in our vertex arrays, put it there
							if ( !vertAlreadyExists ) {
								vertPosIndex.push_back(vertPosIndexTemp);
								vertTCIndex.push_back(vertTCIndexTemp);
								vertNormIndex.push_back(vertNormIndexTemp);
								totalVerts++;	//We created a new vertex
								indices.push_back(totalVerts - 1);	//Set index for this vertex
							}

							//If this is the very first vertex in the face, we need to
							//make sure the rest of the triangles use this vertex
							if ( i == 0 ) {
								firstVIndex = indices[vIndex];	//The first vertex index of this FACE

							}

							//If this was the last vertex in the first triangle, we will make sure
							//the next triangle uses this one (eg. tri1(1,2,3) tri2(1,3,4) tri3(1,4,5))
							if ( i == 2 ) {
								lastVIndex = indices[vIndex];	//The last vertex index of this TRIANGLE
							}
							vIndex++;	//Increment index count
						}

						meshTriangles++;	//One triangle down

											//If there are more than three vertices in the face definition, we need to make sure
											//we convert the face to triangles. We created our first triangle above, now we will
											//create a new triangle for every new vertex in the face, using the very first vertex
											//of the face, and the last vertex from the triangle before the current triangle
						for ( int l = 0; l < triangleCount - 1; ++l )	//Loop through the next vertices to create new triangles
						{
							//First vertex of this triangle (the very first vertex of the face too)
							indices.push_back(firstVIndex);			//Set index for this vertex
							vIndex++;

							//Second Vertex of this triangle (the last vertex used in the tri before this one)
							indices.push_back(lastVIndex);			//Set index for this vertex
							vIndex++;

							//Get the third vertex for this triangle
							ss >> VertDef;

							std::wstring vertPart;
							int whichPart = 0;

							//Parse this string (same as above)
							for ( int j = 0; j < VertDef.length(); ++j ) {
								if ( VertDef[j] != '/' )
									vertPart += VertDef[j];
								if ( VertDef[j] == '/' || j == VertDef.length() - 1 ) {
									std::wistringstream wstringToInt(vertPart);

									if ( whichPart == 0 ) {
										wstringToInt >> vertPosIndexTemp;
										vertPosIndexTemp -= 1;

										//Check to see if the vert pos was the only thing specified
										if ( j == VertDef.length() - 1 ) {
											vertTCIndexTemp = 0;
											vertNormIndexTemp = 0;
										}
									} else if ( whichPart == 1 ) {
										if ( vertPart != L"" ) {
											wstringToInt >> vertTCIndexTemp;
											vertTCIndexTemp -= 1;
										} else
											vertTCIndexTemp = 0;
										if ( j == VertDef.length() - 1 )
											vertNormIndexTemp = 0;

									} else if ( whichPart == 2 ) {
										std::wistringstream wstringToInt(vertPart);

										wstringToInt >> vertNormIndexTemp;
										vertNormIndexTemp -= 1;
									}

									vertPart = L"";
									whichPart++;
								}
							}

							//Check for duplicate vertices
							bool vertAlreadyExists = false;
							if ( totalVerts >= 3 )	//Make sure we at least have one triangle to check
							{
								for ( int iCheck = 0; iCheck < totalVerts; ++iCheck ) {
									if ( vertPosIndexTemp == vertPosIndex[iCheck] && !vertAlreadyExists ) {
										if ( vertTCIndexTemp == vertTCIndex[iCheck] ) {
											indices.push_back(iCheck);			//Set index for this vertex
											vertAlreadyExists = true;		//If we've made it here, the vertex already exists
										}
									}
								}
							}

							if ( !vertAlreadyExists ) {
								vertPosIndex.push_back(vertPosIndexTemp);
								vertTCIndex.push_back(vertTCIndexTemp);
								vertNormIndex.push_back(vertNormIndexTemp);
								totalVerts++;					//New vertex created, add to total verts
								indices.push_back(totalVerts - 1);		//Set index for this vertex
							}

							//Set the second vertex for the next triangle to the last vertex we got		
							lastVIndex = indices[vIndex];	//The last vertex index of this TRIANGLE

							meshTriangles++;	//New triangle defined
							vIndex++;
						}
					}
				}
				break;

			case 'm':	//mtllib - material library _filename
				checkChar = fileIn.get();
				if ( checkChar == 't' ) {
					checkChar = fileIn.get();
					if ( checkChar == 'l' ) {
						checkChar = fileIn.get();
						if ( checkChar == 'l' ) {
							checkChar = fileIn.get();
							if ( checkChar == 'i' ) {
								checkChar = fileIn.get();
								if ( checkChar == 'b' ) {
									checkChar = fileIn.get();
									if ( checkChar == ' ' ) {
										//Store the material libraries file name
										fileIn >> meshMatLib;
									}
								}
							}
						}
					}
				}

				break;

			case 'u':	//usemtl - which material to use
				checkChar = fileIn.get();
				if ( checkChar == 's' ) {
					checkChar = fileIn.get();
					if ( checkChar == 'e' ) {
						checkChar = fileIn.get();
						if ( checkChar == 'm' ) {
							checkChar = fileIn.get();
							if ( checkChar == 't' ) {
								checkChar = fileIn.get();
								if ( checkChar == 'l' ) {
									checkChar = fileIn.get();
									if ( checkChar == ' ' ) {
										meshMaterialsTemp = L"";	//Make sure this is cleared

										fileIn >> meshMaterialsTemp; //Get next type (string)

										meshMaterials.push_back(meshMaterialsTemp);
									}
								}
							}
						}
					}
				}
				break;

			default:
				break;
			}
		}
	} else	//If we could not open the file
	{
		m_swapChain->SetFullscreenState(false, NULL);	//Make sure we are out of fullscreen

														//create message
		std::wstring message = L"Could not open: ";
		message += _filename;

		MessageBox(0, message.c_str(),	//display message
			L"Error", MB_OK);

		return false;
	}

	_subsetIndexStart.push_back(vIndex); //There won't be another index start after our last subset, so set it here

										 //sometimes "g" is defined at the very top of the file, then again before the first group of faces.
										 //This makes sure the first subset does not conatain "0" indices.
	if ( _subsetIndexStart[1] == 0 ) {
		_subsetIndexStart.erase(_subsetIndexStart.begin() + 1);
		m_meshSubsets--;
	}

	//Make sure we have a default for the tex coord and normal
	//if one or both are not specified
	if ( !hasNorm )
		vertNorm.push_back(XMFLOAT3(0.0f, 0.0f, 0.0f));
	if ( !hasTexCoord )
		vertTexCoord.push_back(XMFLOAT2(0.0f, 0.0f));

	//Close the obj file, and open the mtl file
	fileIn.close();
	fileIn.open(meshMatLib.c_str());

	std::wstring lastStringRead;
	size_t matCount = m_materials.size();	//total materials

											//kdset - If our diffuse color was not set, we can use the ambient color (which is usually the same)
											//If the diffuse color WAS set, then we don't need to set our diffuse color to ambient
	bool kdset = false;

	if ( fileIn ) {
		while ( fileIn ) {
			checkChar = fileIn.get();	//Get next char

			switch ( checkChar ) {
				//Check for comment
			case '#':
				checkChar = fileIn.get();
				while ( checkChar != '\n' )
					checkChar = fileIn.get();
				break;

				//Set diffuse color
			case 'K':
				checkChar = fileIn.get();
				if ( checkChar == 'd' )	//Diffuse Color
				{
					checkChar = fileIn.get();	//remove space

					fileIn >> m_materials[matCount - 1].difColor.x;
					fileIn >> m_materials[matCount - 1].difColor.y;
					fileIn >> m_materials[matCount - 1].difColor.z;

					kdset = true;
				}

				//Ambient Color (We'll store it in diffuse if there isn't a diffuse already)
				if ( checkChar == 'a' ) {
					checkChar = fileIn.get();	//remove space
					if ( !kdset ) {
						fileIn >> m_materials[matCount - 1].difColor.x;
						fileIn >> m_materials[matCount - 1].difColor.y;
						fileIn >> m_materials[matCount - 1].difColor.z;
					}
				}
				break;

				//Check for transparency
			case 'T':
				checkChar = fileIn.get();
				if ( checkChar == 'r' ) {
					checkChar = fileIn.get();	//remove space
					float Transparency;
					fileIn >> Transparency;

					m_materials[matCount - 1].difColor.w = Transparency;

					if ( Transparency > 0.0f )
						m_materials[matCount - 1].transparent = true;
				}
				break;

				//Some obj files specify d for transparency
			case 'd':
				checkChar = fileIn.get();
				if ( checkChar == ' ' ) {
					float Transparency;
					fileIn >> Transparency;

					//'d' - 0 being most transparent, and 1 being opaque, opposite of Tr
					Transparency = 1.0f - Transparency;

					m_materials[matCount - 1].difColor.w = Transparency;

					if ( Transparency > 0.0f )
						m_materials[matCount - 1].transparent = true;
				}
				break;

				//Get the diffuse map (texture)
			case 'm':
				checkChar = fileIn.get();
				if ( checkChar == 'a' ) {
					checkChar = fileIn.get();
					if ( checkChar == 'p' ) {
						checkChar = fileIn.get();
						if ( checkChar == '_' ) {
							//map_Kd - Diffuse map
							checkChar = fileIn.get();
							if ( checkChar == 'K' ) {
								checkChar = fileIn.get();
								if ( checkChar == 'd' ) {
									std::wstring fileNamePath;

									fileIn.get();	//Remove whitespace between map_Kd and file

													//Get the file path - We read the pathname char by char since
													//pathnames can sometimes contain spaces, so we will read until
													//we find the file extension
									bool texFilePathEnd = false;
									while ( !texFilePathEnd ) {
										checkChar = fileIn.get();

										fileNamePath += checkChar;

										if ( checkChar == '.' ) {
											for ( int i = 0; i < 3; ++i )
												fileNamePath += fileIn.get();

											texFilePathEnd = true;
										}
									}

									//check if this texture has already been loaded
									bool alreadyLoaded = false;
									for ( int i = 0; i < m_textureNameArray.size(); ++i ) {
										if ( fileNamePath == m_textureNameArray[i] ) {
											alreadyLoaded = true;
											m_materials[matCount - 1].texArrayIndex = i;
											m_materials[matCount - 1].hasTexture = true;
										}
									}

									//if the texture is not already loaded, load it now
									if ( !alreadyLoaded ) {
										ID3D11ShaderResourceView* tempMeshSRV;
										hr = CreateDDSTextureFromFile(m_d3dDevice, fileNamePath.c_str(), NULL, &tempMeshSRV);
										//hr = D3DX11CreateShaderResourceViewFromFile(d3d11Device, fileNamePath.c_str(), NULL, NULL, &tempMeshSRV, NULL);

										if ( SUCCEEDED(hr) ) {
											m_textureNameArray.push_back(fileNamePath.c_str());
											m_materials[matCount - 1].texArrayIndex = (int)m_meshShaderResView.size();
											m_meshShaderResView.push_back(tempMeshSRV);
											m_materials[matCount - 1].hasTexture = true;
										}
									}
								}
							}
							//map_d - alpha map
							else if ( checkChar == 'd' ) {
								//Alpha maps are usually the same as the diffuse map
								//So we will assume that for now by only enabling
								//transparency for this material, as we will already
								//be using the alpha channel in the diffuse map
								m_materials[matCount - 1].transparent = true;
							}
							//map_bump - bump map (we're usinga normal map though)
							else if ( checkChar == 'b' ) {
								checkChar = fileIn.get();
								if ( checkChar == 'u' ) {
									checkChar = fileIn.get();
									if ( checkChar == 'm' ) {
										checkChar = fileIn.get();
										if ( checkChar == 'p' ) {
											std::wstring fileNamePath;

											fileIn.get();	//Remove whitespace between map_bump and file

															//Get the file path - We read the pathname char by char since
															//pathnames can sometimes contain spaces, so we will read until
															//we find the file extension
											bool texFilePathEnd = false;
											while ( !texFilePathEnd ) {
												checkChar = fileIn.get();

												fileNamePath += checkChar;

												if ( checkChar == '.' ) {
													for ( int i = 0; i < 3; ++i )
														fileNamePath += fileIn.get();

													texFilePathEnd = true;
												}
											}

											//check if this texture has already been loaded
											bool alreadyLoaded = false;
											for ( int i = 0; i < m_textureNameArray.size(); ++i ) {
												if ( fileNamePath == m_textureNameArray[i] ) {
													alreadyLoaded = true;
													m_materials[matCount - 1].normMapTexArrayIndex = i;
													m_materials[matCount - 1].hasNormMap = true;
												}
											}

											//if the texture is not already loaded, load it now
											if ( !alreadyLoaded ) {
												ID3D11ShaderResourceView* tempMeshSRV;
												/*hr = D3DX11CreateShaderResourceViewFromFile(d3d11Device, fileNamePath.c_str(),
												NULL, NULL, &tempMeshSRV, NULL);*/
												hr = CreateDDSTextureFromFile(m_d3dDevice, fileNamePath.c_str(), NULL, &tempMeshSRV);
												if ( SUCCEEDED(hr) ) {
													m_textureNameArray.push_back(fileNamePath.c_str());
													m_materials[matCount - 1].normMapTexArrayIndex = (int)m_meshShaderResView.size();
													m_meshShaderResView.push_back(tempMeshSRV);
													m_materials[matCount - 1].hasNormMap = true;
												}
											}
										}
									}
								}
							}
						}
					}
				}
				break;

			case 'n':	//newmtl - Declare new material
				checkChar = fileIn.get();
				if ( checkChar == 'e' ) {
					checkChar = fileIn.get();
					if ( checkChar == 'w' ) {
						checkChar = fileIn.get();
						if ( checkChar == 'm' ) {
							checkChar = fileIn.get();
							if ( checkChar == 't' ) {
								checkChar = fileIn.get();
								if ( checkChar == 'l' ) {
									checkChar = fileIn.get();
									if ( checkChar == ' ' ) {
										//New material, set its defaults
										SurfaceMaterial tempMat;
										m_materials.push_back(tempMat);
										fileIn >> m_materials[matCount].matName;
										m_materials[matCount].transparent = false;
										m_materials[matCount].hasTexture = false;
										m_materials[matCount].hasNormMap = false;
										m_materials[matCount].normMapTexArrayIndex = 0;
										m_materials[matCount].texArrayIndex = 0;
										matCount++;
										kdset = false;
									}
								}
							}
						}
					}
				}
				break;

			default:
				break;
			}
		}
	} else {
		m_swapChain->SetFullscreenState(false, NULL);	//Make sure we are out of fullscreen

		std::wstring message = L"Could not open: ";
		message += meshMatLib;

		MessageBox(0, message.c_str(),
			L"Error", MB_OK);

		return false;
	}

	//Set the subsets material to the index value
	//of the its material in our material array
	for ( int i = 0; i < m_meshSubsets; ++i ) {
		bool hasMat = false;
		for ( int j = 0; j < m_materials.size(); ++j ) {
			if ( meshMaterials[i] == m_materials[j].matName ) {
				_subsetMaterialArray.push_back(j);
				hasMat = true;
			}
		}
		if ( !hasMat )
			_subsetMaterialArray.push_back(0); //Use first material in array
	}

	std::vector<Vertex3D> vertices;
	Vertex3D tempVert;

	//Create our vertices using the information we got 
	//from the file and store them in a vector
	for ( int j = 0; j < totalVerts; ++j ) {
		tempVert.pos = vertPos[vertPosIndex[j]];
		tempVert.normal = vertNorm[vertNormIndex[j]];
		tempVert.texCoord = vertTexCoord[vertTCIndex[j]];

		vertices.push_back(tempVert);
	}

	//////////////////////Compute Normals///////////////////////////
	//If computeNormals was set to true then we will create our own
	//normals, if it was set to false we will use the obj files normals
	if ( _computeNormals ) {
		vector<XMFLOAT3> tempNormal;

		//normalized and unnormalized normals
		XMFLOAT3 unnormalized = XMFLOAT3(0.0f, 0.0f, 0.0f);

		//tangent stuff
		vector<XMFLOAT3> tempTangent;
		XMFLOAT3 tangent = XMFLOAT3(0.0f, 0.0f, 0.0f);
		float tcU1, tcV1, tcU2, tcV2;

		//Used to get vectors (sides) from the position of the verts
		float vecX, vecY, vecZ;

		//Two edges of our triangle
		XMVECTOR edge1 = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
		XMVECTOR edge2 = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);

		//Compute face normals
		for ( int i = 0; i < meshTriangles; ++i ) {
			//Get the vector describing one edge of our triangle (edge 0,2)
			vecX = vertices[indices[(i * 3)]].pos.x - vertices[indices[(i * 3) + 2]].pos.x;
			vecY = vertices[indices[(i * 3)]].pos.y - vertices[indices[(i * 3) + 2]].pos.y;
			vecZ = vertices[indices[(i * 3)]].pos.z - vertices[indices[(i * 3) + 2]].pos.z;
			edge1 = XMVectorSet(vecX, vecY, vecZ, 0.0f);	//Create our first edge

															//Get the vector describing another edge of our triangle (edge 2,1)
			vecX = vertices[indices[(i * 3) + 2]].pos.x - vertices[indices[(i * 3) + 1]].pos.x;
			vecY = vertices[indices[(i * 3) + 2]].pos.y - vertices[indices[(i * 3) + 1]].pos.y;
			vecZ = vertices[indices[(i * 3) + 2]].pos.z - vertices[indices[(i * 3) + 1]].pos.z;
			edge2 = XMVectorSet(vecX, vecY, vecZ, 0.0f);	//Create our second edge

															//Cross multiply the two edge vectors to get the un-normalized face normal
			XMStoreFloat3(&unnormalized, XMVector3Cross(edge1, edge2));
			tempNormal.push_back(unnormalized);			//Save unormalized normal (for normal averaging)

														//Find first texture coordinate edge 2d vector
			tcU1 = vertices[indices[(i * 3)]].texCoord.x - vertices[indices[(i * 3) + 2]].texCoord.x;
			tcV1 = vertices[indices[(i * 3)]].texCoord.y - vertices[indices[(i * 3) + 2]].texCoord.y;

			//Find second texture coordinate edge 2d vector
			tcU2 = vertices[indices[(i * 3) + 2]].texCoord.x - vertices[indices[(i * 3) + 1]].texCoord.x;
			tcV2 = vertices[indices[(i * 3) + 2]].texCoord.y - vertices[indices[(i * 3) + 1]].texCoord.y;

			//Find tangent using both tex coord edges and position edges
			tangent.x = (tcV1 * XMVectorGetX(edge1) - tcV2 * XMVectorGetX(edge2)) * (1.0f / (tcU1 * tcV2 - tcU2 * tcV1));
			tangent.y = (tcV1 * XMVectorGetY(edge1) - tcV2 * XMVectorGetY(edge2)) * (1.0f / (tcU1 * tcV2 - tcU2 * tcV1));
			tangent.z = (tcV1 * XMVectorGetZ(edge1) - tcV2 * XMVectorGetZ(edge2)) * (1.0f / (tcU1 * tcV2 - tcU2 * tcV1));

			tempTangent.push_back(tangent);
		}

		//Compute vertex normals (normal Averaging)
		XMVECTOR normalSum = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
		XMVECTOR tangentSum = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
		int facesUsing = 0;
		float tX;
		float tY;
		float tZ;

		//Go through each vertex
		for ( int i = 0; i < totalVerts; ++i ) {
			//Check which triangles use this vertex
			for ( int j = 0; j < meshTriangles; ++j ) {
				if ( indices[j * 3] == i ||
					indices[(j * 3) + 1] == i ||
					indices[(j * 3) + 2] == i ) {
					tX = XMVectorGetX(normalSum) + tempNormal[j].x;
					tY = XMVectorGetY(normalSum) + tempNormal[j].y;
					tZ = XMVectorGetZ(normalSum) + tempNormal[j].z;

					normalSum = XMVectorSet(tX, tY, tZ, 0.0f);	//If a face is using the vertex, add the unormalized face normal to the normalSum

																//We can reuse tX, tY, tZ to sum up tangents
					tX = XMVectorGetX(tangentSum) + tempTangent[j].x;
					tY = XMVectorGetY(tangentSum) + tempTangent[j].y;
					tZ = XMVectorGetZ(tangentSum) + tempTangent[j].z;

					tangentSum = XMVectorSet(tX, tY, tZ, 0.0f); //sum up face tangents using this vertex
					facesUsing++;
				}
			}

			//Get the actual normal by dividing the normalSum by the number of faces sharing the vertex
			normalSum = normalSum / (float)facesUsing;
			tangentSum = tangentSum / (float)facesUsing;

			//Normalize the normalSum vector
			normalSum = XMVector3Normalize(normalSum);
			tangentSum = XMVector3Normalize(tangentSum);

			//Store the normal in our current vertex
			vertices[i].normal.x = XMVectorGetX(normalSum);
			vertices[i].normal.y = XMVectorGetY(normalSum);
			vertices[i].normal.z = XMVectorGetZ(normalSum);

			vertices[i].tangent.x = XMVectorGetX(tangentSum);
			vertices[i].tangent.y = XMVectorGetY(tangentSum);
			vertices[i].tangent.z = XMVectorGetZ(tangentSum);

			//Clear normalSum and facesUsing for next vertex
			normalSum = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
			tangentSum = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
			facesUsing = 0;

		}
	}

	//Create index buffer
	D3D11_BUFFER_DESC indexBufferDesc;
	ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));

	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(DWORD) * meshTriangles * 3;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA iinitData;

	iinitData.pSysMem = &indices[0];
	HR(m_d3dDevice->CreateBuffer(&indexBufferDesc, &iinitData, _indexBuff));

	//Create Vertex Buffer
	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));

	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(Vertex3D) * totalVerts;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA vertexBufferData;

	ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
	vertexBufferData.pSysMem = &vertices[0];
	HR(m_d3dDevice->CreateBuffer(&vertexBufferDesc, &vertexBufferData, _vertBuff));

	return true;
}
