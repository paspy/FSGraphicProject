#pragma once
#include "Defines.h"
#include "D3DSturcture.h"

using namespace D3DSturcture;

class D3DUtils {

public:
	D3DUtils() {}

	template<typename T>
	static T Clamp(const T& _x, const T& _low, const T& _high) {
		return _x < _low ? _low : (_x > _high ? _high : _x);
	}

	static float AngleFromXY(float x, float y) {
		float theta = 0.0f;
		// Quadrant I or IV
		if ( x >= 0.0f ) {
			// If x = 0, then atanf(y/x) = +pi/2 if y > 0
			//                atanf(y/x) = -pi/2 if y < 0
			theta = atanf(y / x); // in [-pi/2, +pi/2]

			if ( theta < 0.0f )
				theta += 2.0f*XM_PI; // in [0, 2*pi).
		}

		// Quadrant II or III
		else
			theta = atanf(y / x) + XM_PI; // in [0, 2*pi).

		return theta;
	}

	static float DegreesToradians(float _degree) {
		return (_degree * XM_PI / 180.0f);
	}

	static float RandFloat() { return (float)(rand()) / (float)RAND_MAX; }

	static float RandFloat(float a, float b) { return a + RandFloat()*(b - a); }

	static bool CreateModelFromObjFileKaiNi(
		ID3D11Device *_d3dDevice,
		IDXGISwapChain *_swapChain,
		string _filename,
		ID3D11Buffer **_vertBuff,
		ID3D11Buffer **_indexBuff
		);

	static HRESULT CreateShaderAndLayoutFromFile(
		ID3D11Device *_d3dDevice,
		const LPCWSTR _fileName,
		const D3D11_INPUT_ELEMENT_DESC *_inputElemDesc,
		const UINT _elemNum,
		ID3D11VertexShader **_vertexShader,
		ID3D11PixelShader ** _pixelShader,
		ID3D11InputLayout **_inputLayout
		);
};
