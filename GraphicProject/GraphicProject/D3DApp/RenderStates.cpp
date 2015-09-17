#include "RenderStates.h"

ID3D11RasterizerState*		RenderStates::WireframeRS = nullptr;
ID3D11RasterizerState*		RenderStates::NoCullRS	  = nullptr;
ID3D11RasterizerState*		RenderStates::FrontCullRS = nullptr;

ID3D11BlendState*			RenderStates::AlphaToCoverageBS		 = nullptr;
ID3D11BlendState*			RenderStates::TransparentBSbyAlpha	 = nullptr;
ID3D11BlendState*			RenderStates::TransparentBSbyColor	 = nullptr;
ID3D11BlendState*			RenderStates::NoRenderTargetWritesBS = nullptr;

ID3D11DepthStencilState*	RenderStates::MarkMirrorDSS		= nullptr;
ID3D11DepthStencilState*	RenderStates::DrawReflectionDSS	= nullptr;
ID3D11DepthStencilState*	RenderStates::NoDoubleBlendDSS	= nullptr;

void RenderStates::InitAll(ID3D11Device* _d3dDevice) {

	// WireframeRS
	D3D11_RASTERIZER_DESC wireframeDesc;
	ZeroMemory(&wireframeDesc, sizeof(D3D11_RASTERIZER_DESC));
	wireframeDesc.FillMode = D3D11_FILL_WIREFRAME;
	wireframeDesc.CullMode = D3D11_CULL_BACK;
	wireframeDesc.FrontCounterClockwise = false;
	wireframeDesc.DepthClipEnable = true;

	HR(_d3dDevice->CreateRasterizerState(&wireframeDesc, &WireframeRS));

	// NoCullRS
	D3D11_RASTERIZER_DESC noCullDesc;
	ZeroMemory(&noCullDesc, sizeof(D3D11_RASTERIZER_DESC));
	noCullDesc.FillMode = D3D11_FILL_SOLID;
	noCullDesc.CullMode = D3D11_CULL_NONE;
	noCullDesc.FrontCounterClockwise = false;
	noCullDesc.DepthClipEnable = true;

	HR(_d3dDevice->CreateRasterizerState(&noCullDesc, &NoCullRS));

	// FrontCullRS
	D3D11_RASTERIZER_DESC fontCullDesc;
	ZeroMemory(&fontCullDesc, sizeof(D3D11_RASTERIZER_DESC));
	fontCullDesc.FillMode = D3D11_FILL_SOLID;
	fontCullDesc.CullMode = D3D11_CULL_FRONT;
	fontCullDesc.FrontCounterClockwise = false;
	fontCullDesc.DepthClipEnable = true;

	HR(_d3dDevice->CreateRasterizerState(&fontCullDesc, &FrontCullRS));

	// AlphaToCoverageBS
	D3D11_BLEND_DESC alphaToCoverageDesc;
	ZeroMemory(&alphaToCoverageDesc, sizeof(D3D11_BLEND_DESC));
	alphaToCoverageDesc.AlphaToCoverageEnable = true;
	alphaToCoverageDesc.IndependentBlendEnable = false;
	alphaToCoverageDesc.RenderTarget[0].BlendEnable = false;
	alphaToCoverageDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	HR(_d3dDevice->CreateBlendState(&alphaToCoverageDesc, &AlphaToCoverageBS));

	// TransparentBS by Color
	D3D11_BLEND_DESC transparentDesc;
	ZeroMemory(&transparentDesc, sizeof(D3D11_BLEND_DESC));
	transparentDesc.AlphaToCoverageEnable = false;
	transparentDesc.IndependentBlendEnable = false;
	transparentDesc.RenderTarget[0].BlendEnable = true;
	transparentDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_COLOR;
	transparentDesc.RenderTarget[0].DestBlend = D3D11_BLEND_BLEND_FACTOR;
	transparentDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	transparentDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	transparentDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	transparentDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	transparentDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	HR(_d3dDevice->CreateBlendState(&transparentDesc, &TransparentBSbyColor));

	// TransparentBS by Alpha
	ZeroMemory(&transparentDesc, sizeof(D3D11_BLEND_DESC));
	transparentDesc.AlphaToCoverageEnable = false;
	transparentDesc.IndependentBlendEnable = false;
	transparentDesc.RenderTarget[0].BlendEnable = true;
	transparentDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	transparentDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	transparentDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	transparentDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	transparentDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	transparentDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	transparentDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	HR(_d3dDevice->CreateBlendState(&transparentDesc, &TransparentBSbyAlpha));

	// NoRenderTargetWritesBS
	D3D11_BLEND_DESC noRenderTargetWritesDesc;
	ZeroMemory(&noRenderTargetWritesDesc, sizeof(D3D11_BLEND_DESC));
	noRenderTargetWritesDesc.AlphaToCoverageEnable = false;
	noRenderTargetWritesDesc.IndependentBlendEnable = false;
	noRenderTargetWritesDesc.RenderTarget[0].BlendEnable = false;
	noRenderTargetWritesDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	noRenderTargetWritesDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
	noRenderTargetWritesDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	noRenderTargetWritesDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	noRenderTargetWritesDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	noRenderTargetWritesDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	noRenderTargetWritesDesc.RenderTarget[0].RenderTargetWriteMask = 0;

	HR(_d3dDevice->CreateBlendState(&noRenderTargetWritesDesc, &NoRenderTargetWritesBS));

	// MarkMirrorDSS
	D3D11_DEPTH_STENCIL_DESC mirrorDesc;
	ZeroMemory(&mirrorDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));

	mirrorDesc.DepthEnable = true;
	mirrorDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	mirrorDesc.DepthFunc = D3D11_COMPARISON_LESS;
	mirrorDesc.StencilEnable = true;
	mirrorDesc.StencilReadMask = 0xff;
	mirrorDesc.StencilWriteMask = 0xff;

	mirrorDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	mirrorDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	mirrorDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	mirrorDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// We are not rendering backfacing polygons, so these settings do not matter.
	mirrorDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	mirrorDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	mirrorDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	mirrorDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	HR(_d3dDevice->CreateDepthStencilState(&mirrorDesc, &MarkMirrorDSS));

	// DrawReflectionDSS
	D3D11_DEPTH_STENCIL_DESC drawReflectionDesc;
	ZeroMemory(&drawReflectionDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	drawReflectionDesc.DepthEnable = true;
	drawReflectionDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	drawReflectionDesc.DepthFunc = D3D11_COMPARISON_LESS;
	drawReflectionDesc.StencilEnable = true;
	drawReflectionDesc.StencilReadMask = 0xff;
	drawReflectionDesc.StencilWriteMask = 0xff;

	drawReflectionDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	drawReflectionDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	drawReflectionDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	drawReflectionDesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;

	// We are not rendering backfacing polygons, so these settings do not matter.
	drawReflectionDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	drawReflectionDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	drawReflectionDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	drawReflectionDesc.BackFace.StencilFunc = D3D11_COMPARISON_EQUAL;

	HR(_d3dDevice->CreateDepthStencilState(&drawReflectionDesc, &DrawReflectionDSS));

	// NoDoubleBlendDSS
	D3D11_DEPTH_STENCIL_DESC noDoubleBlendDesc;
	ZeroMemory(&noDoubleBlendDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	noDoubleBlendDesc.DepthEnable = true;
	noDoubleBlendDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	noDoubleBlendDesc.DepthFunc = D3D11_COMPARISON_LESS;
	noDoubleBlendDesc.StencilEnable = true;
	noDoubleBlendDesc.StencilReadMask = 0xff;
	noDoubleBlendDesc.StencilWriteMask = 0xff;

	noDoubleBlendDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	noDoubleBlendDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	noDoubleBlendDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_INCR;
	noDoubleBlendDesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;

	// We are not rendering backfacing polygons, so these settings do not matter.
	noDoubleBlendDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	noDoubleBlendDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	noDoubleBlendDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_INCR;
	noDoubleBlendDesc.BackFace.StencilFunc = D3D11_COMPARISON_EQUAL;

	HR(_d3dDevice->CreateDepthStencilState(&noDoubleBlendDesc, &NoDoubleBlendDSS));

}

void RenderStates::DestroyAll() {
	SafeRelease(WireframeRS);
	SafeRelease(NoCullRS);
	SafeRelease(FrontCullRS);
	SafeRelease(AlphaToCoverageBS);
	SafeRelease(TransparentBSbyAlpha);
	SafeRelease(TransparentBSbyColor);
	SafeRelease(NoRenderTargetWritesBS);
	SafeRelease(MarkMirrorDSS);
	SafeRelease(DrawReflectionDSS);
	SafeRelease(NoDoubleBlendDSS);
}