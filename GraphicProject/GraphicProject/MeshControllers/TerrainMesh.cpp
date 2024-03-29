#include "TerrainMesh.h"
#include "../D3DApp/Camera.h"

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
	cbBuffer.material.Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 8.0f);

	// scaling texture
	terrainTexTransform = XMMatrixScaling(100.0f, 100.0f, 0.0f);

	// loading the texture - using dds loader
	HR(CreateDDSTextureFromFile(_d3dDevice, L"Resources/Textures/Grass_diffuse.dds", NULL, &shaderResView));
	HR(CreateDDSTextureFromFile(_d3dDevice, L"Resources/Textures/Grass_normal.dds", NULL, &normalShaderResView));

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

void TerrainMesh::Render(ID3D11DeviceContext * _context, const Camera &_camera, ID3D11RasterizerState *_rs) {
	// Set the default VS shader and depth/stencil state and layout
	_context->VSSetShader(vertexShader, NULL, 0);
	_context->PSSetShader(pixelShader, NULL, 0);
	_context->IASetInputLayout(inputLayout);
	_context->OMSetDepthStencilState(NULL, 0);

	//Set the index buffer
	_context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	//Set the vertex buffer
	_context->IASetVertexBuffers(0, 1, &vertBuffer, &stride, &offset);

	cbBuffer.World = XMMatrixTranspose(worldMat);
	cbBuffer.WorldInvTranspose = D3DUtils::InverseTranspose(worldMat);
	cbBuffer.WorldViewProj = XMMatrixTranspose(worldMat *_camera.GetViewProj());
	cbBuffer.TexTransform = terrainTexTransform;

	_context->UpdateSubresource(constBuffer, 0, NULL, &cbBuffer, 0, 0);
	_context->VSSetConstantBuffers(0, 1, &constBuffer);
	_context->PSSetConstantBuffers(1, 1, &constBuffer);
	_context->PSSetShaderResources(0, 1, &shaderResView);
	_context->PSSetShaderResources(1, 1, &normalShaderResView);
	_context->PSSetSamplers(0, 1, &texSamplerState);
	_context->RSSetState(_rs);
	_context->DrawIndexed(indicesCount, 0, 0);
}


