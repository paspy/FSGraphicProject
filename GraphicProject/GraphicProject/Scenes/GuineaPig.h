#pragma once
#include "../D3DApp/D3DApp.h"
#include "../D3DApp/DirectXTK/DDSTextureLoader.h"

typedef struct Vertex3D {
	Vertex3D() {}
	Vertex3D(XMFLOAT3 _pos, XMFLOAT4 _color) : pos(_pos), color(_color) {}
	Vertex3D(XMFLOAT3 _pos, XMFLOAT2 _tex) : pos(_pos), texCoord(_tex) { XMStoreFloat4(&color, Colors::Black); }
	Vertex3D(XMFLOAT3 _pos, XMFLOAT2 _tex, XMFLOAT3 _norm) : pos(_pos), texCoord(_tex), normal(_norm) {
		XMStoreFloat4(&color, Colors::Black);
	}

	XMFLOAT3 pos;
	XMFLOAT4 color;
	XMFLOAT2 texCoord;
	XMFLOAT3 normal;
	// bump normal mapping
	XMFLOAT3 tangent;
	XMFLOAT3 biTangent;
}*Vertex3D_ptr;

struct BaseLight {
	BaseLight() { ZeroMemory(this, sizeof(BaseLight)); }
	XMFLOAT3 direction;
	float paddding_1;
	XMFLOAT3 position;
	float range;
	XMFLOAT3 spotLightDir;
	float cone;
	XMFLOAT3 attenuation;
	float paddding_2;
	XMFLOAT4 ambient;
	XMFLOAT4 diffuse;
};

struct SurfaceMaterial {
	SurfaceMaterial() : hasNormMap(false), hasTexture(false) {}
	wstring matName;
	XMFLOAT4 difColor;
	int texArrayIndex;
	int normMapTexArrayIndex;
	bool hasNormMap;
	bool hasTexture;
	bool transparent;
};


// constant buffer structures
struct ConstPerObject {
	ConstPerObject() : hasTexture(false), hasNormal(false) {}
	XMMATRIX WVP;
	XMMATRIX World;
	int texIndex;
	XMFLOAT4 difColor;
	// need to 4 bytes
	bool hasTexture;
	bool hasNormal;
};

struct ConstPerFrame {
	BaseLight baseLight;
};

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

	bool CreateModelFromObjFile(wstring _filename,					//.obj filename
		ID3D11Buffer** _vertBuff,									//mesh vertex buffer
		ID3D11Buffer** _indexBuff,									//mesh index buffer
		vector<int>& _subsetIndexStart,								//start index of each subset
		vector<int>& _subsetMaterialArray,							//index value of material for each subset
		vector<SurfaceMaterial>& _material,							//vector of material structures
		int& _subsetCount,											//Number of subsets in mesh
		bool _isRHCoordSys,											//true if model was created in right hand coord system
		bool _computeNormals);										//true to compute the normals, false to use the files normals

private:
	void BuildSphere(int _latLines, int _longLines,
		ID3D11Buffer ** _vertBuffer, ID3D11Buffer ** _indexBuffer,
		int &_numSphereVertices, int &_numSphereFaces);

	void BuildObjConstBuffer();
	void BuildGeometryBuffers();
	void BuildGroundBuffers();
	void BuildTextureAndState();
	void BuildLighting();
	void BuildShader();
	void BuildVertexLayout();
	void BuildRenderStates();

private:
	// sky box mapping
	ID3D11Buffer					*m_sphereIndexBuffer = nullptr;
	ID3D11Buffer					*m_sphereVertBuffer = nullptr;

	ID3D11VertexShader				*m_skyboxVertexShader = nullptr;
	ID3D11PixelShader				*m_skyboxPixelShader = nullptr;

	ID3D11InputLayout				*m_skyboxInputLayout = nullptr;

	ID3D11ShaderResourceView		*m_skyboxShaderResView = nullptr;

	ID3D11DepthStencilState			*m_skyboxDSLessEqual = nullptr;
	ID3D11RasterizerState			*m_skyboxRasterState = nullptr;

	int								m_numSphereVertices;
	int								m_numSphereFaces;

	XMMATRIX						m_sphereWorld;

	// objects
	ID3D11Buffer					*m_cubeVertexBuffer;
	ID3D11Buffer					*m_cubeIndexBuffer;
	ID3D11Buffer					*m_groundVertexBuffer;
	ID3D11Buffer					*m_groundIndexBuffer;

	ID3D11InputLayout				*m_inputLayout;
	ID3D11VertexShader				*m_vertexShader;
	ID3D11PixelShader				*m_pixelShader;

	ConstPerObject					m_cbCubeObject;
	ID3D11Buffer					*m_cbCubeBuffer = nullptr;

	ConstPerObject					m_cbGroundObject;
	ID3D11Buffer					*m_cbGroundBuffer = nullptr;

	// Object
	vector<Vertex3D>				m_gridVerts;
	XMMATRIX						m_cubeWorldMat;
	XMMATRIX						m_groundWorldMat;

	// Render States
	ID3D11RasterizerState			*m_antialiasedLine = nullptr;

	// texture
	ID3D11ShaderResourceView		*m_cubeShaderResView = nullptr;
	ID3D11SamplerState				*m_baseTexSamplerState = nullptr;

	ID3D11ShaderResourceView		*m_grassShaderResView = nullptr;

	// Lighting
	ID3D11Buffer					*m_perFrameBuffer = nullptr;
	BaseLight						m_baseLight;

	ConstPerFrame					m_cbPerFrame;
	ID3D11Buffer					*m_cbPerFrameBuffer = nullptr;

	// blending transparency
	ID3D11BlendState				*m_blendTransparency = nullptr;
	ID3D11RasterizerState			*m_cwCullingMode = nullptr;
	ID3D11RasterizerState			*m_ccwCullingMode = nullptr;

	// obj loader
	ID3D11Buffer					*m_meshVertBuff = nullptr;
	ID3D11Buffer					*m_meshIndexBuff = nullptr;
	XMMATRIX						m_meshWorld;
	int								m_meshSubsets = 0;
	vector<int>						m_meshSubsetIndexStart;
	vector<int>						m_meshSubsetTexture;
	vector<SurfaceMaterial>			m_materials;
	vector<ID3D11ShaderResourceView*> m_meshShaderResView;
	vector<wstring>					m_textureNameArray;

};