#include "ParticleSystem.h"
#include "Camera.h"
#include "RenderStates.h"
#include "D3DUtils.h"

ParticleSystem::ParticleSystem() : 
	m_streamOutVS(nullptr),
	m_streamOutGS(nullptr),
	m_vertexShader(nullptr),
	m_geoShader(nullptr),
	m_pixelShader(nullptr),

	m_initVertBuffer(nullptr),
	m_drawVertBuffer(nullptr),
	m_streamOutVertBuffer(nullptr),
	m_cbPerFrame(nullptr),
	m_linerSamplerState(nullptr),
	m_texArraySRV(nullptr),
	m_randomTexSRV(nullptr),
	m_inputLayout(nullptr)

{
	m_firstRun = true;
	m_gameTime = 0.0f;
	m_timeStep = 0.0f;
	m_age = 0.0f;

	m_eyePositionW = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_emitPositionW = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_emitDirectionW = XMFLOAT3(0.0f, 1.0f, 0.0f);
}

ParticleSystem::~ParticleSystem() {
	SafeRelease(m_streamOutVS);
	SafeRelease(m_streamOutGS);
	SafeRelease(m_vertexShader);
	SafeRelease(m_geoShader);
	SafeRelease(m_pixelShader);
	SafeRelease(m_initVertBuffer);
	SafeRelease(m_drawVertBuffer);
	SafeRelease(m_cbPerFrame);
	SafeRelease(m_linerSamplerState);
	SafeRelease(m_streamOutVertBuffer);
	SafeRelease(m_texArraySRV);
	SafeRelease(m_randomTexSRV);
	SafeRelease(m_inputLayout);
}

void ParticleSystem::Init(
	ID3D11Device* _d3dDevice,
	ID3D11DeviceContext* _context,
	UINT _maxParticles,
	LPCWSTR _shaderFilename,
	vector<wstring> _textrues
	) {

	D3D11_BUFFER_DESC cbbd;
	ZeroMemory(&cbbd, sizeof(D3D11_BUFFER_DESC));
	cbbd.Usage = D3D11_USAGE_DEFAULT;
	cbbd.ByteWidth = sizeof(cbPerFrameP);
	cbbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbbd.CPUAccessFlags = 0;
	cbbd.MiscFlags = 0;
	HR(_d3dDevice->CreateBuffer(&cbbd, NULL, &m_cbPerFrame));

	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = -D3D11_FLOAT32_MAX;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	_d3dDevice->CreateSamplerState(&sampDesc, &m_linerSamplerState);

	D3D11_SO_DECLARATION_ENTRY ParticleStreamOutDecl[5] = {
		// semantic name, semantic index, start component, component count, output slot
		{ 0, "POSITION"	, 0, 0, 3, 0 },
		{ 0, "VELOCITY"	, 0, 0, 3, 0 },
		{ 0, "SIZE"		, 0, 0, 2, 0 },
		{ 0, "AGE"		, 0, 0, 1, 0 },
		{ 0, "TYPE"		, 0, 0, 1, 0 },
	};

	// create the depending shaders
	HR(D3DUtils::CreateShaderAndLayoutFromFile(_d3dDevice, _shaderFilename, ParticleInputLayout, 5, &m_vertexShader, &m_pixelShader, &m_inputLayout));
	HR(D3DUtils::CreateOptionalShaderFromFile(_d3dDevice, _shaderFilename, &m_streamOutGS, true, &m_streamOutVS, ParticleStreamOutDecl));
	HR(D3DUtils::CreateOptionalShaderFromFile(_d3dDevice, _shaderFilename, &m_geoShader));

	m_texArraySRV = D3DUtils::CreateTexture2DArraySRV(_d3dDevice, _context, _textrues);
	m_randomTexSRV = D3DUtils::CreateRandomTexture1DSRV(_d3dDevice);
	m_maxParticles = _maxParticles;
	BuildVertBuffer(_d3dDevice);
}

void ParticleSystem::Reset() {
	m_firstRun = true;
	m_age = 0.0f;
}

void ParticleSystem::Update(float _dt, float gameTime) {
	m_gameTime = gameTime;
	m_timeStep = _dt;
	m_age += _dt;
}

void ParticleSystem::Render(ID3D11DeviceContext* _context, const Camera& _camera, ID3D11BlendState* _bs, float *_bf) {
	float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };

	cbPerFrame.ViewProj = XMMatrixTranspose(_camera.GetViewProj());
	cbPerFrame.GameTime = m_gameTime;
	cbPerFrame.TimeStep = m_timeStep;

	XMFLOAT4 curCamPos;
	XMStoreFloat4(&curCamPos, _camera.GetPosition());
	cbPerFrame.CameraPos = curCamPos;
	cbPerFrame.EmitPositon = XMFLOAT4(m_emitPositionW.x, m_emitPositionW.y, m_emitPositionW.z, 1.0f);
	cbPerFrame.EmitDirection = XMFLOAT4(m_emitDirectionW.x, m_emitDirectionW.y, m_emitDirectionW.z, 1.0f);

	_context->UpdateSubresource(m_cbPerFrame, 0, NULL, &cbPerFrame, 0, 0);
	_context->VSSetConstantBuffers(0, 1, &m_cbPerFrame);
	_context->GSSetConstantBuffers(0, 1, &m_cbPerFrame);
	_context->PSSetConstantBuffers(0, 1, &m_cbPerFrame);
	_context->RSSetState(0);

	// Stream Out Stage
	_context->VSSetShader(m_streamOutVS, NULL, 0);
	_context->GSSetShader(m_streamOutGS, NULL, 0);
	_context->PSSetShader(NULL, NULL, 0);
	_context->GSSetShaderResources(1, 1, &m_randomTexSRV);

	_context->GSSetSamplers(0, 1, &m_linerSamplerState);
	_context->VSSetSamplers(0, 1, &m_linerSamplerState);

	_context->OMSetDepthStencilState(RenderStates::DisableDepth, 0);
	_context->OMSetBlendState(_bs, _bf, 0xffffffff);
	//mFX->SetTexArray(m_texArraySRV);
	//mFX->SetRandomTex(m_randomTexSRV);

	//
	// Set IA stage.
	//
	_context->IASetInputLayout(m_inputLayout);
	_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	UINT stride = sizeof(Particle);
	UINT offset = 0;

	// On the first pass, use the initialization VB.  Otherwise, use
	// the VB that contains the current particle list.
	if (m_firstRun)
		_context->IASetVertexBuffers(0, 1, &m_initVertBuffer, &stride, &offset);
	else
		_context->IASetVertexBuffers(0, 1, &m_drawVertBuffer, &stride, &offset);

	//
	// Draw the current particle list using stream-out only to update them.  
	// The updated vertices are streamed-out to the target VB. 
	//
	_context->SOSetTargets(1, &m_streamOutVertBuffer, &offset);

	if (m_firstRun) {
		_context->Draw(1, 0);
		m_firstRun = false;
	} else {
		_context->DrawAuto();
	}

	_context->VSSetShader(m_vertexShader, NULL, 0);
	_context->GSSetShader(m_geoShader, NULL, 0);
	_context->PSSetShader(m_pixelShader, NULL, 0);
	_context->PSSetShaderResources(0, 1, &m_texArraySRV);

	_context->OMSetDepthStencilState(RenderStates::NoDepthWrites, 0);
	// done streaming-out--unbind the vertex buffer
	ID3D11Buffer* bufferArray[1] = { 0 };
	_context->SOSetTargets(1, bufferArray, &offset);

	// ping-pong the vertex buffers
	swap(m_drawVertBuffer, m_streamOutVertBuffer);

	//
	// Draw the updated particle system we just streamed-out. 
	//
	_context->IASetVertexBuffers(0, 1, &m_drawVertBuffer, &stride, &offset);

	_context->DrawAuto();

	// restore default
	_context->GSSetShader(0, 0, 0);
	_context->OMSetBlendState(0, blendFactor, 0xffffffff); 
	_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void ParticleSystem::BuildVertBuffer(ID3D11Device* _d3dDevice) {
	//
	// Create the buffer to kick-off the particle system.
	//

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_DEFAULT;
	vbd.ByteWidth = sizeof(Particle) * 1;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	// The initial particle emitter has type 0 and age 0.  The rest
	// of the particle attributes do not apply to an emitter.
	Particle p;
	ZeroMemory(&p, sizeof(Particle));
	p.Age = 0.0f;
	p.Type = 0;

	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &p;

	HR(_d3dDevice->CreateBuffer(&vbd, &vinitData, &m_initVertBuffer));

	//
	// Create the ping-pong buffers for stream-out and drawing.
	//
	vbd.ByteWidth = sizeof(Particle) * m_maxParticles;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_STREAM_OUTPUT;

	HR(_d3dDevice->CreateBuffer(&vbd, 0, &m_drawVertBuffer));
	HR(_d3dDevice->CreateBuffer(&vbd, 0, &m_streamOutVertBuffer));
}