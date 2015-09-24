#pragma once
#include "Defines.h"

class BlurFilter {
public:
	BlurFilter();
	~BlurFilter();

	ID3D11ShaderResourceView* GetBlurredOutput();

	// Generate Gaussian blur weights.
	void SetGaussianWeights(float _sigma);

	// Manually specify blur weights.
	void SetWeights(const float weights[9]);
	void Init(ID3D11Device* _d3dDevice, UINT _width, UINT _height, DXGI_FORMAT _format, bool _msaa);
	void BlurInPlace(ID3D11DeviceContext* _context, ID3D11ShaderResourceView* _inputSRV, ID3D11UnorderedAccessView* _inputUAV, int _blurCount);

private:

	UINT m_width;
	UINT m_height;
	DXGI_FORMAT m_format;

	ID3D11ComputeShader *m_vertBlurCS;
	ID3D11ComputeShader *m_horzBlurCS;


	ID3D11ShaderResourceView *m_blurredOutputTexSRV;
	ID3D11UnorderedAccessView *m_blurredOutputTexUAV;
};