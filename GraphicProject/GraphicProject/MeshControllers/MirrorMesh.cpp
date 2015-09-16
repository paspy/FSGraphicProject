#include "MirrorMesh.h"

MirrorMesh::~MirrorMesh() {
	SafeRelease(inputLayout);
	SafeRelease(indexBuffer);
	SafeRelease(vertBuffer[0]);
	SafeRelease(vertBuffer[1]);
	SafeRelease(constBuffer);
	SafeRelease(vertexShader);
	SafeRelease(pixelShader);
	for (int i = 0; i < 3; i++) {
		SafeRelease(shaderResViews[i]);
		SafeRelease(normalShaderResViews[i]);
	}
	SafeRelease(texSamplerState);
}

void MirrorMesh::Init(ID3D11Device * _d3dDevice, LPCWSTR _shaderFilename) {
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
	_d3dDevice->CreateSamplerState(&sampDesc, &texSamplerState);

	// hard coded matrtial setting - NOT GOOD
	cbBuffer.material.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	cbBuffer.material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	cbBuffer.material.Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 8.0f);

	// scaling texture
	terrainTexTransform = XMMatrixScaling(100.0f, 100.0f, 0.0f);

	// loading the texture - using dds loader
	HR(CreateDDSTextureFromFile(_d3dDevice, L"Resources/Textures/Checkboard_diffuse.dds", NULL, &shaderResViews[0]));
	HR(CreateDDSTextureFromFile(_d3dDevice, L"Resources/Textures/Brick_diffuse.dds", NULL, &shaderResViews[1]));
	HR(CreateDDSTextureFromFile(_d3dDevice, L"Resources/Textures/Ice_diffuse.dds", NULL, &shaderResViews[2]));

	HR(CreateDDSTextureFromFile(_d3dDevice, L"Resources/Textures/Checkboard_normal.dds", NULL, &normalShaderResViews[0]));
	HR(CreateDDSTextureFromFile(_d3dDevice, L"Resources/Textures/Brick_normal.dds", NULL, &normalShaderResViews[1]));
	HR(CreateDDSTextureFromFile(_d3dDevice, L"Resources/Textures/Ice_normal.dds", NULL, &normalShaderResViews[2]));

	// create the depending shader
	HR(D3DUtils::CreateShaderAndLayoutFromFile(_d3dDevice, _shaderFilename, vertexLayout, 4, &vertexShader, &pixelShader, &inputLayout));

	BuildBuffer(_d3dDevice);
}

void MirrorMesh::BuildBuffer(ID3D11Device * _d3dDevice) {

	Vertex3D mirrorVert[30];

	// Floor: Observe we tile texture coordinates.
	mirrorVert[0] =  Vertex3D(-3.5f, 0.0f, -10.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 4.0f);
	mirrorVert[1] =  Vertex3D(-3.5f, 0.0f,   0.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	mirrorVert[2] =  Vertex3D( 7.5f, 0.0f,   0.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 4.0f, 0.0f);
					  												
	mirrorVert[3] =  Vertex3D(-3.5f, 0.0f, -10.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 4.0f);
	mirrorVert[4] =  Vertex3D( 7.5f, 0.0f,   0.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 4.0f, 0.0f);
	mirrorVert[5] =  Vertex3D( 7.5f, 0.0f, -10.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 4.0f, 4.0f);

	// Wall: Observe we tile texture coordinates, and that we
	// leave a gap in the middle for the mirror.
	mirrorVert[6] =  Vertex3D(-3.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 2.0f);
	mirrorVert[7] = Vertex3D(-3.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	mirrorVert[8] = Vertex3D(-2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.5f, 0.0f);

	mirrorVert[9] =  Vertex3D(-3.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 2.0f);
	mirrorVert[10] = Vertex3D(-2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.5f, 0.0f);
	mirrorVert[11] = Vertex3D(-2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.5f, 2.0f);

	mirrorVert[12] = Vertex3D(2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 2.0f);
	mirrorVert[13] = Vertex3D(2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	mirrorVert[14] = Vertex3D(7.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 2.0f, 0.0f);

	mirrorVert[15] = Vertex3D(2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 2.0f);
	mirrorVert[16] = Vertex3D(7.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 2.0f, 0.0f);
	mirrorVert[17] = Vertex3D(7.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 2.0f, 2.0f);

	mirrorVert[18] = Vertex3D(-3.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	mirrorVert[19] = Vertex3D(-3.5f, 6.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	mirrorVert[20] = Vertex3D(7.5f, 6.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 6.0f, 0.0f);

	mirrorVert[21] = Vertex3D(-3.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	mirrorVert[22] = Vertex3D(7.5f, 6.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 6.0f, 0.0f);
	mirrorVert[23] = Vertex3D(7.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 6.0f, 1.0f);

	// Mirror
	mirrorVert[24] = Vertex3D(-2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	mirrorVert[25] = Vertex3D(-2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	mirrorVert[26] = Vertex3D(2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);

	mirrorVert[27] = Vertex3D(-2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	mirrorVert[28] = Vertex3D(2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	mirrorVert[29] = Vertex3D(2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);


	D3D11_BUFFER_DESC vbd;
	ZeroMemory(&vbd, sizeof(vbd));
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex3D) * 30;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = mirrorVert;
	HR(_d3dDevice->CreateBuffer(&vbd, &vinitData, &vertBuffer[0]));

}

void MirrorMesh::Render(ID3D11DeviceContext * _d3dImmediateContext, XMMATRIX _camView, XMMATRIX _camProj, ID3D11RasterizerState *_rs) {
	// Set the default VS shader and depth/stencil state and layout
	_d3dImmediateContext->VSSetShader(vertexShader, NULL, 0);
	_d3dImmediateContext->PSSetShader(pixelShader, NULL, 0);
	_d3dImmediateContext->IASetInputLayout(inputLayout);
	_d3dImmediateContext->OMSetDepthStencilState(NULL, 0);

	//Set the index buffer
	_d3dImmediateContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	//Set the vertex buffer
	_d3dImmediateContext->IASetVertexBuffers(0, 1, &vertBuffer[0], &stride, &offset);

	cbBuffer.World = XMMatrixTranspose(worldMat);
	cbBuffer.WorldInvTranspose = D3DUtils::InverseTranspose(worldMat);
	cbBuffer.WorldViewProj = XMMatrixTranspose(worldMat * _camView* _camProj);
	cbBuffer.TexTransform = terrainTexTransform;

	_d3dImmediateContext->UpdateSubresource(constBuffer, 0, NULL, &cbBuffer, 0, 0);
	_d3dImmediateContext->VSSetConstantBuffers(0, 1, &constBuffer);
	_d3dImmediateContext->PSSetConstantBuffers(1, 1, &constBuffer);
	//_d3dImmediateContext->PSSetShaderResources(0, 1, &shaderResView);
	//_d3dImmediateContext->PSSetShaderResources(1, 1, &normalShaderResView);
	_d3dImmediateContext->PSSetSamplers(0, 1, &texSamplerState);
	_d3dImmediateContext->RSSetState(_rs);
	_d3dImmediateContext->DrawIndexed(indicesCount, 0, 0);
}


