#pragma once
#include "Defines.h"

class Camera {
public:
	Camera();
	~Camera(){}

	// Get/Set world camera position.
	XMVECTOR GetPosition()		const							{ return m_position; }
	void SetPosition(const XMVECTOR& v)							{ m_position = v; }
	void SetPosition(float _x, float _y, float _z, float _w)	{ m_position = XMVectorSet(_x, _y, _z, _w); }

	// Get camera basis vectors.
	XMVECTOR GetRight()		const	{ return m_right; }
	XMVECTOR GetUp()		const	{ return m_up; }
	XMVECTOR GetLook()		const	{ return m_look; }

	// Get frustum properties.
	float GetNearZ()	const { return m_nearZ; }
	float GetFarZ()		const { return m_farZ; }
	float GetAspect()	const { return m_aspect; }
	float GetFovY()		const { return m_fovY; }
	float GetFovX()		const 
	{
		float halfWidth = 0.5f*GetNearWindowWidth();
		return 2.0f*atan(halfWidth / m_nearZ);
	}

	// Get near and far plane dimensions in view space coordinates.
	float GetNearWindowWidth()	const { return m_aspect * m_nearWindowHeight; }
	float GetNearWindowHeight()	const { return m_nearWindowHeight; }
	float GetFarWindowWidth()	const { return m_aspect * m_farWindowHeight; }
	float GetFarWindowHeight()	const { return m_farWindowHeight; }

	// Set frustum.
	void SetLens(float fovY, float aspect, float zn, float zf);

	// Define camera space via LookAt parameters.
	void LookAt(FXMVECTOR pos, FXMVECTOR target, FXMVECTOR worldUp);
	void LookAt(const XMFLOAT3& pos, const XMFLOAT3& target, const XMFLOAT3& up);

	// Get View/Proj matrices.
	XMMATRIX GetView()const { return m_view; }
	XMMATRIX GetProj()const { return m_projection; }
	XMMATRIX GetViewProj()const { return XMMatrixMultiply(GetView(), GetProj()); }

	// Strafe/Walk the camera a distance d.
	void Strafe(float d);
	void Walk(float d);

	// Rotate the camera.
	void Pitch(float angle);
	void RotateY(float angle);

	void UpdateViewMatrix();

private:
	XMVECTOR m_position;
	XMVECTOR m_right;
	XMVECTOR m_up;
	XMVECTOR m_look;

	float m_nearZ;
	float m_farZ;
	float m_aspect;
	float m_fovY;
	float m_nearWindowHeight;
	float m_farWindowHeight;

	XMMATRIX m_view;
	XMMATRIX m_projection;
};