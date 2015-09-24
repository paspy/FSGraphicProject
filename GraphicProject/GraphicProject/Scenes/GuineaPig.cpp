#include "GuineaPig.h"
#include "../D3DApp/RenderStates.h"
#include "../D3DApp/GeoGen.h"


GuineaPig::GuineaPig(HINSTANCE hinst) : D3DApp(hinst), m_camWalkMode(true), m_enableBlur(false) { }

GuineaPig::~GuineaPig() {
	// release lighting ptr
	SafeRelease(m_osVertexShader);
	SafeRelease(m_osPixelShader);
	SafeRelease(m_osInputLayout);
	SafeRelease(m_cbPerFrameBuffer);
	// Off screen
	SafeRelease(m_screenQuadVB);
	SafeRelease(m_screenQuadIB);
	SafeRelease(m_offscreenSRV);
	SafeRelease(m_constOffScreen);
	SafeRelease(m_offscreenUAV);
	SafeRelease(m_offscreenRTV);
	RenderStates::DestroyAll();

}

bool GuineaPig::Init() {
	if ( !D3DApp::Init() )
		return false;
	RenderStates::InitAll(m_d3dDevice);

	BuildConstBuffer();
	BuildGeometry();
	BuildLighting();
	BuildOffscreenViews();
	BuildScreenQuadGeometryBuffers();

	m_blur.Init(m_d3dDevice, m_clientWidth, m_clientHeight, DXGI_FORMAT_R8G8B8A8_UNORM, m_enable4xMsaa);
	MessageBox(0, L"WSAD for moving\nF for Fog\nL for wireframe\nM for switch moving and flying\nB for fullscreen Blur", L"Tips", MB_OK);
	return true;
}

void GuineaPig::OnResize() {
	D3DApp::OnResize();
	BuildOffscreenViews();
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
	
	//m_terrain.Init(m_d3dDevice, L"Shaders/Base/Base.hlsl");
	m_wave.Init(m_d3dDevice, L"Shaders/Base/Base.hlsl");

	m_geoMesh.Init(m_d3dDevice, L"Shaders/Base/InstancedBase.hlsl", GeoMesh::GeoType::Box, L"Resources/Textures/WireFence_diffuse.dds", L"Resources/Textures/WireFence_normal.dds");

	m_mirrorMesh.Init(m_d3dDevice, L"Shaders/Base/Base.hlsl");

	m_heighMapTerrain.Init(m_d3dDevice, m_d3dImmediateContext);

	vector<wstring> raindrops;
	raindrops.push_back(L"Resources/Particles/raindrop.dds");
	m_rain.Init(m_d3dDevice, m_d3dImmediateContext, 10000, L"Shaders/Particle/BaseRain.hlsl", raindrops);

	vector<wstring> fiare;
	fiare.push_back(L"Resources/Particles/flare.dds");
	m_fire.Init(m_d3dDevice, m_d3dImmediateContext, 500, L"Shaders/Particle/BaseFire.hlsl", fiare);
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

void GuineaPig::BuildOffscreenViews() {
	SafeRelease(m_offscreenSRV);
	SafeRelease(m_offscreenRTV);
	SafeRelease(m_offscreenUAV);

	D3D11_TEXTURE2D_DESC texDesc;

	texDesc.Width = m_clientWidth;
	texDesc.Height = m_clientHeight;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	if (m_enable4xMsaa) {
		texDesc.SampleDesc.Count = 4;
		texDesc.SampleDesc.Quality = m_4xMsaaQuality - 1;
	} else {
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
	}
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	ID3D11Texture2D* offscreenTex = 0;
	HR(m_d3dDevice->CreateTexture2D(&texDesc, 0, &offscreenTex));

	// Null description means to create a view to all mipmap levels using 
	// the format the texture was created with.
	HR(m_d3dDevice->CreateShaderResourceView(offscreenTex, 0,	&m_offscreenSRV));
	HR(m_d3dDevice->CreateRenderTargetView(offscreenTex, 0,		&m_offscreenRTV));
	HR(m_d3dDevice->CreateUnorderedAccessView(offscreenTex, 0,	&m_offscreenUAV));

	// View saves a reference to the texture so we can release our reference.
	SafeRelease(offscreenTex);
}

void GuineaPig::BuildScreenQuadGeometryBuffers() {

	D3D11_INPUT_ELEMENT_DESC vertexLayout[4] =
	{
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT",	0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	HR(D3DUtils::CreateShaderAndLayoutFromFile(m_d3dDevice, L"Shaders/Base/Base.hlsl", vertexLayout, 4, &m_osVertexShader, &m_osPixelShader, &m_osInputLayout));

	D3D11_BUFFER_DESC cbbd;
	ZeroMemory(&cbbd, sizeof(D3D11_BUFFER_DESC));
	cbbd.Usage = D3D11_USAGE_DEFAULT;
	cbbd.ByteWidth = sizeof(cbOffScreen);
	cbbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbbd.CPUAccessFlags = 0;
	cbbd.MiscFlags = 0;
	HR(m_d3dDevice->CreateBuffer(&cbbd, NULL, &m_constOffScreen));

	GeoGen::MeshData quad;
	GeoGen::CreateFullscreenQuad(quad);
	vector<Vertex3D> vertices(quad.Vertices.size());

	for ( UINT i = 0; i < quad.Vertices.size(); ++i ) {
		vertices[i].Position = quad.Vertices[i].Position;
		vertices[i].Normal = quad.Vertices[i].Normal;
		vertices[i].TexCoord = quad.Vertices[i].TexCoord;
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex3D) * (UINT)quad.Vertices.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = vertices.data();
	HR(m_d3dDevice->CreateBuffer(&vbd, &vinitData, &m_screenQuadVB));

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * (UINT)quad.Indices.size();
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = quad.Indices.data();
	HR(m_d3dDevice->CreateBuffer(&ibd, &iinitData, &m_screenQuadIB));
}

void GuineaPig::DrawScreenQuad() {
	m_d3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_d3dImmediateContext->VSSetShader(m_osVertexShader, NULL, 0);
	m_d3dImmediateContext->PSSetShader(m_osPixelShader, NULL, 0);
	m_d3dImmediateContext->IASetInputLayout(m_osInputLayout);
	UINT stride = sizeof(Vertex3D);
	UINT offset = 0;

	m_d3dImmediateContext->IASetVertexBuffers(0, 1, &m_screenQuadVB, &stride, &offset);
	m_d3dImmediateContext->IASetIndexBuffer(m_screenQuadIB, DXGI_FORMAT_R32_UINT, 0);
	m_cbOffScreen.World = XMMatrixIdentity() /** XMMatrixScaling(0.5f, 0.5f, 0.5f)*/;
	m_cbOffScreen.WorldInvTranspose = XMMatrixIdentity();
	m_cbOffScreen.WorldViewProj = XMMatrixIdentity();
	m_cbOffScreen.TexTransform = XMMatrixIdentity();
	ID3D11ShaderResourceView* SRV = m_blur.GetBlurredOutput();
	m_d3dImmediateContext->UpdateSubresource(m_constOffScreen, 0, NULL, &m_cbOffScreen, 0, 0);
	m_d3dImmediateContext->VSSetConstantBuffers(1, 1, &m_constOffScreen);

	m_d3dImmediateContext->PSSetShaderResources(0, 1, &SRV);

	m_d3dImmediateContext->DrawIndexed(6, 0, 0);
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
	/// **Update Skybox **//

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

	// terrain update
	m_terrain.worldMat = XMMatrixIdentity();
	Rotation = XMMatrixRotationY(0);
	Scale = XMMatrixScaling(2.0f, 1.0f, 2.0f);
	Translation = XMMatrixTranslation(0.0f, 0.0f, 0.0f);
	m_terrain.worldMat = Rotation * Scale * Translation;

	// update waves
	m_wave.Update(_dt, m_timer.TotalTime(), m_d3dImmediateContext);
	m_geoMesh.Update(m_d3dImmediateContext, m_camera);

	m_mirrorMesh.Update();

	m_heighMapTerrain.Update();

	m_rain.Update(static_cast<float>(_dt), static_cast<float>(m_timer.TotalTime()));
	m_fire.Update(static_cast<float>(_dt), static_cast<float>(m_timer.TotalTime()));

}

void GuineaPig::DrawScene() {
	assert(m_d3dImmediateContext);
	assert(m_swapChain);

	if (m_enableBlur) {
		m_d3dImmediateContext->OMSetRenderTargets(1, &m_offscreenRTV, m_depthStencilView);
		m_d3dImmediateContext->ClearRenderTargetView(m_offscreenRTV, reinterpret_cast<const float*>(&Colors::Silver));
		m_d3dImmediateContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		DrawForHold();

		m_d3dImmediateContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);

		m_blur.BlurInPlace(m_d3dImmediateContext, m_offscreenSRV, m_offscreenUAV, 4);

		m_d3dImmediateContext->ClearRenderTargetView(m_renderTargetView, reinterpret_cast<const float*>(&Colors::Silver));
		m_d3dImmediateContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		m_d3dImmediateContext->PSSetConstantBuffers(0, 1, &m_cbPerFrameBuffer);
		DrawScreenQuad();
	} else {
		m_d3dImmediateContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);
		m_d3dImmediateContext->ClearRenderTargetView(m_renderTargetView, reinterpret_cast<const float*>(&Colors::Silver));
		m_d3dImmediateContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		DrawForHold();

	}

	//Present the backbuffer to the screen
	HR(m_swapChain->Present(0, 0));
}

void GuineaPig::DrawForHold() {
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

	// Skybox
	m_skyBox.Render(m_d3dImmediateContext, m_camera, RenderStates::NoCullRS);
	m_heighMapTerrain.Render(m_d3dImmediateContext, m_camera, m_directionalLight);

	m_d3dImmediateContext->PSSetConstantBuffers(0, 1, &m_cbPerFrameBuffer);
	m_geoMesh.Render(m_d3dImmediateContext, m_camera, RenderStates::TransparentBSbyColor, blendFactor1);

	m_rain.SetEmitPos(XMFLOAT3(curCamPos.x, curCamPos.y, curCamPos.z));
	m_rain.Render(m_d3dImmediateContext, m_camera);
	//
	m_fire.SetEmitPos(XMFLOAT3(100.0f, 0.5f, 40.0f));
	m_fire.Render(m_d3dImmediateContext, m_camera, RenderStates::AdditiveBlending, blendFactor1);
	//Set the default blend state (no blending) for opaque objects
	m_d3dImmediateContext->OMSetBlendState(0, blendFactor1, 0xffffffff);
}

void GuineaPig::UpdateKeyboardInput(double _dt) {

	if (GetAsyncKeyState('W')) {
		m_camera.Walk(CAMERA_SPEED*static_cast<float>(_dt));
	}

	if (GetAsyncKeyState('S')) {
		m_camera.Walk(-CAMERA_SPEED*static_cast<float>(_dt));
	}

	if (GetAsyncKeyState('A')) {
		m_camera.Strafe(-CAMERA_SPEED*static_cast<float>(_dt));
	}

	if (GetAsyncKeyState('D')) {
		m_camera.Strafe(CAMERA_SPEED*static_cast<float>(_dt));
	}

	if (GetAsyncKeyState('B') & 0x1)
		m_enableBlur = !m_enableBlur;

	if (GetAsyncKeyState('M') & 0x1 )
		m_camWalkMode = !m_camWalkMode;


	if (m_camWalkMode) {
		XMFLOAT4 camPos;
		XMStoreFloat4(&camPos, m_camera.GetPosition());
		float y = m_heighMapTerrain.GetHeight(camPos.x, camPos.z);
		m_camera.SetPosition(camPos.x, y + 5.0f, camPos.z, camPos.w);
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