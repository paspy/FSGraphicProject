#include "WaveMesh.h"

WaveMesh::~WaveMesh() {
	SafeRelease(inputLayout);
	SafeRelease(indexBuffer);
	SafeRelease(vertBuffer);
	SafeRelease(constBuffer);
	SafeRelease(vertexShader);
	SafeRelease(pixelShader);
	for ( size_t i = 0; i < shaderResView.size(); i++ ) {
		SafeRelease(shaderResView[i]);
	}
	SafeRelease(rasterState);
	SafeRelease(texSamplerState);
}
