#include "WaveMesh.h"

WaveMesh::~WaveMesh() {
	SafeRelease(inputLayout);
	SafeRelease(indexBuffer);
	SafeRelease(vertBuffer);
	SafeRelease(constBuffer);
	SafeRelease(vertexShader);
	SafeRelease(pixelShader);
	SafeRelease(shaderResView);
	SafeRelease(normalShaderResView);
	SafeRelease(rasterState);
	SafeRelease(texSamplerState);
}

void WaveMesh::Init(ID3D11Device * _d3dDevice, LPCWSTR _shaderFilename) {
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
	cbBuffer.material.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	cbBuffer.material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	cbBuffer.material.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 32.0f);

	// init the wave
	waves.Init(100, 100, 1.0f, 0.03f, 3.25f, 0.4f);

	// loading the texture - using dds loader
	HR(CreateDDSTextureFromFile(_d3dDevice, L"Resources/Textures/waveWater.dds", NULL, &shaderResView));
	HR(CreateDDSTextureFromFile(_d3dDevice, L"Resources/Textures/waveWater_normal.dds", NULL, &normalShaderResView));

	// create the depending shader
	HR(D3DUtils::CreateShaderAndLayoutFromFile(_d3dDevice, _shaderFilename, vertexLayout, 4, &vertexShader, &pixelShader, &inputLayout));

	BuildBuffer(_d3dDevice);
}

void WaveMesh::BuildBuffer(ID3D11Device * _d3dDevice) {
	// Create the vertex buffer.  Note that we allocate space only, as
	// we will be updating the data every time step of the simulation.

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_DYNAMIC;
	vbd.ByteWidth = sizeof(Vertex3D) * waves.VertexCount();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vbd.MiscFlags = 0;
	HR(_d3dDevice->CreateBuffer(&vbd, 0, &vertBuffer));

	// Create the index buffer.  The index buffer is fixed, so we only 
	// need to create and set once.

	std::vector<UINT> indices(3 * waves.TriangleCount()); // 3 indices per face

	// Iterate over each quad.
	UINT m = waves.RowCount();
	UINT n = waves.ColumnCount();
	int k = 0;
	for (UINT i = 0; i < m - 1; ++i) {
		for (DWORD j = 0; j < n - 1; ++j) {
			indices[k] = i*n + j;
			indices[k + 1] = i*n + j + 1;
			indices[k + 2] = (i + 1)*n + j;

			indices[k + 3] = (i + 1)*n + j;
			indices[k + 4] = i*n + j + 1;
			indices[k + 5] = (i + 1)*n + j + 1;

			k += 6; // next quad
		}
	}

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * static_cast<UINT>(indices.size());
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
	HR(_d3dDevice->CreateBuffer(&ibd, &iinitData, &indexBuffer));

}

void WaveMesh::Update(double _dt, double _tt, ID3D11DeviceContext * _d3dImmediateContext) {
	// waves verteces update
	static float t_base = 0.0f;
	if ((_tt - t_base) >= 0.25f) {
		t_base += 0.25f;

		DWORD i = 5 + rand() % (waves.RowCount() - 10);
		DWORD j = 5 + rand() % (waves.ColumnCount() - 10);

		float r = D3DUtils::RandFloat(1.0f, 2.0f);

		waves.Disturb(i, j, r);
	}

	waves.Update((float)_dt);

	//Set the updated waves to the vertex buffer

	D3D11_MAPPED_SUBRESOURCE mappedData;
	HR(_d3dImmediateContext->Map(vertBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));

	Vertex3D* v = reinterpret_cast<Vertex3D*>(mappedData.pData);
	for (UINT i = 0; i < waves.VertexCount(); ++i) {
		v[i].Position = waves[i];
		v[i].Normal = waves.Normal(i);
		v[i].TangentU = waves.TangentX(i);

		// Derive tex-coords in [0,1] from position.
		v[i].TexCoord.x = 0.5f + waves[i].x / waves.Width();
		v[i].TexCoord.y = 0.5f - waves[i].z / waves.Depth();
	}

	_d3dImmediateContext->Unmap(vertBuffer, 0);

	// Animate water texture coordinates.

	// Tile water texture.
	XMMATRIX wavesScale = XMMatrixScaling(5.0f, 5.0f, 0.0f);

	// Translate texture over time.
	waterTexOffset.y += 0.05f*static_cast<float>(_dt);
	waterTexOffset.x += 0.1f*static_cast<float>(_dt);
	XMMATRIX wavesOffset = XMMatrixTranslation(waterTexOffset.x, waterTexOffset.y, 0.0f);

	// Combine scale and translation.
	waterTexTransform = wavesScale*wavesOffset;

}

void WaveMesh::Render(mutex * _mutex, ID3D11DeviceContext * _d3dImmediateContext, XMMATRIX _camView, XMMATRIX _camProj) {

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
	cbBuffer.WorldViewProj = XMMatrixTranspose(worldMat * (_camView)* (_camProj));
	cbBuffer.TexTransform = waterTexTransform;

	_d3dImmediateContext->UpdateSubresource(constBuffer, 0, NULL, &cbBuffer, 0, 0);
	_d3dImmediateContext->VSSetConstantBuffers(0, 1, &constBuffer);
	_d3dImmediateContext->PSSetConstantBuffers(1, 1, &constBuffer);
	_d3dImmediateContext->PSSetShaderResources(0, 1, &shaderResView);
	_d3dImmediateContext->PSSetShaderResources(1, 1, &normalShaderResView);
	_d3dImmediateContext->PSSetSamplers(0, 1, &texSamplerState);
	_d3dImmediateContext->RSSetState(rasterState);
	_d3dImmediateContext->DrawIndexed(3 * waves.TriangleCount(), 0, 0);

}
