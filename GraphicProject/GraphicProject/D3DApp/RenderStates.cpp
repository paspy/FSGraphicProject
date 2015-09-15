#include "RenderStates.h"

ID3D11RasterizerState* RenderStates::WireframeRS	= nullptr;
ID3D11RasterizerState* RenderStates::NoCullRS		= nullptr;
ID3D11RasterizerState* RenderStates::FrontCullRS	= nullptr;

ID3D11BlendState*      RenderStates::AlphaToCoverageBS	= nullptr;
ID3D11BlendState*      RenderStates::TransparentBSbyAlpha = nullptr;
ID3D11BlendState*      RenderStates::TransparentBSbyColor = nullptr;

void RenderStates::InitAll(ID3D11Device* _d3dDevice) {
	//
	// WireframeRS
	//
	D3D11_RASTERIZER_DESC wireframeDesc;
	ZeroMemory(&wireframeDesc, sizeof(D3D11_RASTERIZER_DESC));
	wireframeDesc.FillMode = D3D11_FILL_WIREFRAME;
	wireframeDesc.CullMode = D3D11_CULL_BACK;
	wireframeDesc.FrontCounterClockwise = false;
	wireframeDesc.DepthClipEnable = true;

	HR(_d3dDevice->CreateRasterizerState(&wireframeDesc, &WireframeRS));

	//
	// NoCullRS
	//
	D3D11_RASTERIZER_DESC noCullDesc;
	ZeroMemory(&noCullDesc, sizeof(D3D11_RASTERIZER_DESC));
	noCullDesc.FillMode = D3D11_FILL_SOLID;
	noCullDesc.CullMode = D3D11_CULL_NONE;
	noCullDesc.FrontCounterClockwise = false;
	noCullDesc.DepthClipEnable = true;

	HR(_d3dDevice->CreateRasterizerState(&noCullDesc, &NoCullRS));

	//
	// FrontCullRS
	//
	D3D11_RASTERIZER_DESC fontCullDesc;
	ZeroMemory(&fontCullDesc, sizeof(D3D11_RASTERIZER_DESC));
	fontCullDesc.FillMode = D3D11_FILL_SOLID;
	fontCullDesc.CullMode = D3D11_CULL_FRONT;
	fontCullDesc.FrontCounterClockwise = false;
	fontCullDesc.DepthClipEnable = true;

	HR(_d3dDevice->CreateRasterizerState(&fontCullDesc, &FrontCullRS));

	//
	// AlphaToCoverageBS
	//

	D3D11_BLEND_DESC alphaToCoverageDesc;
	ZeroMemory(&alphaToCoverageDesc, sizeof(D3D11_BLEND_DESC));

	alphaToCoverageDesc.AlphaToCoverageEnable = true;
	alphaToCoverageDesc.IndependentBlendEnable = false;
	alphaToCoverageDesc.RenderTarget[0].BlendEnable = false;
	alphaToCoverageDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	HR(_d3dDevice->CreateBlendState(&alphaToCoverageDesc, &AlphaToCoverageBS));

	//
	// TransparentBS by Color
	//

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

	//
	// TransparentBS by Alpha
	//

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
}

void RenderStates::DestroyAll() {
	SafeRelease(WireframeRS);
	SafeRelease(NoCullRS);
	SafeRelease(FrontCullRS);
	SafeRelease(AlphaToCoverageBS);
	SafeRelease(TransparentBSbyAlpha);
	SafeRelease(TransparentBSbyColor);
}