#include "GeoMesh.h"

GeoMesh::~GeoMesh() {
	SafeRelease(inputLayout);
	SafeRelease(indexBuffer);
	SafeRelease(vertBuffer);
	SafeRelease(instancedBuffer);
	SafeRelease(constBuffer);
	SafeRelease(vertexShader);
	SafeRelease(pixelShader);
	SafeRelease(shaderResView);
	SafeRelease(normalShaderResView);
	SafeRelease(texSamplerState);
}

void GeoMesh::Init(ID3D11Device * _d3dDevice, LPCWSTR _shaderFilename, GeoType _genType, LPCWSTR _diffuseFile, LPCWSTR _normalFile) {
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
	cbBuffer.material.Ambient = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	cbBuffer.material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	cbBuffer.material.Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 8.0f);

	// scaling texture
	geoTexTransform = XMMatrixScaling(1.0f, 1.0f, 0.0f);

	// loading the texture - using dds loader
	HR(CreateDDSTextureFromFile(_d3dDevice, _diffuseFile, NULL, &shaderResView));
	HR(CreateDDSTextureFromFile(_d3dDevice, _normalFile, NULL, &normalShaderResView));

	// create the depending shader
	HR(D3DUtils::CreateShaderAndLayoutFromFile(_d3dDevice, _shaderFilename, vertexLayout, 9, &vertexShader, &pixelShader, &inputLayout));

	BuildBuffer(_d3dDevice, _genType);
	BuildInstancedBuffer(_d3dDevice);
}

void GeoMesh::BuildBuffer(ID3D11Device * _d3dDevice, GeoType _genType) {
	GeoGen::MeshData geo;
	vector<Vertex3D> vertices;

	switch (_genType) {
	case GeoType::Box: {
		GeoGen::CreateBox(1.0f, 1.0f, 1.0f, geo);
		break;
	}
	case GeoType::Cylinder: {
		GeoGen::CreateCylinder(0.5f, 0.5f, 3.0f, 15, 15, geo);
		break;
	}
	case GeoType::Grid: {
		GeoGen::CreateGrid(20.0f, 30.0f, 50, 40, geo);
		break;
	}
	case GeoType::Sphere: {
		GeoGen::CreateSphere(0.5f, 20, 20, geo);
		break;
	}
	default:
		return;
		break;
	}

	vertices.resize(geo.Vertices.size());
	for (size_t i = 0; i < geo.Vertices.size(); ++i) {
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
	HR(_d3dDevice->CreateBuffer(&vbd, &vinitData, &vertBuffer));

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

void GeoMesh::BuildInstancedBuffer(ID3D11Device * _d3dDevice) {
	const int n = 5;
	instancedData.resize(n*n*n);

	float width = 200.0f;
	float height = 200.0f;
	float depth = 200.0f;

	float x = -0.5f*width;
	float y = -0.5f*height;
	float z = -0.5f*depth;
	float dx = width / (n - 1);
	float dy = height / (n - 1);
	float dz = depth / (n - 1);
	for (int k = 0; k < n; ++k) {
		for (int i = 0; i < n; ++i) {
			for (int j = 0; j < n; ++j) {
				// Position instanced along a 3D grid.
				instancedData[k*n*n + i*n + j].World = XMFLOAT4X4(
					1.0f, 0.0f, 0.0f, 0.0f,
					0.0f, 1.0f, 0.0f, 0.0f,
					0.0f, 0.0f, 1.0f, 0.0f,
					x + j*dx, y + i*dy, z + k*dz, 1.0f);

				// Random color.
				instancedData[k*n*n + i*n + j].Color.x = D3DUtils::RandFloat(0.0f, 1.0f);
				instancedData[k*n*n + i*n + j].Color.y = D3DUtils::RandFloat(0.0f, 1.0f);
				instancedData[k*n*n + i*n + j].Color.z = D3DUtils::RandFloat(0.0f, 1.0f);
				instancedData[k*n*n + i*n + j].Color.w = 1.0f;
			}
		}
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_DYNAMIC;
	vbd.ByteWidth = sizeof(InstancedData) * (UINT)instancedData.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	HR(_d3dDevice->CreateBuffer(&vbd, 0, &instancedBuffer));
}

void GeoMesh::Render(ID3D11DeviceContext * _d3dImmediateContext, XMMATRIX _camView, XMMATRIX _camProj, ID3D11RasterizerState *_rs) {
	// Set the default VS shader and depth/stencil state and layout
	_d3dImmediateContext->VSSetShader(vertexShader, NULL, 0);
	_d3dImmediateContext->PSSetShader(pixelShader, NULL, 0);
	_d3dImmediateContext->IASetInputLayout(inputLayout);
	_d3dImmediateContext->OMSetDepthStencilState(NULL, 0);

	//Set the index buffer
	_d3dImmediateContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	
	//Set the vertex buffer
	ID3D11Buffer *vbs[2] = { vertBuffer, instancedBuffer };
	_d3dImmediateContext->IASetVertexBuffers(0, 2, vbs, stride, offset);

	cbBuffer.World = XMMatrixTranspose(worldMat);
	cbBuffer.WorldInvTranspose = D3DUtils::InverseTranspose(worldMat);
	cbBuffer.WorldViewProj = XMMatrixTranspose(worldMat * _camView* _camProj);
	cbBuffer.TexTransform = geoTexTransform;

	_d3dImmediateContext->UpdateSubresource(constBuffer, 0, NULL, &cbBuffer, 0, 0);
	_d3dImmediateContext->VSSetConstantBuffers(0, 1, &constBuffer);
	_d3dImmediateContext->PSSetConstantBuffers(1, 1, &constBuffer);
	_d3dImmediateContext->PSSetShaderResources(0, 1, &shaderResView);
	_d3dImmediateContext->PSSetShaderResources(1, 1, &normalShaderResView);
	_d3dImmediateContext->PSSetSamplers(0, 1, &texSamplerState);
	_d3dImmediateContext->RSSetState(_rs);
	_d3dImmediateContext->DrawIndexedInstanced(indicesCount, (UINT)instancedData.size(), 0, 0, 0);
	//_d3dImmediateContext->DrawIndexed(indicesCount, 0, 0);
}


