#include "MirrorMesh.h"
#include "../D3DApp/RenderStates.h"

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
	mtWallAndFloor.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mtWallAndFloor.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mtWallAndFloor.Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);

	mtObject.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mtObject.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mtObject.Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);

	mtMirror.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mtMirror.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f);
	mtMirror.Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);

	mtShadow.Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mtShadow.Diffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.5f);
	mtShadow.Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 16.0f);

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

	// Floor
	mirrorVert[0] =  Vertex3D(-3.5f, 0.0f, -10.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 4.0f);
	mirrorVert[1] =  Vertex3D(-3.5f, 0.0f,   0.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	mirrorVert[2] =  Vertex3D( 7.5f, 0.0f,   0.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 4.0f, 0.0f);
					  												
	mirrorVert[3] =  Vertex3D(-3.5f, 0.0f, -10.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 4.0f);
	mirrorVert[4] =  Vertex3D( 7.5f, 0.0f,   0.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 4.0f, 0.0f);
	mirrorVert[5] =  Vertex3D( 7.5f, 0.0f, -10.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 4.0f, 4.0f);

	// Wall
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

	GeoGen::MeshData geo;
	vector<Vertex3D> vertices;
	GeoGen::CreateSphere(0.5f, 20, 20, geo);

	vertices.resize(geo.Vertices.size());
	for ( size_t i = 0; i < geo.Vertices.size(); ++i ) {
		vertices[i].Position = geo.Vertices[i].Position;
		vertices[i].Normal = geo.Vertices[i].Normal;
		vertices[i].TexCoord = geo.Vertices[i].TexCoord;
		vertices[i].TangentU = geo.Vertices[i].TangentU;
	}

	indicesCount = static_cast<UINT>(geo.Indices.size());

	D3D11_BUFFER_DESC vbd;
	ZeroMemory(&vbd, sizeof(vbd));
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex3D) * static_cast<UINT>(geo.Vertices.size());
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(_d3dDevice->CreateBuffer(&vbd, &vinitData, &vertBuffer[1]));

	D3D11_BUFFER_DESC ibd;
	ZeroMemory(&ibd, sizeof(ibd));
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * indicesCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &geo.Indices[0];
	HR(_d3dDevice->CreateBuffer(&ibd, &iinitData, &indexBuffer));
}

void MirrorMesh::Update() {
	XMMATRIX Rotate = XMMatrixRotationY(0.5f*XM_PI);
	XMMATRIX Scale = XMMatrixScaling(0.45f, 0.45f, 0.45f);
	XMMATRIX Offset = XMMatrixTranslation(0.0f, 1.0f, -5.0f);
	worldMat = Rotate*Scale*Offset;
}

void MirrorMesh::Render(ID3D11DeviceContext * _d3dImmediateContext, XMMATRIX _camView, XMMATRIX _camProj, ID3D11RasterizerState *_rs) {
	// Set the default VS shader and depth/stencil state and layout
	_d3dImmediateContext->VSSetShader(vertexShader, NULL, 0);
	_d3dImmediateContext->PSSetShader(pixelShader, NULL, 0);
	_d3dImmediateContext->IASetInputLayout(inputLayout);
	_d3dImmediateContext->OMSetDepthStencilState(NULL, 0);

	float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };

// draw the normal objects
	//Set the vertex buffer
	_d3dImmediateContext->IASetVertexBuffers(0, 1, &vertBuffer[0], &stride, &offset);

	cbObject.World = XMMatrixTranspose(wallFloorMat);
	cbObject.WorldInvTranspose = D3DUtils::InverseTranspose(wallFloorMat);
	cbObject.WorldViewProj = XMMatrixTranspose(wallFloorMat * _camView* _camProj);
	cbObject.material = mtWallAndFloor;
	cbObject.TexTransform = XMMatrixIdentity();

	_d3dImmediateContext->UpdateSubresource(constBuffer, 0, NULL, &cbObject, 0, 0);
	_d3dImmediateContext->VSSetConstantBuffers(0, 1, &constBuffer);
	_d3dImmediateContext->PSSetConstantBuffers(1, 1, &constBuffer);
	_d3dImmediateContext->PSSetShaderResources(0, 1, &shaderResViews[0]);
	_d3dImmediateContext->PSSetShaderResources(1, 1, &normalShaderResViews[0]);
	_d3dImmediateContext->PSSetSamplers(0, 1, &texSamplerState);
	_d3dImmediateContext->RSSetState(_rs);
	// floor
	_d3dImmediateContext->Draw(6, 0);

	_d3dImmediateContext->PSSetShaderResources(0, 1, &shaderResViews[1]);
	_d3dImmediateContext->PSSetShaderResources(1, 1, &normalShaderResViews[1]);
	// wall
	_d3dImmediateContext->Draw(18, 6);

// draw the normal cube

	//Set the vertex buffer
	_d3dImmediateContext->IASetVertexBuffers(0, 1, &vertBuffer[1], &stride, &offset);
	//Set the index buffer
	_d3dImmediateContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	cbObject.World = XMMatrixTranspose(worldMat);
	cbObject.WorldInvTranspose = D3DUtils::InverseTranspose(worldMat);
	cbObject.WorldViewProj = XMMatrixTranspose(worldMat * _camView* _camProj);
	cbObject.material = mtObject;
	cbObject.TexTransform = XMMatrixIdentity();

	_d3dImmediateContext->UpdateSubresource(constBuffer, 0, NULL, &cbObject, 0, 0);
	_d3dImmediateContext->VSSetConstantBuffers(0, 1, &constBuffer);
	_d3dImmediateContext->PSSetConstantBuffers(1, 1, &constBuffer);
	_d3dImmediateContext->PSSetShaderResources(0, 1, &shaderResViews[1]);
	_d3dImmediateContext->PSSetShaderResources(1, 1, &normalShaderResViews[1]);
	_d3dImmediateContext->PSSetSamplers(0, 1, &texSamplerState);
	_d3dImmediateContext->RSSetState(_rs);

	_d3dImmediateContext->DrawIndexed(indicesCount, 0, 0);

// draw the objects to the stencil buffer only

//Set the vertex buffer
	_d3dImmediateContext->IASetVertexBuffers(0, 1, &vertBuffer[0], &stride, &offset);

	cbObject.World = XMMatrixTranspose(wallFloorMat);
	cbObject.WorldInvTranspose = D3DUtils::InverseTranspose(wallFloorMat);
	cbObject.WorldViewProj = XMMatrixTranspose(wallFloorMat * _camView* _camProj);
	cbObject.material = mtWallAndFloor;
	cbObject.TexTransform = XMMatrixIdentity();

	_d3dImmediateContext->OMSetBlendState(RenderStates::NoRenderTargetWritesBS, blendFactor, 0xffffffff);
	_d3dImmediateContext->OMSetDepthStencilState(RenderStates::MarkMirrorDSS, 1);

	_d3dImmediateContext->UpdateSubresource(constBuffer, 0, NULL, &cbObject, 0, 0);
	_d3dImmediateContext->VSSetConstantBuffers(0, 1, &constBuffer);
	_d3dImmediateContext->PSSetConstantBuffers(1, 1, &constBuffer);
	_d3dImmediateContext->PSSetShaderResources(0, 1, &shaderResViews[3]);
	_d3dImmediateContext->PSSetShaderResources(1, 1, &normalShaderResViews[3]);
	_d3dImmediateContext->PSSetSamplers(0, 1, &texSamplerState);
	_d3dImmediateContext->RSSetState(_rs);
	// mirror
	_d3dImmediateContext->Draw(6, 24);

	// Restore states.
	_d3dImmediateContext->OMSetDepthStencilState(0, 0);
	_d3dImmediateContext->OMSetBlendState(0, blendFactor, 0xffffffff);


}


