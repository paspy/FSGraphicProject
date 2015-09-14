#pragma once
#include "Defines.h"
#include "Waves.h"

namespace D3DSturcture {

	typedef struct Vertex3D {
		Vertex3D() { Position = XMFLOAT3(0, 0, 0); Normal = XMFLOAT3(0, 0, 0); TangentU = XMFLOAT3(0, 0, 0); TexCoord = XMFLOAT2(0, 0); }
		Vertex3D(XMFLOAT3 _pos, XMFLOAT2 _tex) : Position(_pos), TexCoord(_tex) {}
		Vertex3D(XMFLOAT3 _pos, XMFLOAT2 _tex, XMFLOAT3 _norm) : Position(_pos), TexCoord(_tex), Normal(_norm) {}
		Vertex3D(const XMFLOAT3& p, const XMFLOAT3& n, const XMFLOAT3& t, const XMFLOAT2& uv) : Position(p), Normal(n), TangentU(t), TexCoord(uv) {}
		Vertex3D(
			float px, float py, float pz,
			float nx, float ny, float nz,
			float tx, float ty, float tz,
			float u, float v
			) : Position(px, py, pz), Normal(nx, ny, nz), TangentU(tx, ty, tz), TexCoord(u, v) {
		}

		XMFLOAT3 Position;
		XMFLOAT2 TexCoord;
		XMFLOAT3 Normal;
		// bump Normal mapping
		XMFLOAT3 TangentU;
	}*Vertex3D_ptr;

	typedef struct InstancedData {
		XMFLOAT4X4 World;
		XMFLOAT4 Color;
	}*InstancedData_ptr;

	// constant buffer structures
	struct DirectionalLight {
		DirectionalLight() { ZeroMemory(this, sizeof(this)); }
		XMFLOAT4 Ambient;
		XMFLOAT4 Diffuse;
		XMFLOAT4 Specular;

		XMFLOAT3 Direction;
		float Pad;
	};

	struct PointLight {
		PointLight() { ZeroMemory(this, sizeof(this)); }
		XMFLOAT4 Ambient;
		XMFLOAT4 Diffuse;
		XMFLOAT4 Specular;

		XMFLOAT3 Position;
		float Range;

		XMFLOAT3 Att;
		float Pad;
	};

	struct SpotLight {
		SpotLight() { ZeroMemory(this, sizeof(this)); }
		XMFLOAT4 Ambient;
		XMFLOAT4 Diffuse;
		XMFLOAT4 Specular;

		XMFLOAT3 Position;
		float Range;

		XMFLOAT3 Direction;
		float Spot;

		XMFLOAT3 Att;
		float Pad;
	};

	struct cbPerFrame {
		DirectionalLight directionalLight;
		PointLight pointLight;
		SpotLight spotLight;
		XMFLOAT4 cameraPos;

	};

}
