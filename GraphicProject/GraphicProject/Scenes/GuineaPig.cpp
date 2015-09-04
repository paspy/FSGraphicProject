#include "GuineaPig.h"

GuineaPig::GuineaPig(HINSTANCE hinst) : D3DApp(hinst){ }

GuineaPig::~GuineaPig() {
	SafeRelease(m_cbPerFrameBuffer);

	// release lighting ptr
	SafeRelease(m_perFrameBuffer);
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
	m_objMesh.Init(m_d3dDevice);
	m_barrel.Init(m_d3dDevice);

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
		&m_objMesh.vertBuffer,
		&m_objMesh.indexBuffer,
		m_objMesh.textureNameArray,
		m_objMesh.shaderResView,
		m_objMesh.subsetIndexStart,
		m_objMesh.subsetTexture,
		m_objMesh.materials,
		m_objMesh.subsets,
		true,
		true);

	HR(D3DUtils::CreateShaderAndLayoutFromFile(
		m_d3dDevice,
		L"Shaders/Base/Base.hlsl",
		m_objMesh.vertexLayout, 4, &m_objMesh.vertexShader, &m_objMesh.pixelShader, &m_objMesh.inputLayout));

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

}

void GuineaPig::BuildLighting() {
	// Direction light setting
	m_directionalLight.Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	m_directionalLight.Diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	m_directionalLight.Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_directionalLight.Direction = XMFLOAT3(0.57735f, -0.57735f, 0.57735f);
	//m_directionalLight.Direction = XMFLOAT3(0.0f, -0.57735f, 0.0f);

	//// Point light setting
	//m_pointLight.position = XMFLOAT3(0.0f, 2.0f, 22.5f);
	//m_pointLight.range = 25.0f;
	//m_pointLight.attenuation = XMFLOAT3(0.0f, 0.2f, 0.0f);
	//m_pointLight.diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.2f);

	//// Spot light setting
	//m_spotLight.position = XMFLOAT3(0.0f, 5.0f, 1.0f);
	//m_spotLight.direction = XMFLOAT3(0.0f, -1.0f, 0.0f);
	//m_spotLight.range = 100.0f;
	//m_spotLight.cone = 30.0f;
	//m_spotLight.attenuation = XMFLOAT3(0.4f, 0.02f, 0.0f);
	//m_spotLight.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
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

	// Update spot light position
	//m_spotLight.position.x = XMVectorGetX(m_camPosition);
	//m_spotLight.position.y = XMVectorGetY(m_camPosition);
	//m_spotLight.position.z = XMVectorGetZ(m_camPosition);

	//m_spotLight.direction.x = XMVectorGetX(m_camTarget) - m_spotLight.position.x;
	//m_spotLight.direction.y = XMVectorGetY(m_camTarget) - m_spotLight.position.y;
	//m_spotLight.direction.z = XMVectorGetZ(m_camTarget) - m_spotLight.position.z;

	// Update objects
	static float rot = 0.00f;

	rot += (float)_dt;

	m_objMesh.worldMat = XMMatrixIdentity();

	XMMATRIX Rotation = XMMatrixRotationY(XM_PI);
	XMMATRIX Scale = XMMatrixScaling(1.0f, 1.0f, 1.0f);
	XMMATRIX Translation = XMMatrixTranslation(0.0f, .1f, 0.0f);

	//m_objMesh.worldMat = Rotation * Scale * Translation;


	m_barrel.worldMat = XMMatrixIdentity();

	Rotation = XMMatrixRotationX(XM_PIDIV2);
	Scale = XMMatrixScaling(1.0f, 1.0f, 1.0f);
	Translation = XMMatrixTranslation(0.0f, 1.5f, 25.0f);

	m_barrel.worldMat = Rotation * Scale * Translation;
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
	//m_cbPerFrame.pointLight = m_pointLight;
	//m_cbPerFrame.spotLight = m_spotLight;
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
	m_objMesh.Render(m_d3dImmediateContext, m_camPosition, m_camView, m_camProjection);

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