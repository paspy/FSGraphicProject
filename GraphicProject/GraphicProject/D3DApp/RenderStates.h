#pragma once
#include "Defines.h"

class RenderStates {
public:
	static void InitAll(ID3D11Device* _d3dDevice);
	static void DestroyAll();

	static ID3D11RasterizerState* WireframeRS;
	static ID3D11RasterizerState* NoCullRS;
	static ID3D11RasterizerState* FrontCullRS;

	static ID3D11BlendState* AlphaToCoverageBS;
	static ID3D11BlendState* TransparentBSbyColor;
	static ID3D11BlendState* TransparentBSbyAlpha;
};