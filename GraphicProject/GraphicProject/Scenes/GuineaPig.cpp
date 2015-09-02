#include "GuineaPig.h"

GuineaPig::GuineaPig(HINSTANCE hinst) : D3DApp(hinst),
m_groundVertexBuffer(nullptr),
m_inputLayout(nullptr),
m_vertexShader(nullptr),
m_pixelShader(nullptr) 
{

}

GuineaPig::~GuineaPig() {

	// release geometries ptr
	SafeRelease(m_groundIndexBuffer);
	SafeRelease(m_groundVertexBuffer);

	// release shader ptr
	SafeRelease(m_vertexShader);
	SafeRelease(m_pixelShader);

	// release layout ptr
	SafeRelease(m_inputLayout);

	// release constant buffer ptr
	SafeRelease(m_cbMeshBuffer);
	SafeRelease(m_cbGroundBuffer);
	SafeRelease(m_cbPerFrameBuffer);

	// release texture ptr
	// the cube
	SafeRelease(m_baseTexSamplerState);

	// the ground
	SafeRelease(m_grassShaderResView);

	// release lighting ptr
	SafeRelease(m_perFrameBuffer);

	// release render state ptr
	SafeRelease(m_antialiasedLine);
	SafeRelease(m_blendTransparency);
	SafeRelease(m_cwCullingMode);
	SafeRelease(m_ccwCullingMode);

	// obj loader
	SafeRelease(m_meshVertBuff);
	SafeRelease(m_meshIndexBuff);
	for ( size_t i = 0; i < m_meshShaderResView.size(); i++ ) {
		SafeRelease(m_meshShaderResView[i]);
	}
}

bool GuineaPig::Init() {
	if ( !D3DApp::Init() )
		return false;

	BuildConstBuffer();
	BuildGeometry();
	BuildLighting();
	BuildShaderAndLayout();
	BuildRenderStates();

	return true;
}

void GuineaPig::OnResize() {
	D3DApp::OnResize();

}

void GuineaPig::BuildConstBuffer() {
	// objects
	D3D11_BUFFER_DESC cbbd;
	ZeroMemory(&cbbd, sizeof(D3D11_BUFFER_DESC));

	cbbd.Usage = D3D11_USAGE_DEFAULT;
	cbbd.ByteWidth = sizeof(ConstPerObject);
	cbbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbbd.CPUAccessFlags = 0;
	cbbd.MiscFlags = 0;

	HR(m_d3dDevice->CreateBuffer(&cbbd, NULL, &m_cbGroundBuffer));

	HR(m_d3dDevice->CreateBuffer(&cbbd, NULL, &m_cbMeshBuffer));

	// lightings
	ZeroMemory(&cbbd, sizeof(D3D11_BUFFER_DESC));
	cbbd.Usage = D3D11_USAGE_DEFAULT;
	cbbd.ByteWidth = sizeof(ConstPerFrame);
	cbbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbbd.CPUAccessFlags = 0;
	cbbd.MiscFlags = 0;
	HR(m_d3dDevice->CreateBuffer(&cbbd, NULL, &m_cbPerFrameBuffer));

}

void GuineaPig::BuildGeometry() {

	D3DUtils::BuildSphere(m_d3dDevice ,20, 20, &m_skyBox.vertBuffer, &m_skyBox.indexBuffer, m_skyBox.numVertices, m_skyBox.numFaces);
	// loading the texture - using dds loader
	HR(CreateDDSTextureFromFile(m_d3dDevice, L"Resources/Skybox/skymap.dds", NULL, &m_skyBox.shaderResView));

	//D3DUtils::CreateModelFromObjFile(
	//	m_d3dDevice,
	//	m_swapChain,
	//	L"Resources/Models/ground.obj",
	//	&m_meshVertBuff,
	//	&m_meshIndexBuff,
	//	m_textureNameArray,
	//	m_meshShaderResView,
	//	m_meshSubsetIndexStart,
	//	m_meshSubsetTexture,
	//	m_materials,
	//	m_meshSubsets,
	//	true,
	//	false);


	// Describe the Sample State
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));

	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// Create the Sample State
	HR(m_d3dDevice->CreateSamplerState(&sampDesc, &m_baseTexSamplerState));
}

void GuineaPig::BuildLighting() {
	// Direction light setting
	//m_baseLight.direction = XMFLOAT3(0.0f, 1.0f, 0.0f);
	//m_baseLight.ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	//m_baseLight.diffuse = XMFLOAT4(1.2f, 1.2f, 1.2f, 1.2f);

	// Point light setting
	//m_baseLight.position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	//m_baseLight.range = 50.0f;
	//m_baseLight.attenuation = XMFLOAT3(0.0f, 0.1f, 0.0f);
	//m_baseLight.ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	//m_baseLight.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	// Point light setting
	m_baseLight.position = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_baseLight.spotLightDir = XMFLOAT3(0.0f, 0.0f, 1.0f);
	m_baseLight.range = 1000.0f;
	m_baseLight.cone = 20.0f;
	m_baseLight.attenuation = XMFLOAT3(0.4f, 0.02f, 0.0f);
	m_baseLight.ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	m_baseLight.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
}

void GuineaPig::BuildShaderAndLayout() {
	// normal shader
	D3D11_INPUT_ELEMENT_DESC vertLayout[] = {
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT",	0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	HR(D3DUtils::CreateShaderAndLayoutFromFile(m_d3dDevice, L"Shaders/Base/Base.hlsl", vertLayout, 4, &m_vertexShader, &m_pixelShader, &m_inputLayout));

	HR(D3DUtils::CreateShaderAndLayoutFromFile(m_d3dDevice, L"Shaders/Skybox/Skybox.hlsl", m_skyBox.vertexLayout, 2, &m_skyBox.vertexShader, &m_skyBox.pixelShader, &m_skyBox.inputLayout));

}


void GuineaPig::BuildRenderStates() {

	// Raster Description	
	D3D11_RASTERIZER_DESC rasterDesc;
	ZeroMemory(&rasterDesc, sizeof(D3D11_RASTERIZER_DESC));

	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.CullMode = D3D11_CULL_NONE;
	rasterDesc.AntialiasedLineEnable = true;

	HR(m_d3dDevice->CreateRasterizerState(&rasterDesc, &m_antialiasedLine));

	// create blending description
	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(blendDesc));

	D3D11_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc;
	ZeroMemory(&renderTargetBlendDesc, sizeof(renderTargetBlendDesc));

	renderTargetBlendDesc.BlendEnable = true;
	renderTargetBlendDesc.SrcBlend = D3D11_BLEND_SRC_COLOR;
	renderTargetBlendDesc.DestBlend = D3D11_BLEND_BLEND_FACTOR;
	renderTargetBlendDesc.BlendOp = D3D11_BLEND_OP_ADD;
	renderTargetBlendDesc.SrcBlendAlpha = D3D11_BLEND_ONE;
	renderTargetBlendDesc.DestBlendAlpha = D3D11_BLEND_ZERO;
	renderTargetBlendDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	renderTargetBlendDesc.RenderTargetWriteMask = D3D10_COLOR_WRITE_ENABLE_ALL;

	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.RenderTarget[0] = renderTargetBlendDesc;

	HR(m_d3dDevice->CreateBlendState(&blendDesc, &m_blendTransparency));

	// create counter-clockwise and clockwise description
	D3D11_RASTERIZER_DESC cmdesc;
	ZeroMemory(&cmdesc, sizeof(D3D11_RASTERIZER_DESC));

	cmdesc.FillMode = D3D11_FILL_SOLID;
	cmdesc.CullMode = D3D11_CULL_BACK;

	cmdesc.FrontCounterClockwise = true;
	HR(m_d3dDevice->CreateRasterizerState(&cmdesc, &m_ccwCullingMode));

	cmdesc.FrontCounterClockwise = false;
	HR(m_d3dDevice->CreateRasterizerState(&cmdesc, &m_cwCullingMode));

	// skybox render state
	ZeroMemory(&cmdesc, sizeof(D3D11_RASTERIZER_DESC));
	cmdesc.FillMode = D3D11_FILL_SOLID;
	cmdesc.CullMode = D3D11_CULL_NONE;

	HR(m_d3dDevice->CreateRasterizerState(&cmdesc, &m_skyBox.rasterState));

	D3D11_DEPTH_STENCIL_DESC dssDesc;
	ZeroMemory(&dssDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	dssDesc.DepthEnable = true;
	dssDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dssDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

	HR(m_d3dDevice->CreateDepthStencilState(&dssDesc, &m_skyBox.DSLessEqual));
}

void GuineaPig::UpdateScene(double _dt) {

	// Update skybox

	//Reset sphereWorld
	m_skyBox.worldMat = XMMatrixIdentity();

	//Define sphereWorld's world space matrix
	XMMATRIX scale = XMMatrixScaling(5.0f, 5.0f, 5.0f);
	//Make sure the sphere is always centered around camera
	XMMATRIX translation = XMMatrixTranslation(XMVectorGetX(m_camPosition), XMVectorGetY(m_camPosition), XMVectorGetZ(m_camPosition));

	//Set sphereWorld's world space using the transformations
	m_skyBox.worldMat = scale * translation;

	// Update point light position

	//Reset Lights Position
	//XMVECTOR lightVector = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	//lightVector = XMVector3TransformCoord(lightVector, cubeWorldMat * XMMatrixTranslation(0, 5.0f, 0));
	//m_baseLight.position.x = XMVectorGetX(lightVector);
	//m_baseLight.position.y = XMVectorGetY(lightVector);
	//m_baseLight.position.z = XMVectorGetZ(lightVector);

	m_baseLight.position.x = XMVectorGetX(m_camPosition);
	m_baseLight.position.y = XMVectorGetY(m_camPosition);
	m_baseLight.position.z = XMVectorGetZ(m_camPosition);

	m_baseLight.spotLightDir.x = XMVectorGetX(m_camTarget) - m_baseLight.position.x;
	m_baseLight.spotLightDir.y = XMVectorGetY(m_camTarget) - m_baseLight.position.y;
	m_baseLight.spotLightDir.z = XMVectorGetZ(m_camTarget) - m_baseLight.position.z;

	// Update objects
	static float rot = 0.00f;

	rot += (float)_dt;

	m_meshWorld = XMMatrixIdentity();

	XMMATRIX Rotation = XMMatrixRotationY(XM_PI);
	XMMATRIX Scale = XMMatrixScaling(1.0f, 1.0f, 1.0f);
	XMMATRIX Translation = XMMatrixTranslation(0.0f, .1f, 50.0f);

	m_meshWorld = Rotation * Scale * Translation;
}

void GuineaPig::DrawScene() {
	assert(m_d3dImmediateContext);
	assert(m_swapChain);

	//Refresh the render target view
	m_d3dImmediateContext->ClearRenderTargetView(m_renderTargetView, reinterpret_cast<const float*>(&Colors::Black));

	//Refresh the Depth/Stencil view
	m_d3dImmediateContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	UINT stride = sizeof(Vertex3D), offset = 0;

	m_d3dImmediateContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// opaque objects drawing

	// draw skybox
	//Set the default blend state (no blending) for opaque objects
	m_d3dImmediateContext->OMSetBlendState(0, 0, 0xffffffff);


	//"fine-tune" the blending equation
	float blendFactor[] = { 0.75f, 0.75f, 0.75f, 1.0f };

	//Set the default blend state (no blending) for opaque objects
	m_d3dImmediateContext->OMSetBlendState(0, 0, 0xffffffff);

	// apply lighting
	m_cbPerFrame.baseLight = m_baseLight;
	m_d3dImmediateContext->UpdateSubresource(m_cbPerFrameBuffer, 0, NULL, &m_cbPerFrame, 0, 0);
	m_d3dImmediateContext->PSSetConstantBuffers(0, 1, &m_cbPerFrameBuffer);

	// Set the default VS shader and depth/stencil state and layout
	m_d3dImmediateContext->VSSetShader(m_vertexShader, NULL, 0);
	m_d3dImmediateContext->PSSetShader(m_pixelShader, NULL, 0);
	m_d3dImmediateContext->OMSetDepthStencilState(NULL, 0);
	m_d3dImmediateContext->IASetInputLayout(m_inputLayout);

	// Render opaque objects //

	//for (int i = 0; i < m_meshSubsets; i++) {
	//	//Set the grounds index buffer
	//	m_d3dImmediateContext->IASetIndexBuffer(m_meshIndexBuff, DXGI_FORMAT_R32_UINT, 0);
	//	//Set the grounds vertex buffer
	//	m_d3dImmediateContext->IASetVertexBuffers(0, 1, &m_meshVertBuff, &stride, &offset);

	//	//Set the WVP matrix and send it to the constant buffer in effect file
	//	m_cbMeshObject.WVP = XMMatrixTranspose(m_meshWorld * m_camView * m_camProjection);
	//	m_cbMeshObject.World = XMMatrixTranspose(m_meshWorld);
	//	m_cbMeshObject.difColor = m_materials[m_meshSubsetTexture[i]].difColor;
	//	m_cbMeshObject.hasTexture = m_materials[m_meshSubsetTexture[i]].hasTexture;
	//	m_d3dImmediateContext->UpdateSubresource(m_cbMeshBuffer, 0, NULL, &m_cbMeshObject, 0, 0);
	//	m_d3dImmediateContext->VSSetConstantBuffers(0, 1, &m_cbMeshBuffer);
	//	m_d3dImmediateContext->PSSetConstantBuffers(1, 1, &m_cbMeshBuffer);
	//	if (m_materials[m_meshSubsetTexture[i]].hasTexture)
	//		m_d3dImmediateContext->PSSetShaderResources(0, 1, &m_meshShaderResView[m_materials[m_meshSubsetTexture[i]].texArrayIndex]);
	//	m_d3dImmediateContext->PSSetSamplers(0, 1, &m_baseTexSamplerState);

	//	m_d3dImmediateContext->RSSetState(m_antialiasedLine);
	//	int indexStart = m_meshSubsetIndexStart[i];
	//	int indexDrawAmount = m_meshSubsetIndexStart[i + 1] - m_meshSubsetIndexStart[i];
	//	if (!m_materials[m_meshSubsetTexture[i]].transparent)
	//		m_d3dImmediateContext->DrawIndexed(indexDrawAmount, indexStart, 0);
	//}

	//Set the spheres index buffer
	m_d3dImmediateContext->IASetIndexBuffer(m_skyBox.indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	//Set the spheres vertex buffer
	m_d3dImmediateContext->IASetVertexBuffers(0, 1, &m_skyBox.vertBuffer, &stride, &offset);

	//Set the WVP matrix and send it to the constant buffer in effect file
	m_cbGroundObject.WVP = XMMatrixTranspose(m_skyBox.worldMat * m_camView * m_camProjection);
	m_cbGroundObject.World = XMMatrixTranspose(m_skyBox.worldMat);
	m_d3dImmediateContext->UpdateSubresource(m_cbGroundBuffer, 0, NULL, &m_cbGroundObject, 0, 0);
	m_d3dImmediateContext->VSSetConstantBuffers(0, 1, &m_cbGroundBuffer);

	//Send our skymap resource view to pixel shader
	m_d3dImmediateContext->PSSetShaderResources(0, 1, &m_skyBox.shaderResView);
	m_d3dImmediateContext->PSSetSamplers(0, 1, &m_baseTexSamplerState);

	//Set the proper VS and PS shaders, and layout
	m_d3dImmediateContext->VSSetShader(m_skyBox.vertexShader, 0, 0);
	m_d3dImmediateContext->PSSetShader(m_skyBox.pixelShader, 0, 0);
	m_d3dImmediateContext->IASetInputLayout(m_skyBox.inputLayout);
	//Set the new depth/stencil and RS states
	m_d3dImmediateContext->OMSetDepthStencilState(m_skyBox.DSLessEqual, 0);
	m_d3dImmediateContext->RSSetState(m_skyBox.rasterState);

	m_d3dImmediateContext->DrawIndexed(m_skyBox.numFaces * 3, 0, 0);

	//Present the backbuffer to the screen
	HR(m_swapChain->Present(0, 0));
}

void GuineaPig::UpdateKeyboardInput(double _dt) {

	if ( GetAsyncKeyState(VK_LW) ) {
		m_moveBackForward += (float)_dt * 10.0f;
	}

	if ( GetAsyncKeyState(VK_LS) ) {
		m_moveBackForward -= (float)_dt * 10.0f;
	}

	if ( GetAsyncKeyState(VK_LA) ) {
		m_moveLeftRight -= (float)_dt * 10.0f;
	}

	if ( GetAsyncKeyState(VK_LD) ) {
		m_moveLeftRight += (float)_dt * 10.0f;
	}

}

void GuineaPig::UpdateCamera() {
	D3DApp::UpdateCamera();
}

void GuineaPig::OnMouseDown(WPARAM _btnState, int _x, int _y) {
	m_lastMousePos.x = _x;
	m_lastMousePos.y = _y;

	SetCapture(m_hWindow);
}

void GuineaPig::OnMouseUp(WPARAM _btnState, int _x, int _y) {
	ReleaseCapture();
}

void GuineaPig::OnMouseMove(WPARAM _btnState, int _x, int _y) {
	if ( (MK_RBUTTON & _btnState) != 0 ) {

		m_camYaw += 0.01f*(_x - m_lastMousePos.x);
		m_camPitch += 0.01f*(_y - m_lastMousePos.y);

		m_camPitch = D3DUtils::Clamp(m_camPitch, -XM_PIDIV2 + 0.01f, XM_PIDIV2 - 0.01f);
	}

	m_lastMousePos.x = _x;
	m_lastMousePos.y = _y;
}