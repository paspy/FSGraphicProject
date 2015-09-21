#include "ParticleSystem.h"
#include "Camera.h"

ParticleSystem::ParticleSystem() : 
	m_initVertBuffer(nullptr),
	m_drawVertBuffer(nullptr),
	m_streamOutVertBuffer(nullptr),
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
	SafeRelease(m_initVertBuffer);
	SafeRelease(m_drawVertBuffer);
	SafeRelease(m_streamOutVertBuffer);
	SafeRelease(m_texArraySRV);
	SafeRelease(m_randomTexSRV);
	SafeRelease(m_inputLayout);
}

float ParticleSystem::GetAge()const {
	return m_age;
}

void ParticleSystem::SetEyePos(const XMFLOAT3& eyePosW) {
	m_eyePositionW = eyePosW;
}

void ParticleSystem::SetEmitPos(const XMFLOAT3& emitPosW) {
	m_emitPositionW = emitPosW;
}

void ParticleSystem::SetEmitDir(const XMFLOAT3& emitDirW) {
	m_emitDirectionW = emitDirW;
}

void ParticleSystem::Init(ID3D11Device* _d3dDevice, ID3D11ShaderResourceView* _texArraySRV, ID3D11ShaderResourceView* _randomTexSRV, UINT _maxParticles) {
	m_maxParticles = _maxParticles;

	m_texArraySRV = _texArraySRV;
	m_randomTexSRV = _randomTexSRV;

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

void ParticleSystem::Render(ID3D11DeviceContext* _context, const Camera& _camera) {
	XMMATRIX VP = _camera.GetViewProj();

	//
	// Set constants.
	//
	//mFX->SetViewProj(VP);
	//mFX->SetGameTime(m_gameTime);
	//mFX->SetTimeStep(m_timeStep);
	//mFX->SetEyePosW(m_eyePositionW);
	//mFX->SetEmitPosW(m_emitPositionW);
	//mFX->SetEmitDirW(m_emitDirectionW);
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
	//_context->SOSetTargets(1, &m_streamOutVertBuffer, &offset);

	//D3DX11_TECHNIQUE_DESC techDesc;
	//mFX->StreamOutTech->GetDesc(&techDesc);
	//for (UINT p = 0; p < techDesc.Passes; ++p) {
	//	mFX->StreamOutTech->GetPassByIndex(p)->Apply(0, _context);

	//	if (m_firstRun) {
	//		_context->Draw(1, 0);
	//		m_firstRun = false;
	//	} else {
	//		_context->DrawAuto();
	//	}
	//}

	// done streaming-out--unbind the vertex buffer
	ID3D11Buffer* bufferArray[1] = { 0 };
	_context->SOSetTargets(1, bufferArray, &offset);

	// ping-pong the vertex buffers
	std::swap(m_drawVertBuffer, m_streamOutVertBuffer);

	//
	// Draw the updated particle system we just streamed-out. 
	//
	_context->IASetVertexBuffers(0, 1, &m_drawVertBuffer, &stride, &offset);

	//mFX->DrawTech->GetDesc(&techDesc);
	//for (UINT p = 0; p < techDesc.Passes; ++p) {
	//	mFX->DrawTech->GetPassByIndex(p)->Apply(0, _context);

	//	_context->DrawAuto();
	//}
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