#pragma once
#include "Defines.h"

class Camera;

class ParticleSystem {
public:

	struct Particle {
		XMFLOAT3 InitialPos;
		XMFLOAT3 InitialVel;
		XMFLOAT2 Size;
		float Age;
		UINT Type;
	};

	ParticleSystem();
	~ParticleSystem();

	// Time elapsed since the system was reset.
	float GetAge()const;

	void SetEyePos(const XMFLOAT3& eyePosW);
	void SetEmitPos(const XMFLOAT3& emitPosW);
	void SetEmitDir(const XMFLOAT3& emitDirW);

	void Init(ID3D11Device* _d3dDevice, ID3D11ShaderResourceView* _texArraySRV, ID3D11ShaderResourceView* _randomTexSRV, UINT _maxParticles);

	void Reset();
	void Update(float _dt, float gameTime);
	void Render(ID3D11DeviceContext* _context, const Camera& _camera);

private:
	void BuildVertBuffer(ID3D11Device* _d3dDevice);

	ParticleSystem(const ParticleSystem& rhs);
	ParticleSystem& operator=(const ParticleSystem& rhs);

private:

	UINT m_maxParticles;
	bool m_firstRun;

	float m_gameTime;
	float m_timeStep;
	float m_age;

	XMFLOAT3 m_eyePositionW;
	XMFLOAT3 m_emitPositionW;
	XMFLOAT3 m_emitDirectionW;

	ID3D11Buffer *m_initVertBuffer;
	ID3D11Buffer *m_drawVertBuffer;
	ID3D11Buffer *m_streamOutVertBuffer;

	ID3D11ShaderResourceView *m_texArraySRV;
	ID3D11ShaderResourceView *m_randomTexSRV;

	ID3D11InputLayout *m_inputLayout;

	const D3D11_INPUT_ELEMENT_DESC ParticleInputLayout[5] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "VELOCITY", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "SIZE",     0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "AGE",      0, DXGI_FORMAT_R32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TYPE",     0, DXGI_FORMAT_R32_UINT,        0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
};