#include "BlurFilter.h"
#include "D3DUtils.h"

BlurFilter::BlurFilter() : 
	m_blurredOutputTexSRV(nullptr),
	m_blurredOutputTexUAV(nullptr),
	m_vertBlurCS(nullptr),
	m_horzBlurCS(nullptr) {

}

BlurFilter::~BlurFilter() {
	SafeRelease(m_blurredOutputTexSRV);
	SafeRelease(m_blurredOutputTexUAV);
	SafeRelease(m_vertBlurCS);
	SafeRelease(m_horzBlurCS);
	
}

ID3D11ShaderResourceView* BlurFilter::GetBlurredOutput() {
	return m_blurredOutputTexSRV;
}

void BlurFilter::SetGaussianWeights(float _sigma) {
	float d = 2.0f*_sigma*_sigma;

	float weights[9];
	float sum = 0.0f;
	for (int i = 0; i < 8; ++i) {
		float x = (float)i;
		weights[i] = expf(-x*x / d);

		sum += weights[i];
	}

	// Divide by the sum so all the weights add up to 1.0.
	for (int i = 0; i < 8; ++i) {
		weights[i] /= sum;
	}

	//Effects::BlurFX->SetWeights(weights);
}

void BlurFilter::SetWeights(const float weights[9]) {
	//Effects::BlurFX->SetWeights(weights);
}

void BlurFilter::Init(ID3D11Device* _d3dDevice, UINT _width, UINT _height, DXGI_FORMAT _format) {

	// Start fresh.
	SafeRelease(m_blurredOutputTexSRV);
	SafeRelease(m_blurredOutputTexUAV);
	SafeRelease(m_vertBlurCS);
	SafeRelease(m_horzBlurCS);

	m_width = _width;
	m_height = _height;
	m_format = _format;

	// Note, compressed formats cannot be used for UAV.  We get error like:
	// ERROR: ID3D11Device::CreateTexture2D: The _format (0x4d, BC3_UNORM) 
	// cannot be bound as an UnorderedAccessView, or cast to a _format that
	// could be bound as an UnorderedAccessView.  Therefore this _format 
	// does not support D3D11_BIND_UNORDERED_ACCESS.

	D3D11_TEXTURE2D_DESC blurredTexDesc;
	blurredTexDesc.Width = _width;
	blurredTexDesc.Height = _height;
	blurredTexDesc.MipLevels = 1;
	blurredTexDesc.ArraySize = 1;
	blurredTexDesc.Format = _format;
	blurredTexDesc.SampleDesc.Count = 1;
	blurredTexDesc.SampleDesc.Quality = 0;
	blurredTexDesc.Usage = D3D11_USAGE_DEFAULT;
	blurredTexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	blurredTexDesc.CPUAccessFlags = 0;
	blurredTexDesc.MiscFlags = 0;

	ID3D11Texture2D* blurredTex = nullptr;
	HR(_d3dDevice->CreateTexture2D(&blurredTexDesc, 0, &blurredTex));

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = _format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	HR(_d3dDevice->CreateShaderResourceView(blurredTex, &srvDesc, &m_blurredOutputTexSRV));

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	uavDesc.Format = _format;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;
	HR(_d3dDevice->CreateUnorderedAccessView(blurredTex, &uavDesc, &m_blurredOutputTexUAV));

	// Views save a reference to the texture so we can release our reference.
	SafeRelease(blurredTex);

	D3DUtils::CreateOptionalShaderFromFile(_d3dDevice, L"Shaders/PostFilter/Blur.hlsl", &m_vertBlurCS, "VertBlurCS");
	D3DUtils::CreateOptionalShaderFromFile(_d3dDevice, L"Shaders/PostFilter/Blur.hlsl", &m_horzBlurCS, "HorzBlurCS");
}

void BlurFilter::BlurInPlace(ID3D11DeviceContext* _context, ID3D11ShaderResourceView* _inputSRV, ID3D11UnorderedAccessView* _inputUAV, int _blurCount) {
	//
	// Run the compute shader to blur the offscreen texture.
	// 

	for (int i = 0; i < _blurCount; i++) {

		// HORIZONTAL blur pass.
		//D3DX11_TECHNIQUE_DESC techDesc;
		//Effects::BlurFX->HorzBlurTech->GetDesc(&techDesc);
		//for (UINT p = 0; p < techDesc.Passes; ++p) {
		//	Effects::BlurFX->SetInputMap(_inputSRV);
		//	Effects::BlurFX->SetOutputMap(m_blurredOutputTexUAV);
		//	Effects::BlurFX->HorzBlurTech->GetPassByIndex(p)->Apply(0, _context);

		

		// How many groups do we need to dispatch to cover a row of pixels, where each
		// group covers 256 pixels (the 256 is defined in the ComputeShader).
		UINT numGroupsX = (UINT)ceilf(m_width / 256.0f);
		_context->Dispatch(numGroupsX, m_height, 1);


		// Unbind the input texture from the CS for good housekeeping.
		ID3D11ShaderResourceView* nullSRV[1] = { 0 };
		_context->CSSetShaderResources(0, 1, nullSRV);

		// Unbind output from compute shader (we are going to use this output as an input in the next pass, 
		// and a resource cannot be both an output and input at the same time.
		ID3D11UnorderedAccessView* nullUAV[1] = { 0 };
		_context->CSSetUnorderedAccessViews(0, 1, nullUAV, 0);

		// VERTICAL blur pass.
		//Effects::BlurFX->VertBlurTech->GetDesc(&techDesc);
		//for (UINT p = 0; p < techDesc.Passes; ++p) {
		//	Effects::BlurFX->SetInputMap(m_blurredOutputTexSRV);
		//	Effects::BlurFX->SetOutputMap(_inputUAV);
		//	Effects::BlurFX->VertBlurTech->GetPassByIndex(p)->Apply(0, _context);

		//	// How many groups do we need to dispatch to cover a column of pixels, where each
		//	// group covers 256 pixels  (the 256 is defined in the ComputeShader).
		//	UINT numGroupsY = (UINT)ceilf(m_height / 256.0f);
		//	_context->Dispatch(m_width, numGroupsY, 1);
		//}

		_context->CSSetShaderResources(0, 1, nullSRV);
		_context->CSSetUnorderedAccessViews(0, 1, nullUAV, 0);
	}

	// Disable compute shader.
	_context->CSSetShader(0, 0, 0);
}
