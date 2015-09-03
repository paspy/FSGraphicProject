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
	cbbd.ByteWidth = sizeof(ConstPerFrame);
	cbbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbbd.CPUAccessFlags = 0;
	cbbd.MiscFlags = 0;
	HR(m_d3dDevice->CreateBuffer(&cbbd, NULL, &m_cbPerFrameBuffer));

}

void GuineaPig::BuildGeometry() {
	m_skyBox.Init(m_d3dDevice);
	m_objMesh.Init(m_d3dDevice);
	m_building.Init(m_d3dDevice);
	m_barrel.Init(m_d3dDevice);

	D3DUtils::BuildSphere(m_d3dDevice ,10, 10, &m_skyBox.vertBuffer, &m_skyBox.indexBuffer, m_skyBox.numVertices, m_skyBox.numFaces);
	// loading the texture - using dds loader
	HR(CreateDDSTextureFromFile(m_d3dDevice, L"Resources/Skybox/skymap.dds", NULL, &m_skyBox.shaderResView));
	// create the depending shader
	HR(D3DUtils::CreateShaderAndLayoutFromFile(
		m_d3dDevice,
		L"Shaders/Skybox/Skybox.hlsl",
		m_skyBox.vertexLayout, 2, &m_skyBox.vertexShader, &m_skyBox.pixelShader, &m_skyBox.inputLayout));

	vector<Vertex3D> verts;
	D3DUtils::loadOBJ("Resources/Models/ground.obj", verts);
	verts;


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
		L"Resources/Models/spaceCompound.obj",
		&m_building.vertBuffer,
		&m_building.indexBuffer,
		m_building.textureNameArray,
		m_building.shaderResView,
		m_building.subsetIndexStart,
		m_building.subsetTexture,
		m_building.materials,
		m_building.subsets,
		true,
		true);


	HR(D3DUtils::CreateShaderAndLayoutFromFile(
		m_d3dDevice,
		L"Shaders/Base/Base.hlsl",
		m_building.vertexLayout, 4, &m_building.vertexShader, &m_building.pixelShader, &m_building.inputLayout));

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
	m_baseLight.direction = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_baseLight.ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	m_baseLight.diffuse = XMFLOAT4(1.2f, 1.2f, 1.2f, 1.2f);

	// Point light setting
	//m_baseLight.position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	//m_baseLight.range = 50.0f;
	//m_baseLight.attenuation = XMFLOAT3(0.0f, 0.1f, 0.0f);
	//m_baseLight.ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	//m_baseLight.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	// Point light setting
	//m_baseLight.position = XMFLOAT3(0.0f, 1.0f, 0.0f);
	//m_baseLight.spotLightDir = XMFLOAT3(0.0f, 0.0f, 1.0f);
	//m_baseLight.range = 1000.0f;
	//m_baseLight.cone = 20.0f;
	//m_baseLight.attenuation = XMFLOAT3(0.4f, 0.02f, 0.0f);
	//m_baseLight.ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	//m_baseLight.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
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

	//m_baseLight.position.x = XMVectorGetX(m_camPosition);
	//m_baseLight.position.y = XMVectorGetY(m_camPosition);
	//m_baseLight.position.z = XMVectorGetZ(m_camPosition);

	//m_baseLight.spotLightDir.x = XMVectorGetX(m_camTarget) - m_baseLight.position.x;
	//m_baseLight.spotLightDir.y = XMVectorGetY(m_camTarget) - m_baseLight.position.y;
	//m_baseLight.spotLightDir.z = XMVectorGetZ(m_camTarget) - m_baseLight.position.z;

	// Update objects
	static float rot = 0.00f;

	rot += (float)_dt;

	m_objMesh.worldMat = XMMatrixIdentity();

	XMMATRIX Rotation = XMMatrixRotationY(XM_PI);
	XMMATRIX Scale = XMMatrixScaling(1.0f, 1.0f, 1.0f);
	XMMATRIX Translation = XMMatrixTranslation(0.0f, .1f, 0.0f);

	m_objMesh.worldMat = Rotation * Scale * Translation;

	m_building.worldMat = XMMatrixIdentity();

	 Rotation = XMMatrixRotationY(XM_PI);
	 Scale = XMMatrixScaling(1.0f, 1.0f, 1.0f);
	 Translation = XMMatrixTranslation(0.0f, 0.11f, 50.0f);

	m_building.worldMat = Rotation * Scale * Translation;

	m_barrel.worldMat = XMMatrixIdentity();

	Rotation = XMMatrixRotationY(XM_PI);
	Scale = XMMatrixScaling(1.0f, 1.0f, 1.0f);
	Translation = XMMatrixTranslation(0.0f, 0.11f, -50.0f);

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
	m_cbPerFrame.baseLight = m_baseLight;
	m_d3dImmediateContext->UpdateSubresource(m_cbPerFrameBuffer, 0, NULL, &m_cbPerFrame, 0, 0);
	m_d3dImmediateContext->PSSetConstantBuffers(0, 1, &m_cbPerFrameBuffer);

	// Bind the render target view and depth/stencil view to the pipeline.
	m_d3dImmediateContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);

	//Set the default blend state (no blending) for opaque objects
	m_d3dImmediateContext->OMSetBlendState(0, 0, 0xffffffff);

	// Render opaque objects //

	// Set the default VS shader and depth/stencil state and layout
	m_d3dImmediateContext->VSSetShader(m_objMesh.vertexShader, NULL, 0);
	m_d3dImmediateContext->PSSetShader(m_objMesh.pixelShader, NULL, 0);
	m_d3dImmediateContext->IASetInputLayout(m_objMesh.inputLayout);
	m_d3dImmediateContext->OMSetDepthStencilState(NULL, 0);

	for (int i = 0; i < m_objMesh.subsets; i++) {
		//Set the grounds index buffer
		m_d3dImmediateContext->IASetIndexBuffer(m_objMesh.indexBuffer, DXGI_FORMAT_R32_UINT, 0);
		//Set the grounds vertex buffer
		m_d3dImmediateContext->IASetVertexBuffers(0, 1, &m_objMesh.vertBuffer, &m_objMesh.stride, &m_objMesh.offset);

		//Set the WVP matrix and send it to the constant buffer in effect file
		m_objMesh.cbBuffer.WVP = XMMatrixTranspose(m_objMesh.worldMat * m_camView * m_camProjection);
		m_objMesh.cbBuffer.World = XMMatrixTranspose(m_objMesh.worldMat);
		m_objMesh.cbBuffer.difColor = m_objMesh.materials[m_objMesh.subsetTexture[i]].difColor;
		m_objMesh.cbBuffer.hasTexture = m_objMesh.materials[m_objMesh.subsetTexture[i]].hasTexture;
		m_objMesh.cbBuffer.hasNormal = m_objMesh.materials[m_objMesh.subsetTexture[i]].hasNormMap;
		m_d3dImmediateContext->UpdateSubresource(m_objMesh.constBuffer, 0, NULL, &m_objMesh.cbBuffer, 0, 0);
		m_d3dImmediateContext->VSSetConstantBuffers(0, 1, &m_objMesh.constBuffer);
		m_d3dImmediateContext->PSSetConstantBuffers(1, 1, &m_objMesh.constBuffer);
		//if ( m_objMesh.materials[m_objMesh.subsetTexture[i]].hasTexture > 0.5f)
			m_d3dImmediateContext->PSSetShaderResources(0, 1, &m_objMesh.shaderResView[m_objMesh.materials[m_objMesh.subsetTexture[i]].texArrayIndex]);
		//if ( m_objMesh.materials[m_objMesh.subsetTexture[i]].hasNormMap > 0.5f)
			m_d3dImmediateContext->PSSetShaderResources(1, 1, &m_objMesh.shaderResView[m_objMesh.materials[m_objMesh.subsetTexture[i]].normMapTexArrayIndex]);
		m_d3dImmediateContext->PSSetSamplers(0, 1, &m_objMesh.texSamplerState);

		m_d3dImmediateContext->RSSetState(m_objMesh.rasterState);
		int indexStart = m_objMesh.subsetIndexStart[i];
		int indexDrawAmount = m_objMesh.subsetIndexStart[i + 1] - m_objMesh.subsetIndexStart[i];
		//if (m_objMesh.materials[m_objMesh.subsetIndexStart[i]].transparent < 0.5f)
			m_d3dImmediateContext->DrawIndexed(indexDrawAmount, indexStart, 0);
	}

	m_d3dImmediateContext->VSSetShader(m_building.vertexShader, NULL, 0);
	m_d3dImmediateContext->PSSetShader(m_building.pixelShader, NULL, 0);
	m_d3dImmediateContext->IASetInputLayout(m_building.inputLayout);
	m_d3dImmediateContext->OMSetDepthStencilState(NULL, 0);

	for ( int i = 0; i < m_building.subsets; i++ ) {
		//Set the grounds index buffer
		m_d3dImmediateContext->IASetIndexBuffer(m_building.indexBuffer, DXGI_FORMAT_R32_UINT, 0);
		//Set the grounds vertex buffer
		m_d3dImmediateContext->IASetVertexBuffers(0, 1, &m_building.vertBuffer, &m_building.stride, &m_building.offset);

		//Set the WVP matrix and send it to the constant buffer in effect file
		m_building.cbBuffer.WVP = XMMatrixTranspose(m_building.worldMat * m_camView * m_camProjection);
		m_building.cbBuffer.World = XMMatrixTranspose(m_building.worldMat);
		m_building.cbBuffer.difColor = m_building.materials[m_building.subsetTexture[i]].difColor;
		m_building.cbBuffer.hasTexture = m_building.materials[m_building.subsetTexture[i]].hasTexture;
		m_building.cbBuffer.hasNormal = m_building.materials[m_building.subsetTexture[i]].hasNormMap;
		m_d3dImmediateContext->UpdateSubresource(m_building.constBuffer, 0, NULL, &m_building.cbBuffer, 0, 0);
		m_d3dImmediateContext->VSSetConstantBuffers(0, 1, &m_building.constBuffer);
		m_d3dImmediateContext->PSSetConstantBuffers(1, 1, &m_building.constBuffer);
		if ( m_building.materials[m_building.subsetTexture[i]].hasTexture > 0.5f)
			m_d3dImmediateContext->PSSetShaderResources(0, 1, &m_building.shaderResView[m_building.materials[m_building.subsetTexture[i]].texArrayIndex]);
		if ( m_building.materials[m_building.subsetTexture[i]].hasNormMap > 0.5f)
			m_d3dImmediateContext->PSSetShaderResources(1, 1, &m_building.shaderResView[m_building.materials[m_building.subsetTexture[i]].normMapTexArrayIndex]);
		m_d3dImmediateContext->PSSetSamplers(0, 1, &m_building.texSamplerState);

		m_d3dImmediateContext->RSSetState(m_building.rasterState);
		int indexStart = m_building.subsetIndexStart[i];
		int indexDrawAmount = m_building.subsetIndexStart[i + 1] - m_building.subsetIndexStart[i];
		//if ( m_building.materials[m_building.subsetIndexStart[i]].transparent > 0.5f)
			m_d3dImmediateContext->DrawIndexed(indexDrawAmount, indexStart, 0);
	}

	m_d3dImmediateContext->VSSetShader(m_barrel.vertexShader, NULL, 0);
	m_d3dImmediateContext->PSSetShader(m_barrel.pixelShader, NULL, 0);
	m_d3dImmediateContext->IASetInputLayout(m_barrel.inputLayout);
	m_d3dImmediateContext->OMSetDepthStencilState(NULL, 0);

	for ( int i = 0; i < m_barrel.subsets; i++ ) {
		//Set the grounds index buffer
		m_d3dImmediateContext->IASetIndexBuffer(m_barrel.indexBuffer, DXGI_FORMAT_R32_UINT, 0);
		//Set the grounds vertex buffer
		m_d3dImmediateContext->IASetVertexBuffers(0, 1, &m_barrel.vertBuffer, &m_barrel.stride, &m_barrel.offset);

		//Set the WVP matrix and send it to the constant buffer in effect file
		m_barrel.cbBuffer.WVP = XMMatrixTranspose(m_barrel.worldMat * m_camView * m_camProjection);
		m_barrel.cbBuffer.World = XMMatrixTranspose(m_barrel.worldMat);
		m_barrel.cbBuffer.difColor = m_barrel.materials[m_barrel.subsetTexture[i]].difColor;
		m_barrel.cbBuffer.hasTexture = m_barrel.materials[m_barrel.subsetTexture[i]].hasTexture;
		m_barrel.cbBuffer.hasNormal = m_barrel.materials[m_barrel.subsetTexture[i]].hasNormMap;
		m_d3dImmediateContext->UpdateSubresource(m_barrel.constBuffer, 0, NULL, &m_barrel.cbBuffer, 0, 0);
		m_d3dImmediateContext->VSSetConstantBuffers(0, 1, &m_barrel.constBuffer);
		m_d3dImmediateContext->PSSetConstantBuffers(1, 1, &m_barrel.constBuffer);
		if ( m_barrel.materials[m_barrel.subsetTexture[i]].hasTexture > 0.5f )
			m_d3dImmediateContext->PSSetShaderResources(0, 1, &m_barrel.shaderResView[m_barrel.materials[m_barrel.subsetTexture[i]].texArrayIndex]);
		if ( m_barrel.materials[m_barrel.subsetTexture[i]].hasNormMap > 0.5f )
			m_d3dImmediateContext->PSSetShaderResources(1, 1, &m_barrel.shaderResView[m_barrel.materials[m_barrel.subsetTexture[i]].normMapTexArrayIndex]);
		m_d3dImmediateContext->PSSetSamplers(0, 1, &m_barrel.texSamplerState);

		m_d3dImmediateContext->RSSetState(m_barrel.rasterState);
		int indexStart = m_barrel.subsetIndexStart[i];
		int indexDrawAmount = m_barrel.subsetIndexStart[i + 1] - m_barrel.subsetIndexStart[i];
		if ( m_barrel.materials[m_barrel.subsetIndexStart[i]].transparent < 0.5f)
			m_d3dImmediateContext->DrawIndexed(indexDrawAmount, indexStart, 0);
	}

	// Skybox
	//Set the proper VS and PS shaders, and layout
	m_d3dImmediateContext->VSSetShader(m_skyBox.vertexShader, 0, 0);
	m_d3dImmediateContext->PSSetShader(m_skyBox.pixelShader, 0, 0);
	m_d3dImmediateContext->IASetInputLayout(m_skyBox.inputLayout);
	//Set the spheres index buffer
	m_d3dImmediateContext->IASetIndexBuffer(m_skyBox.indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	//Set the spheres vertex buffer
	m_d3dImmediateContext->IASetVertexBuffers(0, 1, &m_skyBox.vertBuffer, &m_skyBox.stride, &m_skyBox.offset);
	//Set the WVP matrix and send it to the constant buffer in shader file
	m_skyBox.cBuffer.WVP = XMMatrixTranspose(m_skyBox.worldMat * m_camView * m_camProjection);
	m_d3dImmediateContext->UpdateSubresource(m_skyBox.constBuffer, 0, NULL, &m_skyBox.cBuffer, 0, 0);
	m_d3dImmediateContext->VSSetConstantBuffers(0, 1, &m_skyBox.constBuffer);
	//Send our skymap resource view to pixel shader
	m_d3dImmediateContext->PSSetShaderResources(0, 1, &m_skyBox.shaderResView);
	m_d3dImmediateContext->PSSetSamplers(0, 1, &m_skyBox.texSamplerState);
	//Set the new depth/stencil and RS states
	m_d3dImmediateContext->OMSetDepthStencilState(m_skyBox.DSLessEqual, 0);
	m_d3dImmediateContext->RSSetState(m_skyBox.rasterState);
	m_d3dImmediateContext->DrawIndexed(m_skyBox.numFaces * 3, 0, 0);
	// skybox -- end

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