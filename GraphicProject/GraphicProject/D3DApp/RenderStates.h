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
	static ID3D11BlendState* NoRenderTargetWritesBS;

	static ID3D11DepthStencilState* MarkMirrorDSS;
	static ID3D11DepthStencilState* DrawReflectionDSS;
	static ID3D11DepthStencilState* NoDoubleBlendDSS;
};