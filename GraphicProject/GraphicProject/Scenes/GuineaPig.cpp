#include "GuineaPig.h"

GuineaPig::GuineaPig(HINSTANCE hinst) : D3DApp(hinst){ }

GuineaPig::~GuineaPig() {
	// release lighting ptr
	SafeRelease(m_cbPerFrameBuffer);

}

bool GuineaPig::Init() {
	if ( !D3DApp::Init() )
		return false;

	BuildConstBuffer();
	BuildGeometry();
	BuildLighting();

	return true;
}

void GuineaPig::OnResize() {
	D3DApp::OnResize();

}

void GuineaPig::BuildConstBuffer() {
	// lightings
	D3D11_BUFFER_DESC cbbd;
	ZeroMemory(&cbbd, sizeof(D3D11_BUFFER_DESC));
	cbbd.Usage = D3D11_USAGE_DEFAULT;
	cbbd.ByteWidth = sizeof(cbPerFrame);
	cbbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbbd.CPUAccessFlags = 0;
	cbbd.MiscFlags = 0;
	HR(m_d3dDevice->CreateBuffer(&cbbd, NULL, &m_cbPerFrameBuffer));

}

void GuineaPig::BuildGeometry() {
	m_skyBox.Init(m_d3dDevice);
	m_ground.Init(m_d3dDevice);
	m_barrel.Init(m_d3dDevice);
	m_bed.Init(m_d3dDevice);

	D3DUtils::BuildSphere(m_d3dDevice ,10, 10, &m_skyBox.vertBuffer, &m_skyBox.indexBuffer, m_skyBox.numVertices, m_skyBox.numFaces);
	// loading the texture - using dds loader
	HR(CreateDDSTextureFromFile(m_d3dDevice, L"Resources/Skybox/skymap.dds", NULL, &m_skyBox.shaderResView));
	// create the depending shader
	HR(D3DUtils::CreateShaderAndLayoutFromFile(
		m_d3dDevice,
		L"Shaders/Skybox/Skybox.hlsl",
		m_skyBox.vertexLayout, 2, &m_skyBox.vertexShader, &m_skyBox.pixelShader, &m_skyBox.inputLayout));


	D3DUtils::CreateModelFromObjFile(
		m_d3dDevice,
		m_swapChain,
		L"Resources/Models/ground.obj",
		&m_ground.vertBuffer,
		&m_ground.indexBuffer,
		m_ground.textureNameArray,
		m_ground.shaderResView,
		m_ground.subsetIndexStart,
		m_ground.subsetTexture,
		m_ground.materials,
		m_ground.subsets,
		true,
		true);

	HR(D3DUtils::CreateShaderAndLayoutFromFile(
		m_d3dDevice,
		L"Shaders/Base/Base.hlsl",
		m_ground.vertexLayout, 4, &m_ground.vertexShader, &m_ground.pixelShader, &m_ground.inputLayout));

	D3DUtils::CreateModelFromObjFile(
		m_d3dDevice,
		m_swapChain,
		L"Resources/Models/barrel.obj",
		&m_barrel.vertBuffer,
		&m_barrel.indexBuffer,
		m_barrel.textureNameArray,
		m_barrel.shaderResView,
		m_barrel.subsetIndexStart,
		m_barrel.subsetTexture,
		m_barrel.materials,
		m_barrel.subsets,
		true,
		true);

	HR(D3DUtils::CreateShaderAndLayoutFromFile(
		m_d3dDevice,
		L"Shaders/Base/Base.hlsl",
		m_barrel.vertexLayout, 4, &m_barrel.vertexShader, &m_barrel.pixelShader, &m_barrel.inputLayout));

	D3DUtils::CreateModelFromObjFile(
		m_d3dDevice,
		m_swapChain,
		L"Resources/Models/Bed.obj",
		&m_bed.vertBuffer,
		&m_bed.indexBuffer,
		m_bed.textureNameArray,
		m_bed.shaderResView,
		m_bed.subsetIndexStart,
		m_bed.subsetTexture,
		m_bed.materials,
		m_bed.subsets,
		true,
		true);

	HR(D3DUtils::CreateShaderAndLayoutFromFile(
		m_d3dDevice,
		L"Shaders/Base/Base.hlsl",
		m_bed.vertexLayout, 4, &m_bed.vertexShader, &m_bed.pixelShader, &m_bed.inputLayout));

	D3DUtils::CreateModelFromObjFileKaiNi(NULL, NULL, "Resources/Models/barrel.obj", NULL, NULL);
}

void GuineaPig::BuildLighting() {
	// Direction light setting
	m_directionalLight.Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	m_directionalLight.Diffuse = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	m_directionalLight.Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	m_directionalLight.Direction = XMFLOAT3(0.58f, -0.58f, 0.58f);

	// Point light setting
	m_pointLight.Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	m_pointLight.Diffuse = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
	m_pointLight.Specular = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
	m_pointLight.Att = XMFLOAT3(0.0f, 0.2f, 0.0f);
	m_pointLight.Range = 25.0f;
	// position change in update scence

	// Spot light setting
	m_spotLight.Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	m_spotLight.Diffuse = XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);
	m_spotLight.Specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_spotLight.Att = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_spotLight.Spot = 100.0f;
	m_spotLight.Range = 1000.0f;

}

void GuineaPig::UpdateScene(double _dt) {

	// **Update Skybox **//
	//Reset sphereWorld
	m_skyBox.worldMat = XMMatrixIdentity();

	//Define sphereWorld's world space matrix
	XMMATRIX scale = XMMatrixScaling(5.0f, 5.0f, 5.0f);
	//Make sure the sphere is always centered around camera
	XMMATRIX translation = XMMatrixTranslation(XMVectorGetX(m_camPosition), XMVectorGetY(m_camPosition), XMVectorGetZ(m_camPosition));

	//Set sphereWorld's world space using the transformations
	m_skyBox.worldMat = scale * translation;
	// **Update Skybox **//

	// **Update Directional Light **//
	m_directionalLight.Direction.x = 0.85f * cosf(static_cast<float>(m_timer.TotalTime()));
	m_directionalLight.Direction.z = 0.85f * sinf(static_cast<float>(m_timer.TotalTime()));
	m_directionalLight.Direction.y = -0.58f;
	// **Update Directional Light **//


	// **Update Point Light **//
	m_pointLight.Position.x = 5.0f*cosf(2.2f * static_cast<float>(m_timer.TotalTime()));
	m_pointLight.Position.z = 5.0f*sinf(2.2f * static_cast<float>(m_timer.TotalTime()));
	m_pointLight.Position.y = 2.5f;
	// **Update Point Light **//

	// **Update Spot Light **//
	m_spotLight.Position.x = XMVectorGetX(m_camPosition);
	m_spotLight.Position.y = XMVectorGetY(m_camPosition);
	m_spotLight.Position.z = XMVectorGetZ(m_camPosition);
	XMStoreFloat3(&m_spotLight.Direction, XMVector3Normalize(m_camTarget - m_camPosition));
	// **Update Spot Light **//


	// Update objects
	static float rot = 0.00f;

	rot += (float)_dt;

	m_ground.worldMat = XMMatrixIdentity();

	XMMATRIX Rotation = XMMatrixRotationY(XM_PI);
	XMMATRIX Scale = XMMatrixScaling(5.0f, 1.0f, 5.0f);
	XMMATRIX Translation = XMMatrixTranslation(0.0f, .1f, 0.0f);

	m_ground.worldMat = Rotation * Scale * Translation;

	m_barrel.worldMat = XMMatrixIdentity();

	Rotation = XMMatrixRotationY(0);
	Scale = XMMatrixScaling(5.0f, 5.0f, 5.0f);
	Translation = XMMatrixTranslation(0.0f, 5.0f, 25.0f);

	m_barrel.worldMat = Rotation * Scale * Translation;

	m_bed.worldMat = XMMatrixIdentity();

	Rotation = XMMatrixRotationY(0);
	Scale = XMMatrixScaling(1.0f, 1.0f, 1.0f);
	Translation = XMMatrixTranslation(0.0f, 1.0f, -30.0f);

	m_bed.worldMat = Rotation * Scale * Translation;
}

void GuineaPig::DrawScene() {
	assert(m_d3dImmediateContext);
	assert(m_swapChain);

	//Refresh the render target view
	m_d3dImmediateContext->ClearRenderTargetView(m_renderTargetView, reinterpret_cast<const float*>(&Colors::Black));

	//Refresh the Depth/Stencil view
	m_d3dImmediateContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	m_d3dImmediateContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// opaque objects drawing

	// apply lighting
	m_cbPerFrame.directionalLight = m_directionalLight;
	m_cbPerFrame.pointLight = m_pointLight;
	m_cbPerFrame.spotLight = m_spotLight;
	XMFLOAT4 curCamPos;
	XMStoreFloat4(&curCamPos, m_camPosition);
	m_cbPerFrame.cameraPos = curCamPos;

	m_d3dImmediateContext->UpdateSubresource(m_cbPerFrameBuffer, 0, NULL, &m_cbPerFrame, 0, 0);
	m_d3dImmediateContext->PSSetConstantBuffers(0, 1, &m_cbPerFrameBuffer);

	// Bind the render target view and depth/stencil view to the pipeline.
	m_d3dImmediateContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);

	//Set the default blend state (no blending) for opaque objects
	m_d3dImmediateContext->OMSetBlendState(0, 0, 0xffffffff);

	// Render opaque objects //

	// obj meshs
	m_barrel.Render(m_d3dImmediateContext, m_camPosition, m_camView, m_camProjection);
	m_bed.Render(m_d3dImmediateContext, m_camPosition, m_camView, m_camProjection);
	m_ground.Render(m_d3dImmediateContext, m_camPosition, m_camView, m_camProjection);

	// Skybox
	m_skyBox.Render(m_d3dImmediateContext, m_camView, m_camProjection);

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