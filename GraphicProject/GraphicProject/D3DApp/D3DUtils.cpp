#include "D3DUtils.h"
#include "DirectXTK/DDSTextureLoader.h"

HRESULT D3DUtils::CreateShaderAndLayoutFromFile(
	ID3D11Device *_d3dDevice,
	const LPCWSTR _fileName,
	const D3D11_INPUT_ELEMENT_DESC *_inputElemDesc,
	const UINT _elemNum,
	ID3D11VertexShader **_vertexShader,
	ID3D11PixelShader ** _pixelShader,
	ID3D11InputLayout **_inputLayout ) {

	HRESULT hr = E_NOTIMPL;
	ID3DBlob* vertexShaderBlob = nullptr;
	ID3DBlob* pixelShaderBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;

	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
	flags |= D3DCOMPILE_DEBUG;
#endif

	hr = D3DCompileFromFile(_fileName, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VSMain", "vs_5_0", flags, 0, &vertexShaderBlob, &errorBlob);

	if (FAILED(hr)) {
		if (errorBlob) {
			string err = (char*)errorBlob->GetBufferPointer();
			wstring werr(err.begin(), err.end());
			MessageBox(0, werr.c_str(), L"Error", MB_OK);
			//OutputDebugStringA((char*)errorBlob->GetBufferPointer());
			SafeRelease(errorBlob);
		}

		SafeRelease(vertexShaderBlob);
		return hr;
	}

	hr = D3DCompileFromFile(_fileName, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PSMain", "ps_5_0", flags, 0, &pixelShaderBlob, &errorBlob);

	if (FAILED(hr)) {
		if (errorBlob) {
			string err = (char*)errorBlob->GetBufferPointer();
			wstring werr(err.begin(), err.end());
			MessageBox(0, werr.c_str(), L"Error", MB_OK);
			//OutputDebugStringA((char*)errorBlob->GetBufferPointer());
			SafeRelease(errorBlob);
		}

		SafeRelease(pixelShaderBlob);
		return hr;
	}



	// create vertex layout
	HR(_d3dDevice->CreateInputLayout(_inputElemDesc, _elemNum, vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), _inputLayout));
	// create vertex shader
	HR(_d3dDevice->CreateVertexShader(vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), NULL, _vertexShader));
	// create pixel shader
	HR(_d3dDevice->CreatePixelShader(pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize(), NULL, _pixelShader));

	return hr;
}

HRESULT D3DUtils::CreateOptionalShaderFromFile(ID3D11Device * _d3dDevice, const LPCWSTR _tesselFileName, ID3D11HullShader ** _hullSahder, ID3D11DomainShader ** _domainShader) {
	HRESULT hr = E_NOTIMPL;
	ID3DBlob* hullShaderBlob = nullptr;
	ID3DBlob* domainShaderBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;

	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
	flags |= D3DCOMPILE_DEBUG;
#endif

	hr = D3DCompileFromFile(_tesselFileName, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "HSMain", "hs_5_0", flags, 0, &hullShaderBlob, &errorBlob);
	if ( FAILED(hr) ) {
		if ( errorBlob ) {
			string err = (char*)errorBlob->GetBufferPointer();
			wstring werr(err.begin(), err.end());
			MessageBox(0, werr.c_str(), L"Error", MB_OK);
			SafeRelease(errorBlob);
		}

		SafeRelease(hullShaderBlob);
		return hr;
	}

	hr = D3DCompileFromFile(_tesselFileName, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "DSMain", "ds_5_0", flags, 0, &domainShaderBlob, &errorBlob);
	if ( FAILED(hr) ) {
		if ( errorBlob ) {
			string err = (char*)errorBlob->GetBufferPointer();
			wstring werr(err.begin(), err.end());
			MessageBox(0, werr.c_str(), L"Error", MB_OK);
			SafeRelease(errorBlob);
		}

		SafeRelease(domainShaderBlob);
		return hr;
	}

	// create hull shader
	HR(_d3dDevice->CreateHullShader(hullShaderBlob->GetBufferPointer(), hullShaderBlob->GetBufferSize(), NULL, _hullSahder));
	// create domain shader
	HR(_d3dDevice->CreateDomainShader(domainShaderBlob->GetBufferPointer(), domainShaderBlob->GetBufferSize(), NULL, _domainShader));

	return hr;
}

HRESULT D3DUtils::CreateOptionalShaderFromFile(
	ID3D11Device * _d3dDevice, 
	const LPCWSTR _geoFileName,
	ID3D11GeometryShader ** _geoShader, 
	bool _streamOut,
	ID3D11VertexShader **_streamOutVS,
	D3D11_SO_DECLARATION_ENTRY *_streamOutDecl) {

	HRESULT hr = E_NOTIMPL;

	ID3DBlob* geoShaderBlob = nullptr;
	ID3DBlob* steamOutVSBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;
	
	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
	flags |= D3DCOMPILE_DEBUG;
#endif

	if (!_streamOut) {
		hr = D3DCompileFromFile(_geoFileName, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "GSMain", "gs_5_0", flags, 0, &geoShaderBlob, &errorBlob);
		if (FAILED(hr)) {
			if (errorBlob) {
				string err = (char*)errorBlob->GetBufferPointer();
				wstring werr(err.begin(), err.end());
				MessageBox(0, werr.c_str(), L"Error", MB_OK);
				SafeRelease(errorBlob);
			}

			SafeRelease(geoShaderBlob);
			return hr;
		}
		// create geometry shader
		HR(_d3dDevice->CreateGeometryShader(geoShaderBlob->GetBufferPointer(), geoShaderBlob->GetBufferSize(), NULL, _geoShader));
	} else {
		if (_streamOutVS == nullptr || _streamOutDecl == nullptr) {
			return E_INVALIDARG;
		}
		hr = D3DCompileFromFile(_geoFileName, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "StreamOutVS", "vs_5_0", flags, 0, &steamOutVSBlob, &errorBlob);

		if (FAILED(hr)) {
			if (errorBlob) {
				string err = (char*)errorBlob->GetBufferPointer();
				wstring werr(err.begin(), err.end());
				MessageBox(0, werr.c_str(), L"Error", MB_OK);
				SafeRelease(errorBlob);
			}

			SafeRelease(steamOutVSBlob);
			return hr;
		}

		hr = D3DCompileFromFile(_geoFileName, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "StreamOutGS", "gs_5_0", flags, 0, &geoShaderBlob, &errorBlob);
		if (FAILED(hr)) {
			if (errorBlob) {
				string err = (char*)errorBlob->GetBufferPointer();
				wstring werr(err.begin(), err.end());
				MessageBox(0, werr.c_str(), L"Error", MB_OK);
				SafeRelease(errorBlob);
			}

			SafeRelease(geoShaderBlob);
			return hr;
		}
		HR(_d3dDevice->CreateVertexShader(steamOutVSBlob->GetBufferPointer(), steamOutVSBlob->GetBufferSize(), NULL, _streamOutVS));
		HR(_d3dDevice->CreateGeometryShaderWithStreamOutput(geoShaderBlob->GetBufferPointer(), geoShaderBlob->GetBufferSize(), _streamOutDecl, 5, NULL, 0, 0, NULL, _geoShader));
	}
	
	return hr;
}

HRESULT D3DUtils::CreateOptionalShaderFromFile(ID3D11Device * _d3dDevice, const LPCWSTR _compFileName, ID3D11ComputeShader ** _compShader) {
	HRESULT hr = E_NOTIMPL;

	ID3DBlob* compShaderBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;

	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
	flags |= D3DCOMPILE_DEBUG;
#endif

	hr = D3DCompileFromFile(_compFileName, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "CSMain", "cs_5_0", flags, 0, &compShaderBlob, &errorBlob);
	if ( FAILED(hr) ) {
		if ( errorBlob ) {
			string err = (char*)errorBlob->GetBufferPointer();
			wstring werr(err.begin(), err.end());
			MessageBox(0, werr.c_str(), L"Error", MB_OK);
			SafeRelease(errorBlob);
		}

		SafeRelease(compShaderBlob);
		return hr;
	}

	// create geometry shader
	HR(_d3dDevice->CreateComputeShader(compShaderBlob->GetBufferPointer(), compShaderBlob->GetBufferSize(), NULL, _compShader));

	return hr;
}

void D3DUtils::ExtractFrustumPlanes(XMFLOAT4 planes[6], XMMATRIX M) {
	// Left
	planes[0].x = M.r[0].m128_f32[3] + M.r[0].m128_f32[0];
	planes[0].y = M.r[1].m128_f32[3] + M.r[1].m128_f32[0];
	planes[0].z = M.r[2].m128_f32[3] + M.r[2].m128_f32[0];
	planes[0].w = M.r[3].m128_f32[3] + M.r[3].m128_f32[0];

	// Right
	planes[1].x = M.r[0].m128_f32[3] - M.r[0].m128_f32[0];
	planes[1].y = M.r[1].m128_f32[3] - M.r[1].m128_f32[0];
	planes[1].z = M.r[2].m128_f32[3] - M.r[2].m128_f32[0];
	planes[1].w = M.r[3].m128_f32[3] - M.r[3].m128_f32[0];

	// Bottom
	planes[2].x = M.r[0].m128_f32[3] + M.r[0].m128_f32[1];
	planes[2].y = M.r[1].m128_f32[3] + M.r[1].m128_f32[1];
	planes[2].z = M.r[2].m128_f32[3] + M.r[2].m128_f32[1];
	planes[2].w = M.r[3].m128_f32[3] + M.r[3].m128_f32[1];

	// Top
	planes[3].x = M.r[0].m128_f32[3] - M.r[0].m128_f32[1];
	planes[3].y = M.r[1].m128_f32[3] - M.r[1].m128_f32[1];
	planes[3].z = M.r[2].m128_f32[3] - M.r[2].m128_f32[1];
	planes[3].w = M.r[3].m128_f32[3] - M.r[3].m128_f32[1];

	// Near
	planes[4].x = M.r[0].m128_f32[2];
	planes[4].y = M.r[1].m128_f32[2];
	planes[4].z = M.r[2].m128_f32[2];
	planes[4].w = M.r[3].m128_f32[2];

	// Far
	planes[5].x = M.r[0].m128_f32[3] - M.r[0].m128_f32[2];
	planes[5].y = M.r[1].m128_f32[3] - M.r[1].m128_f32[2];
	planes[5].z = M.r[2].m128_f32[3] - M.r[2].m128_f32[2];
	planes[5].w = M.r[3].m128_f32[3] - M.r[3].m128_f32[2];

	// Normalize the plane equations.
	for ( int i = 0; i < 6; ++i ) {
		XMVECTOR v = XMPlaneNormalize(XMLoadFloat4(&planes[i]));
		XMStoreFloat4(&planes[i], v);
	}
}

ID3D11ShaderResourceView * D3DUtils::CreateRandomTexture1DSRV(ID3D11Device * _d3dDevice) {
	// Create the random data.
	XMFLOAT4 randomValues[1024];

	for (int i = 0; i < 1024; ++i) {
		randomValues[i].x = RandFloat(-1.0f, 1.0f);
		randomValues[i].y = RandFloat(-1.0f, 1.0f);
		randomValues[i].z = RandFloat(-1.0f, 1.0f);
		randomValues[i].w = RandFloat(-1.0f, 1.0f);
	}

	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = randomValues;
	initData.SysMemPitch = 1024 * sizeof(XMFLOAT4);
	initData.SysMemSlicePitch = 0;

	//
	// Create the texture.
	//
	D3D11_TEXTURE1D_DESC texDesc;
	texDesc.Width = 1024;
	texDesc.MipLevels = 1;
	texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	texDesc.Usage = D3D11_USAGE_IMMUTABLE;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	texDesc.ArraySize = 1;

	ID3D11Texture1D* randomTex = 0;
	HR(_d3dDevice->CreateTexture1D(&texDesc, &initData, &randomTex));

	//
	// Create the resource view.
	//
	D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
	viewDesc.Format = texDesc.Format;
	viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
	viewDesc.Texture1D.MipLevels = texDesc.MipLevels;
	viewDesc.Texture1D.MostDetailedMip = 0;

	ID3D11ShaderResourceView* randomTexSRV = 0;
	HR(_d3dDevice->CreateShaderResourceView(randomTex, &viewDesc, &randomTexSRV));

	SafeRelease(randomTex);

	return randomTexSRV;
}

ID3D11ShaderResourceView * D3DUtils::CreateTexture2DArraySRV(
	ID3D11Device * _d3dDevice, 
	ID3D11DeviceContext * context, 
	vector<wstring>& filenames) {

	UINT size = static_cast<UINT>(filenames.size());

	vector<ID3D11Texture2D*> srcTex(size);
	for (UINT i = 0; i < size; ++i) {
		//HR(CreateDDSTextureFromFile(_d3dDevice, filenames[i].c_str(), (ID3D11Resource**)&srcTex[i], NULL));
		CreateDDSTextureFromFileEx(
			_d3dDevice, context, filenames[i].c_str(), NULL, D3D11_USAGE_STAGING, 0, D3D11_CPU_ACCESS_READ, 0, false, (ID3D11Resource**)&srcTex[i], NULL);
	}

	D3D11_TEXTURE2D_DESC texElementDesc;
	srcTex[0]->GetDesc(&texElementDesc);

	D3D11_TEXTURE2D_DESC texArrayDesc;
	texArrayDesc.Width = texElementDesc.Width;
	texArrayDesc.Height = texElementDesc.Height;
	texArrayDesc.MipLevels = texElementDesc.MipLevels;
	texArrayDesc.ArraySize = size;
	texArrayDesc.Format = texElementDesc.Format;
	texArrayDesc.SampleDesc.Count = 1;
	texArrayDesc.SampleDesc.Quality = 0;
	texArrayDesc.Usage = D3D11_USAGE_DEFAULT;
	texArrayDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texArrayDesc.CPUAccessFlags = 0;
	texArrayDesc.MiscFlags = 0;

	ID3D11Texture2D* texArray = 0;
	HR(_d3dDevice->CreateTexture2D(&texArrayDesc, 0, &texArray));

	// for each texture element...
	for (UINT texElement = 0; texElement < size; ++texElement) {
		// for each mipmap level...
		for (UINT mipLevel = 0; mipLevel < texElementDesc.MipLevels; ++mipLevel) {
			D3D11_MAPPED_SUBRESOURCE mappedTex2D;
			HR(context->Map(srcTex[texElement], mipLevel, D3D11_MAP_READ, 0, &mappedTex2D));

			context->UpdateSubresource(
				texArray, D3D11CalcSubresource(mipLevel, texElement, texElementDesc.MipLevels),
				0, mappedTex2D.pData, mappedTex2D.RowPitch, mappedTex2D.DepthPitch
				);

			context->Unmap(srcTex[texElement], mipLevel);
		}
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
	viewDesc.Format = texArrayDesc.Format;
	viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	viewDesc.Texture2DArray.MostDetailedMip = 0;
	viewDesc.Texture2DArray.MipLevels = texArrayDesc.MipLevels;
	viewDesc.Texture2DArray.FirstArraySlice = 0;
	viewDesc.Texture2DArray.ArraySize = size;

	ID3D11ShaderResourceView* texArraySRV = nullptr;
	HR(_d3dDevice->CreateShaderResourceView(texArray, &viewDesc, &texArraySRV));

	//
	// Cleanup--we only need the resource view.
	//

	SafeRelease(texArray);

	for (UINT i = 0; i < size; ++i) {
		SafeRelease(srcTex[i]);
	}

	return texArraySRV;
}

vector<ID3D11ShaderResourceView*> D3DUtils::CreateTexture2DArraySRV(
	ID3D11Device * _d3dDevice,
	vector<wstring>& filenames) {

	UINT size = static_cast<UINT>(filenames.size());

	vector<ID3D11ShaderResourceView*> srcTex(size);
	for (UINT i = 0; i < size; ++i) {
		HR(CreateDDSTextureFromFile(_d3dDevice, filenames[i].c_str(), NULL, &srcTex[i]));
	}

	return srcTex;
}

// deprecated
bool D3DUtils::CreateModelFromObjFileKaiNi(ID3D11Device * _d3dDevice, IDXGISwapChain * _swapChain, string _filename, ID3D11Buffer ** _vertBuff, ID3D11Buffer ** _indexBuff) {

	vector<shape_t> shapes;
	vector<material_t> materials;

	string err = tinyobj::LoadObj(shapes, materials, _filename.c_str(), "Resources/Models/");

	if (!err.empty()) {
		wstring werr(err.begin(), err.end());
		MessageBox(0, werr.c_str(), L"Error", MB_OK);
		return false;
	}

	std::cout << "# of shapes    : " << shapes.size() << std::endl;
	std::cout << "# of materials : " << materials.size() << std::endl;

	return false;
}

