#include "D3DUtils.h"

HRESULT D3DUtils::CreateShaderAndLayoutFromFile(
	ID3D11Device *_d3dDevice,
	const LPCWSTR _fileName,
	const D3D11_INPUT_ELEMENT_DESC *_inputElemDesc,
	const UINT _elemNum,
	ID3D11VertexShader **_vertexShader,
	ID3D11PixelShader ** _pixelShader,
	ID3D11InputLayout **_inputLayout ) {

	HRESULT hr;
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

