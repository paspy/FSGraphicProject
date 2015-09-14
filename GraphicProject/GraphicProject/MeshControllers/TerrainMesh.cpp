#include "TerrainMesh.h"

TerrainMesh::~TerrainMesh() {
	SafeRelease(inputLayout);
	SafeRelease(indexBuffer);
	SafeRelease(vertBuffer);
	SafeRelease(constBuffer);
	SafeRelease(vertexShader);
	SafeRelease(pixelShader);
	SafeRelease(shaderResView);
	SafeRelease(normalShaderResView);
	SafeRelease(texSamplerState);
}

void TerrainMesh::Init(ID3D11Device * _d3dDevice, LPCWSTR _shaderFilename) {
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
	cbBuffer.material.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 32.0f);

	// scaling texture
	terrainTexTransform = XMMatrixScaling(10.0f, 10.0f, 0.0f);

	// loading the texture - using dds loader
	HR(CreateDDSTextureFromFile(_d3dDevice, L"Resources/Models/ground_diffuse.dds", NULL, &shaderResView));
	HR(CreateDDSTextureFromFile(_d3dDevice, L"Resources/Models/ground_normal.dds", NULL, &normalShaderResView));

	// create the depending shader
	HR(D3DUtils::CreateShaderAndLayoutFromFile(_d3dDevice, _shaderFilename, vertexLayout, 4, &vertexShader, &pixelShader, &inputLayout));

	BuildBuffer(_d3dDevice);
}

void TerrainMesh::BuildBuffer(ID3D11Device * _d3dDevice) {
	GeoGen::MeshData grid;

	GeoGen::CreateGrid(250.0f, 250.0f, 100, 100, grid);

	indicesCount = static_cast<UINT>(grid.Indices.size());

	vector<Vertex3D> vertices(grid.Vertices.size());
	for (size_t i = 0; i < grid.Vertices.size(); ++i) {
		XMFLOAT3 p = grid.Vertices[i].Position;

		p.y = GeoGen::GetHillHeight(p.x, p.z);

		vertices[i].Position = p;
		vertices[i].Normal = GeoGen::GetHillNormal(p.x, p.z);
		vertices[i].TexCoord = grid.Vertices[i].TexCoord;
		vertices[i].TangentU = grid.Vertices[i].TangentU;
	}

	D3D11_BUFFER_DESC vbd;
	ZeroMemory(&vbd, sizeof(vbd));
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex3D) * static_cast<UINT>(grid.Vertices.size());
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(_d3dDevice->CreateBuffer(&vbd, &vinitData, &vertBuffer));

	D3D11_BUFFER_DESC ibd;
	ZeroMemory(&ibd, sizeof(ibd));
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * indicesCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &grid.Indices[0];
	HR(_d3dDevice->CreateBuffer(&ibd, &iinitData, &indexBuffer));
}

void TerrainMesh::Render(ID3D11DeviceContext * _d3dImmediateContext, XMMATRIX _camView, XMMATRIX _camProj, ID3D11RasterizerState *_rasterState) {
	// Set the default VS shader and depth/stencil state and layout
	_d3dImmediateContext->VSSetShader(vertexShader, NULL, 0);
	_d3dImmediateContext->PSSetShader(pixelShader, NULL, 0);
	_d3dImmediateContext->IASetInputLayout(inputLayout);
	_d3dImmediateContext->OMSetDepthStencilState(NULL, 0);

	//Set the index buffer
	_d3dImmediateContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	//Set the vertex buffer
	_d3dImmediateContext->IASetVertexBuffers(0, 1, &vertBuffer, &stride, &offset);

	cbBuffer.World = XMMatrixTranspose(worldMat);
	cbBuffer.WorldInvTranspose = D3DUtils::InverseTranspose(worldMat);
	cbBuffer.WorldViewProj = XMMatrixTranspose(worldMat * _camView* _camProj);
	cbBuffer.TexTransform = terrainTexTransform;

	_d3dImmediateContext->UpdateSubresource(constBuffer, 0, NULL, &cbBuffer, 0, 0);
	_d3dImmediateContext->VSSetConstantBuffers(0, 1, &constBuffer);
	_d3dImmediateContext->PSSetConstantBuffers(1, 1, &constBuffer);
	_d3dImmediateContext->PSSetShaderResources(0, 1, &shaderResView);
	_d3dImmediateContext->PSSetShaderResources(1, 1, &normalShaderResView);
	_d3dImmediateContext->PSSetSamplers(0, 1, &texSamplerState);
	_d3dImmediateContext->RSSetState(_rasterState);
	_d3dImmediateContext->DrawIndexed(indicesCount, 0, 0);
}


