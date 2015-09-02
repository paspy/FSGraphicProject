#pragma once
#include "../D3DApp/D3DApp.h"

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
	void BuildShaderAndLayout();
	void BuildRenderStates();

private:
	Skybox								m_skyBox;

	// ground
	ID3D11InputLayout					*m_inputLayout;
	ID3D11VertexShader					*m_vertexShader;
	ID3D11PixelShader					*m_pixelShader;

	// Lighting
	ID3D11Buffer						*m_perFrameBuffer = nullptr;
	BaseLight							m_baseLight;

	ConstPerFrame						m_cbPerFrame;
	ID3D11Buffer						*m_cbPerFrameBuffer = nullptr;

	// blending transparency
	ID3D11BlendState					*m_blendTransparency = nullptr;
	ID3D11RasterizerState				*m_cwCullingMode = nullptr;
	ID3D11RasterizerState				*m_ccwCullingMode = nullptr;

	// obj loader
	ConstPerObject						m_cbMeshObject;
	ID3D11Buffer						*m_cbMeshBuffer = nullptr;
	ID3D11Buffer						*m_meshVertBuff = nullptr;
	ID3D11Buffer						*m_meshIndexBuff = nullptr;
	XMMATRIX							m_meshWorld;
	int									m_meshSubsets = 0;
	vector<int>							m_meshSubsetIndexStart;
	vector<int>							m_meshSubsetTexture;
	vector<SurfaceMaterial>				m_materials;
	vector<ID3D11ShaderResourceView*>	m_meshShaderResView;
	vector<wstring>						m_textureNameArray;

};

