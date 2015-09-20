#include "Camera.h"

Camera::Camera() : 
	m_position(XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f)),
	m_right(XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f)),
	m_up(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)),
	m_look(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f)) {
	SetLens(0.25f*XM_PI, 1.0f, 1.0f, 1000.0f);
}

void Camera::SetLens(float fovY, float aspect, float zn, float zf) {
	// cache properties
	m_fovY = fovY;
	m_aspect = aspect;
	m_nearZ = zn;
	m_farZ = zf;

	m_nearWindowHeight = 2.0f * m_nearZ * tanf(0.5f*m_fovY);
	m_farWindowHeight = 2.0f * m_farZ * tanf(0.5f*m_fovY);

	XMMATRIX m_projection = XMMatrixPerspectiveFovLH(m_fovY, m_aspect, m_nearZ, m_farZ);
}

void Camera::LookAt(XMVECTOR pos, XMVECTOR target, XMVECTOR worldUp) {
	XMVECTOR L = XMVector3Normalize(XMVectorSubtract(target, pos));
	XMVECTOR R = XMVector3Normalize(XMVector3Cross(worldUp, L));
	XMVECTOR U = XMVector3Cross(L, R);

	m_position = pos;
	m_look	= L;
	m_right	= R;
	m_up	= U;
}

void Camera::LookAt(const XMFLOAT3& pos, const XMFLOAT3& target, const XMFLOAT3& up) {
	XMVECTOR P = XMLoadFloat3(&pos);
	XMVECTOR T = XMLoadFloat3(&target);
	XMVECTOR U = XMLoadFloat3(&up);

	LookAt(P, T, U);
}

void Camera::Strafe(float d) {
	// m_position += d*m_right
	m_position = XMVectorMultiplyAdd(XMVectorReplicate(d), m_right, m_position);
}

void Camera::Walk(float d) {
	// m_position += d*m_look
	m_position = XMVectorMultiplyAdd(XMVectorReplicate(d), m_look, m_position);
}

void Camera::Pitch(float angle) {
	// Rotate up and look vector about the right vector.
	XMMATRIX R = XMMatrixRotationAxis(m_right, angle);
	m_up = XMVector3TransformNormal(m_up, R);
	m_look = XMVector3TransformNormal(m_look, R);
}

void Camera::RotateY(float angle) {
	// Rotate the basis vectors about the world y-axis.

	XMMATRIX R = XMMatrixRotationY(angle);
	m_right = XMVector3TransformNormal(m_right, R);
	m_up = XMVector3TransformNormal(m_up, R);
	m_look = XMVector3TransformNormal(m_look, R);
}

void Camera::UpdateViewMatrix() {
	XMVECTOR R = m_right;
	XMVECTOR U = m_up;
	XMVECTOR L = m_look;
	XMVECTOR P = m_position;

	// Keep camera's axes orthogonal to each other and of unit length.
	L = XMVector3Normalize(L);
	U = XMVector3Normalize(XMVector3Cross(L, R));

	// U, L already ortho-normal, so no need to normalize cross product.
	R = XMVector3Cross(U, L);

	// Fill in the view matrix entries.
	float x = -XMVectorGetX(XMVector3Dot(P, R));
	float y = -XMVectorGetX(XMVector3Dot(P, U));
	float z = -XMVectorGetX(XMVector3Dot(P, L));

	m_right = R;
	m_up = U;
	m_look = L;
	XMFLOAT4 right, up, look;
	XMStoreFloat4(&right, m_right);
	XMStoreFloat4(&up, m_up);
	XMStoreFloat4(&look, m_look);

	m_view.r[0] = XMLoadFloat4(&XMFLOAT4(right.x, up.x, look.x, 0.0f));
	m_view.r[1] = XMLoadFloat4(&XMFLOAT4(right.y, up.y, look.y, 0.0f));
	m_view.r[2] = XMLoadFloat4(&XMFLOAT4(right.z, up.z, look.z, 0.0f));
	m_view.r[3] = XMLoadFloat4(&XMFLOAT4(x, y, z, 1.0f));
}