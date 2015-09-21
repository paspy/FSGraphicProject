#include "GuineaPig.h"
#include "../D3DApp/RenderStates.h"

GuineaPig::GuineaPig(HINSTANCE hinst) : D3DApp(hinst), m_camWalkMode(false){ }

GuineaPig::~GuineaPig() {
	// release lighting ptr
	SafeRelease(m_cbPerFrameBuffer);
	RenderStates::DestroyAll();

}

bool GuineaPig::Init() {
	if ( !D3DApp::Init() )
		return false;
	RenderStates::InitAll(m_d3dDevice);

	BuildConstBuffer();
	BuildGeometry();
	BuildLighting();

	return true;
}

void GuineaPig::OnResize() {
	D3DApp::OnResize();
	// reset camera
	m_camera.SetLens(0.4f*XM_PI, AspectRatio(), 1.0f, 3000.0f);
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
	m_barrel.Init(m_d3dDevice, m_swapChain, L"Resources/Models/barrel.obj", true, true, L"Shaders/Base/Base.hlsl");
	
	//m_terrain.Init(m_d3dDevice, L"Shaders/Base/Base.hlsl");
	//m_wave.Init(m_d3dDevice, L"Shaders/Base/Base.hlsl");

	m_geoMesh.Init(m_d3dDevice, L"Shaders/Base/InstancedBase.hlsl", GeoMesh::GeoType::Box, L"Resources/Textures/WireFence_diffuse.dds", L"Resources/Textures/WireFence_normal.dds");

	m_mirrorMesh.Init(m_d3dDevice, L"Shaders/Base/Base.hlsl");

	//m_quadMesh.Init(m_d3dDevice, L"Shaders/Base/InstancedBase.hlsl", GeoMesh::GeoType::Grid, L"Resources/Textures/WireFence_diffuse.dds", L"Resources/Textures/WireFence_normal.dds");

	m_heighMapTerrain.Init(m_d3dDevice, m_d3dImmediateContext);
	
	D3DUtils::CreateModelFromObjFileKaiNi(NULL, NULL, "Resources/Models/barrel.obj", NULL, NULL);
}

void GuineaPig::BuildLighting() {
	// Direction light setting
	m_directionalLight.Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	m_directionalLight.Diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
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
	XMMATRIX translation = XMMatrixTranslation(XMVectorGetX(m_camera.GetPosition()), XMVectorGetY(m_camera.GetPosition()), XMVectorGetZ(m_camera.GetPosition()));

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
	m_spotLight.Position.x = XMVectorGetX(m_camera.GetPosition());
	m_spotLight.Position.y = XMVectorGetY(m_camera.GetPosition());
	m_spotLight.Position.z = XMVectorGetZ(m_camera.GetPosition());
	XMStoreFloat3(&m_spotLight.Direction, XMVector3Normalize(m_camera.GetLook() - m_camera.GetPosition()));
	// **Update Spot Light **//


	// Update objects
	XMMATRIX Rotation, Scale, Translation;

	// barrel update
	m_barrel.worldMat = XMMatrixIdentity();
	Rotation = XMMatrixRotationY(0);
	Scale = XMMatrixScaling(5.0f, 5.0f, 5.0f);
	Translation = XMMatrixTranslation(0.0f, 5.0f, 25.0f);
	m_barrel.worldMat = Rotation * Scale * Translation;

	// m_geoMesh update
	m_geoMesh.worldMat = XMMatrixIdentity();
	Rotation = XMMatrixRotationY(0);
	Scale = XMMatrixScaling(5.0f, 5.0f, 5.0f);
	Translation = XMMatrixTranslation(0.0f, 20.0f, 0.0f);
	m_geoMesh.worldMat = Rotation * Scale * Translation;

	// terrain update
	m_terrain.worldMat = XMMatrixIdentity();
	Rotation = XMMatrixRotationY(0);
	Scale = XMMatrixScaling(2.0f, 1.0f, 2.0f);
	Translation = XMMatrixTranslation(0.0f, 0.0f, 0.0f);
	m_terrain.worldMat = Rotation * Scale * Translation;

	// update waves
	//m_wave.Update(_dt, m_timer.TotalTime(), m_d3dImmediateContext);
	//m_geoMesh.Update(m_d3dImmediateContext, m_camera);

	m_mirrorMesh.Update();

	m_heighMapTerrain.Update();
}

void GuineaPig::DrawScene() {
	assert(m_d3dImmediateContext);
	assert(m_swapChain);

	//Refresh the render target view
	m_d3dImmediateContext->ClearRenderTargetView(m_renderTargetView, reinterpret_cast<const float*>(&Colors::Black));

	//Refresh the Depth/Stencil view
	m_d3dImmediateContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	m_d3dImmediateContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	float blendFactor1[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	float blendFactor2[] = { 0.7f, 0.7f, 0.7f, 1.0f };

	// apply lighting
	m_cbPerFrame.directionalLight = m_directionalLight;
	m_cbPerFrame.pointLight = m_pointLight;
	m_cbPerFrame.spotLight = m_spotLight;
	XMFLOAT4 curCamPos;
	XMStoreFloat4(&curCamPos, m_camera.GetPosition());
	m_cbPerFrame.cameraPos = curCamPos;

	m_d3dImmediateContext->UpdateSubresource(m_cbPerFrameBuffer, 0, NULL, &m_cbPerFrame, 0, 0);
	m_d3dImmediateContext->PSSetConstantBuffers(0, 1, &m_cbPerFrameBuffer);

	// Bind the render target view and depth/stencil view to the pipeline.
	m_d3dImmediateContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);

	// Render opaque objects //


	// Skybox
	m_skyBox.Render(m_d3dImmediateContext, m_camera, RenderStates::NoCullRS);
	m_barrel.Render	(m_d3dImmediateContext, m_camera, RenderStates::NoCullRS);

	m_heighMapTerrain.Render(m_d3dImmediateContext, m_camera, m_directionalLight);
	// obj meshs
	//m_terrain.Render(m_d3dImmediateContext, m_camera, 0);


	//m_wave.Render(m_d3dImmediateContext, m_camera, 0, RenderStates::TransparentBSbyColor, blendFactor1);
	
	//m_geoMesh.Render(m_d3dImmediateContext, m_camera, RenderStates::TransparentBSbyColor, blendFactor1);

	//m_mirrorMesh.Render(m_d3dImmediateContext, m_camera, NULL);

	//Set the default blend state (no blending) for opaque objects

	m_d3dImmediateContext->OMSetBlendState(0, blendFactor1, 0xffffffff);
	//Present the backbuffer to the screen
	HR(m_swapChain->Present(0, 0));
}

void GuineaPig::UpdateKeyboardInput(double _dt) {

	if (GetAsyncKeyState('W') & 0x8000) {
		m_camera.Walk(CAMERA_SPEED*static_cast<float>(_dt));
	}

	if (GetAsyncKeyState('S') & 0x8000) {
		m_camera.Walk(-CAMERA_SPEED*static_cast<float>(_dt));
	}

	if (GetAsyncKeyState('A') & 0x8000) {
		m_camera.Strafe(-CAMERA_SPEED*static_cast<float>(_dt));
	}

	if (GetAsyncKeyState('D') & 0x8000) {
		m_camera.Strafe(CAMERA_SPEED*static_cast<float>(_dt));
	}

	if (GetAsyncKeyState('M') & 0x8000) m_camWalkMode = !m_camWalkMode;

	if (m_camWalkMode) {
		//XMFLOAT3 camPos = m_camera.GetPosition();
		//float y = mTerrain.GetHeight(camPos.x, camPos.z);
		//mCam.SetPosition(camPos.x, y + 2.0f, camPos.z);
	}
	m_camera.UpdateViewMatrix();
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

		float dx = XMConvertToRadians(0.25f*static_cast<float>(_x - m_lastMousePos.x));
		float dy = XMConvertToRadians(0.25f*static_cast<float>(_y - m_lastMousePos.y));

		m_camera.RotateY(dx);
		m_camera.Pitch(dy);
	}

	m_lastMousePos.x = _x;
	m_lastMousePos.y = _y;
}