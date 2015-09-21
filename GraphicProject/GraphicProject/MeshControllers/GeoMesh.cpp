#include "GeoMesh.h"
#include "../D3DApp/Camera.h"

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
	SafeRelease(CCWcullMode);
	SafeRelease(CWcullMode);
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

	D3D11_RASTERIZER_DESC cmdesc;
	ZeroMemory(&cmdesc, sizeof(cmdesc));

	cmdesc.FillMode = D3D11_FILL_SOLID;
	cmdesc.CullMode = D3D11_CULL_BACK;

	cmdesc.FrontCounterClockwise = true;
	HR(_d3dDevice->CreateRasterizerState(&cmdesc, &CCWcullMode));
	cmdesc.FrontCounterClockwise = false;
	HR(_d3dDevice->CreateRasterizerState(&cmdesc, &CWcullMode));

	// hard coded matrtial setting - NOT GOOD
	cbBuffer.material.Ambient = XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f);
	cbBuffer.material.Diffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	cbBuffer.material.Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 8.0f);

	// scaling texture
	geoTexTransform = XMMatrixScaling(1.0f, 1.0f, 0.0f);

	// loading the texture - using dds loader
	HR(CreateDDSTextureFromFile(_d3dDevice, _diffuseFile, NULL, &shaderResView));
	HR(CreateDDSTextureFromFile(_d3dDevice, _normalFile, NULL, &normalShaderResView));

	// create the depending shader
	HR(D3DUtils::CreateShaderAndLayoutFromFile(_d3dDevice, _shaderFilename, vertexLayout, 8, &vertexShader, &pixelShader, &inputLayout));

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

	InstancedData tmp;
	DirectX::XMStoreFloat4x4(&tmp.World, XMMatrixScaling(2.0f, 2.0f, 2.0f) * XMMatrixTranslation(5, 20, 0));
	instancedData.push_back(tmp);
	DirectX::XMStoreFloat4x4(&tmp.World, XMMatrixScaling(2.0f, 2.0f, 2.0f) * XMMatrixTranslation(0, 20, 0));
	instancedData.push_back(tmp);
	DirectX::XMStoreFloat4x4(&tmp.World, XMMatrixScaling(2.0f, 2.0f, 2.0f) * XMMatrixTranslation(-5, 20, 0));
	instancedData.push_back(tmp);

	D3D11_BUFFER_DESC vbd;
	ZeroMemory(&vbd, sizeof(vbd));
	vbd.Usage = D3D11_USAGE_DYNAMIC;
	vbd.ByteWidth = sizeof(InstancedData) * (UINT)instancedData.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;
	HR(_d3dDevice->CreateBuffer(&vbd, 0, &instancedBuffer));
}


float GeoMesh::GetDistanceFromCamera(XMFLOAT4X4 _m4x4, const Camera &_camera) {
	XMVECTOR tmpPos = XMVectorZero();
	XMMATRIX tmp;

	tmpPos = XMVector3TransformCoord(tmpPos, XMLoadFloat4x4(&_m4x4));

	float distX = XMVectorGetX(tmpPos) - XMVectorGetX(_camera.GetPosition());
	float distY = XMVectorGetY(tmpPos) - XMVectorGetY(_camera.GetPosition());
	float distZ = XMVectorGetZ(tmpPos) - XMVectorGetZ(_camera.GetPosition());

	return (distX*distX + distY*distY + distZ*distZ);
}


void GeoMesh::Update(ID3D11DeviceContext * _context, const Camera &_camera) {
	D3D11_MAPPED_SUBRESOURCE mapSubres;
	_context->Map(instancedBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapSubres);

	if ( GetDistanceFromCamera(instancedData[0].World, _camera) < GetDistanceFromCamera(instancedData[1].World, _camera) ) {
		swap(instancedData[0].World, instancedData[1].World);
		if ( GetDistanceFromCamera(instancedData[1].World, _camera) < GetDistanceFromCamera(instancedData[2].World, _camera) ) {
			swap(instancedData[1].World, instancedData[2].World);
		}
	} else {
		if ( GetDistanceFromCamera(instancedData[1].World, _camera) < GetDistanceFromCamera(instancedData[2].World, _camera) ) {
			swap(instancedData[1].World, instancedData[2].World);
		}
	}

	memcpy(mapSubres.pData, instancedData.data(), sizeof(InstancedData)*instancedData.size());
	_context->Unmap(instancedBuffer, 0);
}

void GeoMesh::Render(ID3D11DeviceContext * _context, const Camera &_camera, ID3D11BlendState* _bs = nullptr, float *_bf = nullptr) {

	// Set the default VS shader and depth/stencil state and layout
	_context->VSSetShader(vertexShader, NULL, 0);
	_context->PSSetShader(pixelShader, NULL, 0);
	_context->IASetInputLayout(inputLayout);
	_context->OMSetDepthStencilState(NULL, 0);

	//Set the index buffer
	_context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	
	//Set the vertex buffer
	ID3D11Buffer *vbs[2] = { vertBuffer, instancedBuffer };
	_context->IASetVertexBuffers(0, 2, vbs, stride, offset);

	cbBuffer.WorldInvTranspose = D3DUtils::InverseTranspose(worldMat);

	cbBuffer.View = XMMatrixTranspose(_camera.GetView());
	cbBuffer.Proj = XMMatrixTranspose(_camera.GetProj());
	cbBuffer.TexTransform = geoTexTransform;

	_context->UpdateSubresource(constBuffer, 0, NULL, &cbBuffer, 0, 0);

	_context->VSSetConstantBuffers(0, 1, &constBuffer);
	_context->PSSetConstantBuffers(1, 1, &constBuffer);
	_context->PSSetShaderResources(0, 1, &shaderResView);
	_context->PSSetShaderResources(1, 1, &normalShaderResView);
	_context->PSSetSamplers(0, 1, &texSamplerState);
	_context->OMSetBlendState(_bs, _bf, 0xffffffff);

	_context->RSSetState(CCWcullMode);
	_context->DrawIndexedInstanced(indicesCount, (UINT)instancedData.size(), 0, 0, 0);

	_context->RSSetState(CWcullMode);
	_context->DrawIndexedInstanced(indicesCount, (UINT)instancedData.size(), 0, 0, 0);
}


