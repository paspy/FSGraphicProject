#include "Skybox.h"
#include "../D3DApp/Camera.h"

void LoadThread(Skybox *_skybox, ID3D11Device * _d3dDevice) {
	if ( _skybox ) {
		_skybox->LoadStuff(_d3dDevice);
	}
}

Skybox::~Skybox() {
	SafeRelease(inputLayout);
	SafeRelease(indexBuffer);
	SafeRelease(vertBuffer);
	SafeRelease(constBuffer);
	SafeRelease(vertexShader);
	SafeRelease(pixelShader);
	SafeRelease(shaderResView);
	SafeRelease(DSLessEqual);
	SafeRelease(texSamplerState);
}

void Skybox::Init(ID3D11Device * _d3dDevice) {
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

	D3D11_DEPTH_STENCIL_DESC dssDesc;
	ZeroMemory(&dssDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	dssDesc.DepthEnable = true;
	dssDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dssDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

	HR(_d3dDevice->CreateDepthStencilState(&dssDesc, &DSLessEqual));

	// build the "skysphere"
	BuildSphere(_d3dDevice, 10, 10, &vertBuffer, &indexBuffer, numVertices, numFaces);

	thread loadThread = thread(LoadThread, this, _d3dDevice);
	loadThread.join();
}

void Skybox::LoadStuff(ID3D11Device * _d3dDevice) {
	// loading the texture - using dds loader
	HR(CreateDDSTextureFromFile(_d3dDevice, L"Resources/Skybox/snow.dds", NULL, &shaderResView));

	// create the depending shader
	HR(D3DUtils::CreateShaderAndLayoutFromFile(_d3dDevice, L"Shaders/Skybox/Skybox.hlsl", vertexLayout, 2, &vertexShader, &pixelShader, &inputLayout));

}

void Skybox::Render(ID3D11DeviceContext * _context, const Camera& _camera, ID3D11RasterizerState *_rs) {
	//Set the proper VS and PS shaders, and layout
	_context->VSSetShader(vertexShader, 0, 0);
	_context->PSSetShader(pixelShader, 0, 0);
	_context->IASetInputLayout(inputLayout);
	//Set the spheres index buffer
	_context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	//Set the spheres vertex buffer
	_context->IASetVertexBuffers(0, 1, &vertBuffer, &stride, &offset);
	//Set the WorldViewProj matrix and send it to the constant buffer in shader file
	cBuffer.WorldViewProj = XMMatrixTranspose(worldMat * _camera.GetViewProj());

	_context->UpdateSubresource(constBuffer, 0, NULL, &cBuffer, 0, 0);
	_context->VSSetConstantBuffers(0, 1, &constBuffer);
	//Send our skymap resource view to pixel shader
	_context->PSSetShaderResources(0, 1, &shaderResView);
	_context->PSSetSamplers(0, 1, &texSamplerState);
	//Set the new depth/stencil and RS states
	_context->OMSetDepthStencilState(DSLessEqual, 0);
	_context->RSSetState(_rs);
	_context->DrawIndexed(numFaces * 3, 0, 0);
}

void Skybox::BuildSphere(
	ID3D11Device *_d3dDevice,
	int _latLines,
	int _longLines,
	ID3D11Buffer ** _vertBuffer,
	ID3D11Buffer ** _indexBuffer,
	int &_numSphereVertices,
	int &_numSphereFaces) {

	_numSphereVertices = ((_latLines - 2) * _longLines) + 2;
	_numSphereFaces = ((_latLines - 3)*(_longLines)* 2) + (_longLines * 2);

	float sphereYaw = 0.0f;
	float spherePitch = 0.0f;
	XMMATRIX RotationX;
	XMMATRIX RotationY;
	XMMATRIX RotationZ;

	vector<Vertex3D> vertices(_numSphereVertices);

	XMVECTOR currVertPos = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

	vertices[0].Position.x = 0.0f;
	vertices[0].Position.y = 0.0f;
	vertices[0].Position.z = 1.0f;

	for ( int i = 0; i < _latLines - 2; i++ ) {
		spherePitch = (i + 1) * (3.14f / (_latLines - 1));
		RotationX = XMMatrixRotationX(spherePitch);
		for ( int j = 0; j < _longLines; j++ ) {
			sphereYaw = j * (6.28f / (_longLines));
			RotationY = XMMatrixRotationZ(sphereYaw);
			currVertPos = XMVector3TransformNormal(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), (RotationX * RotationY));
			currVertPos = XMVector3Normalize(currVertPos);
			vertices[i*_longLines + j + 1].Position.x = XMVectorGetX(currVertPos);
			vertices[i*_longLines + j + 1].Position.y = XMVectorGetY(currVertPos);
			vertices[i*_longLines + j + 1].Position.z = XMVectorGetZ(currVertPos);
		}
	}

	vertices[_numSphereVertices - 1].Position.x = 0.0f;
	vertices[_numSphereVertices - 1].Position.y = 0.0f;
	vertices[_numSphereVertices - 1].Position.z = -1.0f;

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
	HR(_d3dDevice->CreateBuffer(&vertexBufferDesc, &vertexBufferData, _vertBuffer));

	vector<DWORD> indices(_numSphereFaces * 3);

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
	indexBufferDesc.ByteWidth = sizeof(DWORD) * _numSphereFaces * 3;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA iinitData;

	iinitData.pSysMem = &indices[0];
	HR(_d3dDevice->CreateBuffer(&indexBufferDesc, &iinitData, _indexBuffer));
}
