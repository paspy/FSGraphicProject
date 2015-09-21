#include "Terrain.h"
#include "../D3DApp/D3DUtils.h"
#include "../D3DApp/Camera.h"
#include "../D3DApp/RenderStates.h"

Terrain::Terrain() :
	m_quadPatchVertBuffer(nullptr),
	m_quadPatchIndexBuffer(nullptr),
	m_layerMapArraySRV(nullptr),
	m_blendMapSRV(nullptr),
	m_heightMapSRV(nullptr),
	m_inputLayout(nullptr),
	m_vertexShader(nullptr),
	m_pixelShader(nullptr),
	m_hullShader(nullptr),
	m_domainShader(nullptr),
	m_cbPerFrame(nullptr),
	m_cbPerObject(nullptr),
	m_linerSamplerState(nullptr),
	m_heighMapSamplerState(nullptr),

	m_numPatchVertices(0),
	m_numPatchQuadFaces(0),
	m_numPatchVertRows(0),
	m_numPatchVertCols(0),
	m_wireFrameRS(false)

{
	m_world =  XMMatrixIdentity();

	m_material.Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_material.Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 64.0f);
	m_material.Reflect = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

}

Terrain::~Terrain() {
	SafeRelease(m_quadPatchVertBuffer);
	SafeRelease(m_quadPatchIndexBuffer);
	SafeRelease(m_vertexShader);
	SafeRelease(m_pixelShader);
	SafeRelease(m_domainShader);
	SafeRelease(m_hullShader);
	SafeRelease(m_cbPerObject);
	SafeRelease(m_cbPerFrame);
	SafeRelease(m_linerSamplerState);
	SafeRelease(m_heighMapSamplerState);
	SafeRelease(m_inputLayout)
	SafeRelease(m_layerMapArraySRV);
	SafeRelease(m_blendMapSRV);
	SafeRelease(m_heightMapSRV);
}

float Terrain::GetWidth()const {
	// Total terrain width.
	return (m_info.HeightmapWidth - 1)*m_info.CellSpacing;
}

float Terrain::GetDepth()const {
	// Total terrain depth.
	return (m_info.HeightmapHeight - 1)*m_info.CellSpacing;
}

float Terrain::GetHeight(float x, float z)const {
	// Transform from terrain local space to "cell" space.
	float c = (x + 0.5f*GetWidth()) / m_info.CellSpacing;
	float d = (z - 0.5f*GetDepth()) / -m_info.CellSpacing;

	// Get the row and column we are in.
	int row = (int)floorf(d);
	int col = (int)floorf(c);

	// Grab the heights of the cell we are in.
	// A*--*B
	//  | /|
	//  |/ |
	// C*--*D
	float A = m_heightmap[row*m_info.HeightmapWidth + col];
	float B = m_heightmap[row*m_info.HeightmapWidth + col + 1];
	float C = m_heightmap[(row + 1)*m_info.HeightmapWidth + col];
	float D = m_heightmap[(row + 1)*m_info.HeightmapWidth + col + 1];

	// Where we are relative to the cell.
	float s = c - (float)col;
	float t = d - (float)row;

	// If upper triangle ABC.
	if (s + t <= 1.0f) {
		float uy = B - A;
		float vy = C - A;
		return A + s*uy + t*vy;
	} else // lower triangle DCB.
	{
		float uy = C - D;
		float vy = B - D;
		return D + (1.0f - s)*uy + (1.0f - t)*vy;
	}
}

XMMATRIX Terrain::GetWorld()const {
	return m_world;
}

void Terrain::SetWorld(XMMATRIX M) {
	m_world = M;
}

void Terrain::Init(ID3D11Device* _d3dDevice, ID3D11DeviceContext* _context) {

	D3D11_BUFFER_DESC cbbd;
	ZeroMemory(&cbbd, sizeof(D3D11_BUFFER_DESC));
	cbbd.Usage = D3D11_USAGE_DEFAULT;
	cbbd.ByteWidth = sizeof(cbPerFrameT);
	cbbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbbd.CPUAccessFlags = 0;
	cbbd.MiscFlags = 0;
	HR(_d3dDevice->CreateBuffer(&cbbd, NULL, &m_cbPerFrame));

	ZeroMemory(&cbbd, sizeof(D3D11_BUFFER_DESC));
	cbbd.Usage = D3D11_USAGE_DEFAULT;
	cbbd.ByteWidth = sizeof(cbPerObjectT) * 4;
	cbbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbbd.CPUAccessFlags = 0;
	cbbd.MiscFlags = 0;
	HR(_d3dDevice->CreateBuffer(&cbbd, NULL, &m_cbPerObject));

	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	_d3dDevice->CreateSamplerState(&sampDesc, &m_linerSamplerState);

	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	_d3dDevice->CreateSamplerState(&sampDesc, &m_heighMapSamplerState);

	// create the depending shaders
	HR(D3DUtils::CreateShaderAndLayoutFromFile(_d3dDevice, L"Shaders/Terrain/BaseTerrain.hlsl", vertexLayout, 3, &m_vertexShader, &m_pixelShader, &m_inputLayout));
	HR(D3DUtils::CreateOptionalShaderFromFile(_d3dDevice, L"Shaders/Terrain/BaseTerrain.hlsl", &m_hullShader, &m_domainShader));

	m_info.HeightMapFilename = L"Resources/Terrain/terrain.raw";
	m_info.LayerMapFilename0 = L"Resources/Terrain/grass.dds";
	m_info.LayerMapFilename1 = L"Resources/Terrain/darkdirt.dds";
	m_info.LayerMapFilename2 = L"Resources/Terrain/stone.dds";
	m_info.LayerMapFilename3 = L"Resources/Terrain/lightdirt.dds";
	m_info.LayerMapFilename4 = L"Resources/Terrain/snow.dds";
	m_info.BlendMapFilename =  L"Resources/Terrain/blend.dds";
	m_info.HeightScale = 50.0f;
	m_info.HeightmapWidth = 2049;
	m_info.HeightmapHeight = 2049;
	m_info.CellSpacing = 0.5f;

	// Divide heightmap into patches such that each patch has CellsPerPatch.
	m_numPatchVertRows = ((m_info.HeightmapHeight - 1) / CellsPerPatch) + 1;
	m_numPatchVertCols = ((m_info.HeightmapWidth - 1) / CellsPerPatch) + 1;

	m_numPatchVertices = m_numPatchVertRows*m_numPatchVertCols;
	m_numPatchQuadFaces = (m_numPatchVertRows - 1)*(m_numPatchVertCols - 1);

	LoadHeightmap();
	Smooth();
	CalcAllPatchBoundsY();

	BuildQuadPatchVB(_d3dDevice);
	BuildQuadPatchIB(_d3dDevice);
	BuildHeightmapSRV(_d3dDevice);

	vector<wstring> layerFilenames;
	layerFilenames.push_back(m_info.LayerMapFilename0);
	layerFilenames.push_back(m_info.LayerMapFilename1);
	layerFilenames.push_back(m_info.LayerMapFilename2);
	layerFilenames.push_back(m_info.LayerMapFilename3);
	layerFilenames.push_back(m_info.LayerMapFilename4);
	m_layerMapArraySRV = D3DUtils::CreateTexture2DArraySRV(_d3dDevice, _context, layerFilenames);

	HR(CreateDDSTextureFromFile(_d3dDevice, m_info.BlendMapFilename.c_str(), NULL, &m_blendMapSRV));
}

void Terrain::Update() {
	if (GetAsyncKeyState('L') & 0x8000) 
		m_wireFrameRS = !m_wireFrameRS;
}

void Terrain::Render(ID3D11DeviceContext* _context, const Camera& _camera, D3DSturcture::DirectionalLight _light) {
	_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
	_context->OMSetDepthStencilState(NULL, 0);
	// Set the default shaders and depth/stencil state and layout
	_context->IASetInputLayout(m_inputLayout);
	_context->VSSetShader(m_vertexShader, NULL, 0);
	_context->HSSetShader(m_hullShader, NULL, 0);
	_context->DSSetShader(m_domainShader, NULL, 0);
	_context->PSSetShader(m_pixelShader, NULL, 0);

	_context->VSSetSamplers(0, 1, &m_heighMapSamplerState);
	_context->DSSetSamplers(0, 1, &m_heighMapSamplerState);

	_context->PSSetSamplers(0, 1, &m_linerSamplerState);
	_context->PSSetSamplers(1, 1, &m_heighMapSamplerState);


	float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };

	UINT stride = sizeof(VertexT);
	UINT offset = 0;
	_context->IASetVertexBuffers(0, 1, &m_quadPatchVertBuffer, &stride, &offset);
	_context->IASetIndexBuffer(m_quadPatchIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	XMMATRIX viewProj = _camera.GetViewProj();
	XMMATRIX world = m_world;
	XMMATRIX worldInvTranspose = D3DUtils::InverseTranspose(m_world);
	XMMATRIX worldViewProj = world*viewProj;

	XMFLOAT4 worldPlanes[6];
	D3DUtils::ExtractFrustumPlanes(worldPlanes, _camera.GetViewProj());

	// set per frame const buffer
	XMFLOAT4 curCamPos;
	XMStoreFloat4(&curCamPos, _camera.GetPosition());
	cbPerFrame.CameraPos = curCamPos;
	cbPerFrame.DirLight = _light;
	cbPerFrame.MinDist = 20.0f;
	cbPerFrame.MaxDist = 500.0f;
	cbPerFrame.MinTess = 0.0f;
	cbPerFrame.MaxTess = 6.0f;
	cbPerFrame.TexelCellSpaceU = 1.0f / m_info.HeightmapWidth;
	cbPerFrame.TexelCellSpaceV = 1.0f / m_info.HeightmapHeight;
	cbPerFrame.WorldCellSpace = m_info.CellSpacing;
	
	for (int i = 0; i < 6; i++) {
		cbPerFrame.WorldFrustumPlanes[i] = worldPlanes[i];
	}
	// set per object const buffer
	cbPerObj.material = m_material;
	cbPerObj.ViewProj = _camera.GetViewProj();


	_context->UpdateSubresource(m_cbPerObject, 0, NULL, &cbPerObj, 0, 0);
	_context->VSSetConstantBuffers(0, 1, &m_cbPerObject);
	_context->HSSetConstantBuffers(0, 1, &m_cbPerObject);
	_context->DSSetConstantBuffers(0, 1, &m_cbPerObject);	
	_context->PSSetConstantBuffers(0, 1, &m_cbPerObject);	

	_context->UpdateSubresource(m_cbPerFrame, 0, NULL, &cbPerFrame, 0, 0);
	_context->VSSetConstantBuffers(1, 1, &m_cbPerFrame);
	_context->HSSetConstantBuffers(1, 1, &m_cbPerFrame);
	_context->DSSetConstantBuffers(1, 1, &m_cbPerFrame);	
	_context->PSSetConstantBuffers(1, 1, &m_cbPerFrame);	

	_context->PSSetShaderResources(0, 1, &m_layerMapArraySRV);
	_context->PSSetShaderResources(1, 1, &m_blendMapSRV);
	_context->PSSetShaderResources(2, 1, &m_heightMapSRV);


	

	if (m_wireFrameRS) {
		_context->RSSetState(RenderStates::WireframeRS);
	} else {
		_context->RSSetState(0);
	}

	_context->DrawIndexed(m_numPatchQuadFaces * 4, 0, 0);
	
	// set back to default 
	_context->RSSetState(0);
	_context->HSSetShader(0, 0, 0);
	_context->DSSetShader(0, 0, 0);
	_context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

}

void Terrain::LoadHeightmap() {
	// A height for each vertex
	vector<unsigned char> in(m_info.HeightmapWidth * m_info.HeightmapHeight);

	// Open the file.
	ifstream inFile;
	inFile.open(m_info.HeightMapFilename.c_str(), ios_base::binary);

	if (inFile) {
		// Read the RAW bytes.
		inFile.read((char*)&in[0], (streamsize)in.size());

		// Done with file.
		inFile.close();
	}

	// Copy the array data into a float array and scale it.
	m_heightmap.resize(m_info.HeightmapHeight * m_info.HeightmapWidth, 0);
	for (UINT i = 0; i < m_info.HeightmapHeight * m_info.HeightmapWidth; ++i) {
		m_heightmap[i] = (in[i] / 255.0f)*m_info.HeightScale;
	}
}

void Terrain::Smooth() {
	vector<float> dest(m_heightmap.size());

	for (UINT i = 0; i < m_info.HeightmapHeight; ++i) {
		for (UINT j = 0; j < m_info.HeightmapWidth; ++j) {
			dest[i*m_info.HeightmapWidth + j] = Average(i, j);
		}
	}

	// Replace the old heightmap with the filtered one.
	m_heightmap = dest;
}

bool Terrain::InBounds(int i, int j) {
	// True if ij are valid indices; false otherwise.
	return
		i >= 0 && i < (int)m_info.HeightmapHeight &&
		j >= 0 && j < (int)m_info.HeightmapWidth;
}

float Terrain::Average(int i, int j) {
	// Function computes the average height of the ij element.
	// It averages itself with its eight neighbor pixels.  Note
	// that if a pixel is missing neighbor, we just don't include it
	// in the average--that is, edge pixels don't have a neighbor pixel.
	//
	// ----------
	// | 1| 2| 3|
	// ----------
	// |4 |ij| 6|
	// ----------
	// | 7| 8| 9|
	// ----------

	float avg = 0.0f;
	float num = 0.0f;

	// Use int to allow negatives.  If we use UINT, @ i=0, m=i-1=UINT_MAX
	// and no iterations of the outer for loop occur.
	for (int m = i - 1; m <= i + 1; ++m) {
		for (int n = j - 1; n <= j + 1; ++n) {
			if (InBounds(m, n)) {
				avg += m_heightmap[m*m_info.HeightmapWidth + n];
				num += 1.0f;
			}
		}
	}

	return avg / num;
}

void Terrain::CalcAllPatchBoundsY() {
	m_patchBoundsY.resize(m_numPatchQuadFaces);

	// For each patch
	for (UINT i = 0; i < m_numPatchVertRows - 1; ++i) {
		for (UINT j = 0; j < m_numPatchVertCols - 1; ++j) {
			CalcPatchBoundsY(i, j);
		}
	}
}

void Terrain::CalcPatchBoundsY(UINT i, UINT j) {
	// Scan the heightmap values this patch covers and compute the min/max height.

	UINT x0 = j*CellsPerPatch;
	UINT x1 = (j + 1)*CellsPerPatch;

	UINT y0 = i*CellsPerPatch;
	UINT y1 = (i + 1)*CellsPerPatch;

	float minY = +FLT_MAX;
	float maxY = -FLT_MAX;
	for (UINT y = y0; y <= y1; ++y) {
		for (UINT x = x0; x <= x1; ++x) {
			UINT k = y*m_info.HeightmapWidth + x;
			minY = min(minY, m_heightmap[k]);
			maxY = max(maxY, m_heightmap[k]);
		}
	}

	UINT patchID = i*(m_numPatchVertCols - 1) + j;
	m_patchBoundsY[patchID] = XMFLOAT2(minY, maxY);
}

void Terrain::BuildQuadPatchVB(ID3D11Device* _d3dDevice) {
	vector<VertexT> patchVertices(m_numPatchVertRows*m_numPatchVertCols);

	float halfWidth = 0.5f*GetWidth();
	float halfDepth = 0.5f*GetDepth();

	float patchWidth = GetWidth() / (m_numPatchVertCols - 1);
	float patchDepth = GetDepth() / (m_numPatchVertRows - 1);
	float du = 1.0f / (m_numPatchVertCols - 1);
	float dv = 1.0f / (m_numPatchVertRows - 1);

	for (UINT i = 0; i < m_numPatchVertRows; ++i) {
		float z = halfDepth - i*patchDepth;
		for (UINT j = 0; j < m_numPatchVertCols; ++j) {
			float x = -halfWidth + j*patchWidth;

			patchVertices[i*m_numPatchVertCols + j].Position = XMFLOAT3(x, 0.0f, z);

			// Stretch texture over grid.
			patchVertices[i*m_numPatchVertCols + j].TexCoord.x = j*du;
			patchVertices[i*m_numPatchVertCols + j].TexCoord.y = i*dv;
		}
	}

	// Store axis-aligned bounding box y-bounds in upper-left patch corner.
	for (UINT i = 0; i < m_numPatchVertRows - 1; ++i) {
		for (UINT j = 0; j < m_numPatchVertCols - 1; ++j) {
			UINT patchID = i*(m_numPatchVertCols - 1) + j;
			patchVertices[i*m_numPatchVertCols + j].BoundsY = m_patchBoundsY[patchID];
		}
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(VertexT) * static_cast<UINT>(patchVertices.size());
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &patchVertices[0];
	HR(_d3dDevice->CreateBuffer(&vbd, &vinitData, &m_quadPatchVertBuffer));
}

void Terrain::BuildQuadPatchIB(ID3D11Device* _d3dDevice) {
	vector<USHORT> indices(m_numPatchQuadFaces * 4); // 4 indices per quad face

	// Iterate over each quad and compute indices.
	int k = 0;
	for (UINT i = 0; i < m_numPatchVertRows - 1; ++i) {
		for (UINT j = 0; j < m_numPatchVertCols - 1; ++j) {
			// Top row of 2x2 quad patch
			indices[k] = i*m_numPatchVertCols + j;
			indices[k + 1] = i*m_numPatchVertCols + j + 1;

			// Bottom row of 2x2 quad patch
			indices[k + 2] = (i + 1)*m_numPatchVertCols + j;
			indices[k + 3] = (i + 1)*m_numPatchVertCols + j + 1;

			k += 4; // next quad
		}
	}

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(USHORT) * static_cast<UINT>(indices.size());
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
	HR(_d3dDevice->CreateBuffer(&ibd, &iinitData, &m_quadPatchIndexBuffer));
}

void Terrain::BuildHeightmapSRV(ID3D11Device* _d3dDevice) {
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = m_info.HeightmapWidth;
	texDesc.Height = m_info.HeightmapHeight;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R16_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	// HALF is defined in xnamath.h, for storing 16-bit float.
	vector<PackedVector::HALF> hmap(m_heightmap.size());
	transform(m_heightmap.begin(), m_heightmap.end(), hmap.begin(), PackedVector::XMConvertFloatToHalf);

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = &hmap[0];
	data.SysMemPitch = m_info.HeightmapWidth*sizeof(PackedVector::HALF);
	data.SysMemSlicePitch = 0;

	ID3D11Texture2D* hmapTex = 0;
	HR(_d3dDevice->CreateTexture2D(&texDesc, &data, &hmapTex));

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;
	HR(_d3dDevice->CreateShaderResourceView(hmapTex, &srvDesc, &m_heightMapSRV));

	// SRV saves reference.
	SafeRelease(hmapTex);
}