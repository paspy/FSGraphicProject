#pragma once
#include "../D3DApp/D3DApp.h"
#include "../D3DApp/ParticleSystem.h"
#include "../D3DApp/BlurFilter.h"

#include "../MeshControllers/ObjMesh.h"
#include "../MeshControllers/Skybox.h"
#include "../MeshControllers/TerrainMesh.h"
#include "../MeshControllers/WaveMesh.h"
#include "../MeshControllers/GeoMesh.h"
#include "../MeshControllers/MirrorMesh.h"

#include "../MeshControllers/Terrain.h"

using namespace D3DSturcture;

class GuineaPig : public D3DApp {
public:
	GuineaPig(HINSTANCE hinst);
	~GuineaPig();

	bool Init();
	void OnResize();
	void UpdateKeyboardInput(double _dt);
	void UpdateScene(double _dt);
	void DrawScene();

	void OnMouseDown(WPARAM _btnState, int _x, int _y);
	void OnMouseUp(WPARAM _btnState, int _x, int _y);
	void OnMouseMove(WPARAM _btnState, int _x, int _y);

private:
	void BuildConstBuffer();
	void BuildGeometry();
	void BuildLighting();
	void BuildOffscreenViews();
	void BuildScreenQuadGeometryBuffers();
	void DrawScreenQuad();

private:
	Skybox				m_skyBox;
	WaveMesh			m_wave;
	TerrainMesh			m_terrain;
	GeoMesh				m_geoMesh;
	MirrorMesh			m_mirrorMesh;

	Terrain				m_heighMapTerrain;

	GeoMesh				m_quadMesh;

	// Lighting
	DirectionalLight	m_directionalLight;
	PointLight			m_pointLight;
	SpotLight			m_spotLight;

	cbPerFrame			m_cbPerFrame;
	ID3D11Buffer		*m_cbPerFrameBuffer = nullptr;

	// Particle
	ParticleSystem		m_rain;
	ParticleSystem		m_fire;

	// camera setting
	bool				m_camWalkMode;

	// off screen stuff
	BlurFilter					m_blur;
	cbOffScreen					m_cbOffScreen;
	ID3D11VertexShader			*m_osVertexShader = nullptr;
	ID3D11PixelShader			*m_osPixelShader = nullptr;
	ID3D11InputLayout			*m_osInputLayout = nullptr;
	ID3D11Buffer				*m_screenQuadVB = nullptr;
	ID3D11Buffer				*m_screenQuadIB = nullptr;
	ID3D11Buffer				*m_constOffScreen = nullptr;
	ID3D11ShaderResourceView	*m_offscreenSRV = nullptr;
	ID3D11UnorderedAccessView	*m_offscreenUAV = nullptr;
	ID3D11RenderTargetView		*m_offscreenRTV = nullptr;

};

