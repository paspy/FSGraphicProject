#pragma once
#include "../D3DApp/D3DApp.h"

#include "../MeshControllers/ObjMesh.h"
#include "../MeshControllers/Skybox.h"
#include "../MeshControllers/TerrainMesh.h"
#include "../MeshControllers/WaveMesh.h"
#include "../MeshControllers/GeoMesh.h"
#include "../MeshControllers/MirrorMesh.h"

using namespace D3DSturcture;

class GuineaPig : public D3DApp {
public:
	GuineaPig(HINSTANCE hinst);
	~GuineaPig();

	bool Init();
	void OnResize();
	void UpdateKeyboardInput(double _dt);
	void UpdateScene(double _dt);
	void UpdateCamera();
	void DrawScene();

	void OnMouseDown(WPARAM _btnState, int _x, int _y);
	void OnMouseUp(WPARAM _btnState, int _x, int _y);
	void OnMouseMove(WPARAM _btnState, int _x, int _y);

private:
	void BuildConstBuffer();
	void BuildGeometry();
	void BuildLighting();

private:
	Skybox				m_skyBox;
	ObjMesh				m_barrel;
	ObjMesh				m_bed;
	WaveMesh			m_wave;
	TerrainMesh			m_terrain;
	GeoMesh				m_geoMesh;
	MirrorMesh			m_mirrorMesh;

	// Lighting
	DirectionalLight	m_directionalLight;
	PointLight			m_pointLight;
	SpotLight			m_spotLight;

	cbPerFrame			m_cbPerFrame;
	ID3D11Buffer		*m_cbPerFrameBuffer = nullptr;
};

