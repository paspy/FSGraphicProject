#include "D3DUtils.h"

HRESULT D3DUtils::CreateShaderAndLayoutFromFile(
	ID3D11Device *_d3dDevice,
	const LPCWSTR _fileName,
	const D3D11_INPUT_ELEMENT_DESC *_inputElemDesc,
	const UINT _elemNum,
	ID3D11VertexShader **_vertexShader,
	ID3D11PixelShader ** _pixelShader,
	ID3D11InputLayout **_inputLayout
	) {

	HRESULT hr;
	ID3DBlob* vertexShaderBlob = nullptr;
	ID3DBlob* pixelShaderBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;

	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
	flags |= D3DCOMPILE_DEBUG;
#endif

	hr = D3DCompileFromFile(_fileName, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VSMain", "vs_5_0", flags, 0, &vertexShaderBlob, &errorBlob);

	if (FAILED(hr)) {
		if (errorBlob) {
			string err = (char*)errorBlob->GetBufferPointer();
			wstring werr(err.begin(), err.end());
			MessageBox(0, werr.c_str(), L"Error", MB_OK);
			//OutputDebugStringA((char*)errorBlob->GetBufferPointer());
			SafeRelease(errorBlob);
		}

		SafeRelease(vertexShaderBlob);
		return hr;
	}

	hr = D3DCompileFromFile(_fileName, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PSMain", "ps_5_0", flags, 0, &pixelShaderBlob, &errorBlob);

	if (FAILED(hr)) {
		if (errorBlob) {
			string err = (char*)errorBlob->GetBufferPointer();
			wstring werr(err.begin(), err.end());
			MessageBox(0, werr.c_str(), L"Error", MB_OK);
			//OutputDebugStringA((char*)errorBlob->GetBufferPointer());
			SafeRelease(errorBlob);
		}

		SafeRelease(pixelShaderBlob);
		return hr;
	}

	// create vertex layout
	HR(_d3dDevice->CreateInputLayout(_inputElemDesc, _elemNum, vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), _inputLayout));
	// create vertex shader
	HR(_d3dDevice->CreateVertexShader(vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), NULL, _vertexShader));
	// create pixel shader
	HR(_d3dDevice->CreatePixelShader(pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize(), NULL, _pixelShader));

	return hr;
}


void D3DUtils::BuildSphere(
	ID3D11Device *_d3dDevice,
	int _latLines,
	int _longLines,
	ID3D11Buffer ** _vertBuffer,
	ID3D11Buffer ** _indexBuffer,
	int &_numSphereVertices,
	int &_numSphereFaces) {

	_numSphereVertices = ((_latLines - 2) * _longLines) + 2;
	_numSphereFaces = ((_latLines - 3)*(_longLines)* 2) + (_longLines * 2);

	float sphereYaw = 0.0f;
	float spherePitch = 0.0f;
	XMMATRIX RotationX;
	XMMATRIX RotationY;
	XMMATRIX RotationZ;

	vector<Vertex3D> vertices(_numSphereVertices);

	XMVECTOR currVertPos = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

	vertices[0].pos.x = 0.0f;
	vertices[0].pos.y = 0.0f;
	vertices[0].pos.z = 1.0f;

	for (int i = 0; i < _latLines - 2; i++) {
		spherePitch = (i + 1) * (3.14f / (_latLines - 1));
		RotationX = XMMatrixRotationX(spherePitch);
		for (int j = 0; j < _longLines; j++) {
			sphereYaw = j * (6.28f / (_longLines));
			RotationY = XMMatrixRotationZ(sphereYaw);
			currVertPos = XMVector3TransformNormal(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), (RotationX * RotationY));
			currVertPos = XMVector3Normalize(currVertPos);
			vertices[i*_longLines + j + 1].pos.x = XMVectorGetX(currVertPos);
			vertices[i*_longLines + j + 1].pos.y = XMVectorGetY(currVertPos);
			vertices[i*_longLines + j + 1].pos.z = XMVectorGetZ(currVertPos);
		}
	}

	vertices[_numSphereVertices - 1].pos.x = 0.0f;
	vertices[_numSphereVertices - 1].pos.y = 0.0f;
	vertices[_numSphereVertices - 1].pos.z = -1.0f;

	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));

	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(Vertex3D) * _numSphereVertices;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA vertexBufferData;

	ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
	vertexBufferData.pSysMem = &vertices[0];
	HR(_d3dDevice->CreateBuffer(&vertexBufferDesc, &vertexBufferData, _vertBuffer));

	vector<DWORD> indices(_numSphereFaces * 3);

	int k = 0;
	for (int l = 0; l < _longLines - 1; ++l) {
		indices[k] = 0;
		indices[k + 1] = l + 1;
		indices[k + 2] = l + 2;
		k += 3;
	}

	indices[k] = 0;
	indices[k + 1] = _longLines;
	indices[k + 2] = 1;
	k += 3;

	for (int i = 0; i < _latLines - 3; i++) {
		for (int j = 0; j < _longLines - 1; j++) {
			indices[k] = i*_longLines + j + 1;
			indices[k + 1] = i*_longLines + j + 2;
			indices[k + 2] = (i + 1)*_longLines + j + 1;

			indices[k + 3] = (i + 1)*_longLines + j + 1;
			indices[k + 4] = i*_longLines + j + 2;
			indices[k + 5] = (i + 1)*_longLines + j + 2;

			k += 6; // next quad
		}

		indices[k] = (i*_longLines) + _longLines;
		indices[k + 1] = (i*_longLines) + 1;
		indices[k + 2] = ((i + 1)*_longLines) + _longLines;

		indices[k + 3] = ((i + 1)*_longLines) + _longLines;
		indices[k + 4] = (i*_longLines) + 1;
		indices[k + 5] = ((i + 1)*_longLines) + 1;

		k += 6;
	}

	for (int l = 0; l < _longLines - 1; ++l) {
		indices[k] = _numSphereVertices - 1;
		indices[k + 1] = (_numSphereVertices - 1) - (l + 1);
		indices[k + 2] = (_numSphereVertices - 1) - (l + 2);
		k += 3;
	}

	indices[k] = _numSphereVertices - 1;
	indices[k + 1] = (_numSphereVertices - 1) - _longLines;
	indices[k + 2] = _numSphereVertices - 2;

	D3D11_BUFFER_DESC indexBufferDesc;
	ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));

	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(DWORD) * _numSphereFaces * 3;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA iinitData;

	iinitData.pSysMem = &indices[0];
	HR(_d3dDevice->CreateBuffer(&indexBufferDesc, &iinitData, _indexBuffer));
}

bool D3DUtils::CreateModelFromObjFileKaiNi(ID3D11Device * _d3dDevice, IDXGISwapChain * _swapChain, string _filename, ID3D11Buffer ** _vertBuff, ID3D11Buffer ** _indexBuff) {

	vector<shape_t> shapes;
	vector<material_t> materials;

	string err = tinyobj::LoadObj(shapes, materials, _filename.c_str(), "Resources/Models/");

	if (!err.empty()) {
		wstring werr(err.begin(), err.end());
		MessageBox(0, werr.c_str(), L"Error", MB_OK);
		return false;
	}

	std::cout << "# of shapes    : " << shapes.size() << std::endl;
	std::cout << "# of materials : " << materials.size() << std::endl;




	return false;
}


// source: http://www.braynzarsoft.net/index.php?p=D3D11OBJMODEL
bool D3DUtils::CreateModelFromObjFile(
	ID3D11Device *_d3dDevice,
	IDXGISwapChain *_swapChain,
	wstring _filename,
	ID3D11Buffer ** _vertBuff,
	ID3D11Buffer ** _indexBuff,
	vector<wstring> &_textureNameArray,
	vector<ID3D11ShaderResourceView*> &_meshShaderResView,
	vector<int>& _subsetIndexStart,
	vector<int>& _subsetMaterialArray,
	vector<SurfaceMaterial>& _materials,
	int & _subsetCount,
	bool _isRHCoordSys,
	bool _computeNormals) {

	HRESULT hr = 0;

	wifstream fileIn(_filename.c_str());	//Open file
	wstring meshMatLib;						//String to hold our obj material library _filename

	//Arrays to store our model's information
	vector<DWORD> indices;
	vector<XMFLOAT3> vertPos;
	vector<XMFLOAT2> vertTexCoord;
	vector<XMFLOAT3> vertNorm;
	vector<wstring> meshMaterials;

	//Vertex definition indices
	vector<int> vertPosIndex;
	vector<int> vertNormIndex;
	vector<int> vertTCIndex;

	//Make sure we have a default if no tex coords or normals are defined
	bool hasTexCoord = false;
	bool hasNorm = false;

	//Temp variables to store into vectors
	wstring meshMaterialsTemp;
	int vertPosIndexTemp;
	int vertNormIndexTemp;
	int vertTCIndexTemp;

	wchar_t checkChar;		//The variable we will use to store one char from file at a time
	wstring face;		//Holds the string containing our face vertices
	int vIndex = 0;			//Keep track of our vertex index count
	int triangleCount = 0;	//Total Triangles
	int totalVerts = 0;
	int meshTriangles = 0;

	//Check to see if the file was opened
	if (fileIn) {
		while (fileIn) {
			checkChar = fileIn.get();	//Get next char

			switch (checkChar) {
			case '#':
				checkChar = fileIn.get();
				while (checkChar != '\n')
					checkChar = fileIn.get();
				break;
			case 'v':	//Get Vertex Descriptions
				checkChar = fileIn.get();
				if (checkChar == ' ')	//v - vert position
				{
					float vz, vy, vx;
					fileIn >> vx >> vy >> vz;	//Store the next three types

					if (_isRHCoordSys)	//If model is from an RH Coord System
						vertPos.push_back(XMFLOAT3(vx, vy, vz * -1.0f));	//Invert the Z axis
					else
						vertPos.push_back(XMFLOAT3(vx, vy, vz));
				}
				if (checkChar == 't')	//vt - vert tex coords
				{
					float vtcu, vtcv;
					fileIn >> vtcu >> vtcv;		//Store next two types

					if (_isRHCoordSys)	//If model is from an RH Coord System
						vertTexCoord.push_back(XMFLOAT2(vtcu, 1.0f - vtcv));	//Reverse the "v" axis
					else
						vertTexCoord.push_back(XMFLOAT2(vtcu, vtcv));

					hasTexCoord = true;	//We know the model uses texture coords
				}
				//Since we compute the normals later, we don't need to check for normals
				//In the file, but i'll do it here anyway
				if (checkChar == 'n')	//vn - vert normal
				{
					float vnx, vny, vnz;
					fileIn >> vnx >> vny >> vnz;	//Store next three types

					if (_isRHCoordSys)	//If model is from an RH Coord System
						vertNorm.push_back(XMFLOAT3(vnx, vny, vnz * -1.0f));	//Invert the Z axis
					else
						vertNorm.push_back(XMFLOAT3(vnx, vny, vnz));

					hasNorm = true;	//We know the model defines normals
				}
				break;

				//New group (Subset)
			case 'g':	//g - defines a group
				checkChar = fileIn.get();
				if (checkChar == ' ') {
					_subsetIndexStart.push_back(vIndex);		//Start index for this subset
					_subsetCount++;
				}
				break;

				//Get Face Index
			case 'f':	//f - defines the faces
				checkChar = fileIn.get();
				if (checkChar == ' ') {
					face = L"";
					wstring VertDef;	//Holds one vertex definition at a time
					triangleCount = 0;

					checkChar = fileIn.get();
					while (checkChar != '\n') {
						face += checkChar;			//Add the char to our face string
						checkChar = fileIn.get();	//Get the next Character
						if (checkChar == ' ')		//If its a space...
							triangleCount++;		//Increase our triangle count
					}

					//Check for space at the end of our face string
					if (face[face.length() - 1] == ' ')
						triangleCount--;	//Each space adds to our triangle count

					triangleCount -= 1;		//Ever vertex in the face AFTER the first two are new faces

					wstringstream ss(face);

					if (face.length() > 0) {
						int firstVIndex, lastVIndex;	//Holds the first and last vertice's index

						for (int i = 0; i < 3; ++i)		//First three vertices (first triangle)
						{
							ss >> VertDef;	//Get vertex definition (vPos/vTexCoord/vNorm)

							wstring vertPart;
							int whichPart = 0;		//(vPos, vTexCoord, or vNorm)

													//Parse this string
							for (int j = 0; j < VertDef.length(); ++j) {
								if (VertDef[j] != '/')	//If there is no divider "/", add a char to our vertPart
									vertPart += VertDef[j];

								//If the current char is a divider "/", or its the last character in the string
								if (VertDef[j] == '/' || j == VertDef.length() - 1) {
									wistringstream wstringToInt(vertPart);	//Used to convert wstring to int

									if (whichPart == 0)	//If vPos
									{
										wstringToInt >> vertPosIndexTemp;
										vertPosIndexTemp -= 1;		//subtract one since c++ arrays start with 0, and obj start with 1

																	//Check to see if the vert pos was the only thing specified
										if (j == VertDef.length() - 1) {
											vertNormIndexTemp = 0;
											vertTCIndexTemp = 0;
										}
									}

									else if (whichPart == 1)	//If vTexCoord
									{
										if (vertPart != L"")	//Check to see if there even is a tex coord
										{
											wstringToInt >> vertTCIndexTemp;
											vertTCIndexTemp -= 1;	//subtract one since c++ arrays start with 0, and obj start with 1
										} else	//If there is no tex coord, make a default
											vertTCIndexTemp = 0;

										//If the cur. char is the second to last in the string, then
										//there must be no normal, so set a default normal
										if (j == VertDef.length() - 1)
											vertNormIndexTemp = 0;

									} else if (whichPart == 2)	//If vNorm
									{
										wistringstream wstringToInt(vertPart);

										wstringToInt >> vertNormIndexTemp;
										vertNormIndexTemp -= 1;		//subtract one since c++ arrays start with 0, and obj start with 1
									}

									vertPart = L"";	//Get ready for next vertex part
									whichPart++;	//Move on to next vertex part					
								}
							}

							//Check to make sure there is at least one subset
							if (_subsetCount == 0) {
								_subsetIndexStart.push_back(vIndex);		//Start index for this subset
								_subsetCount++;
							}

							//Avoid duplicate vertices
							bool vertAlreadyExists = false;
							if (totalVerts >= 3)	//Make sure we at least have one triangle to check
							{
								//Loop through all the vertices
								for (int iCheck = 0; iCheck < totalVerts; ++iCheck) {
									//If the vertex position and texture coordinate in memory are the same
									//As the vertex position and texture coordinate we just now got out
									//of the obj file, we will set this faces vertex index to the vertex's
									//index value in memory. This makes sure we don't create duplicate vertices
									if (vertPosIndexTemp == vertPosIndex[iCheck] && !vertAlreadyExists) {
										if (vertTCIndexTemp == vertTCIndex[iCheck]) {
											indices.push_back(iCheck);		//Set index for this vertex
											vertAlreadyExists = true;		//If we've made it here, the vertex already exists
										}
									}
								}
							}

							//If this vertex is not already in our vertex arrays, put it there
							if (!vertAlreadyExists) {
								vertPosIndex.push_back(vertPosIndexTemp);
								vertTCIndex.push_back(vertTCIndexTemp);
								vertNormIndex.push_back(vertNormIndexTemp);
								totalVerts++;	//We created a new vertex
								indices.push_back(totalVerts - 1);	//Set index for this vertex
							}

							//If this is the very first vertex in the face, we need to
							//make sure the rest of the triangles use this vertex
							if (i == 0) {
								firstVIndex = indices[vIndex];	//The first vertex index of this FACE

							}

							//If this was the last vertex in the first triangle, we will make sure
							//the next triangle uses this one (eg. tri1(1,2,3) tri2(1,3,4) tri3(1,4,5))
							if (i == 2) {
								lastVIndex = indices[vIndex];	//The last vertex index of this TRIANGLE
							}
							vIndex++;	//Increment index count
						}

						meshTriangles++;	//One triangle down

											//If there are more than three vertices in the face definition, we need to make sure
											//we convert the face to triangles. We created our first triangle above, now we will
											//create a new triangle for every new vertex in the face, using the very first vertex
											//of the face, and the last vertex from the triangle before the current triangle
						for (int l = 0; l < triangleCount - 1; ++l)	//Loop through the next vertices to create new triangles
						{
							//First vertex of this triangle (the very first vertex of the face too)
							indices.push_back(firstVIndex);			//Set index for this vertex
							vIndex++;

							//Second Vertex of this triangle (the last vertex used in the tri before this one)
							indices.push_back(lastVIndex);			//Set index for this vertex
							vIndex++;

							//Get the third vertex for this triangle
							ss >> VertDef;

							wstring vertPart;
							int whichPart = 0;

							//Parse this string (same as above)
							for (int j = 0; j < VertDef.length(); ++j) {
								if (VertDef[j] != '/')
									vertPart += VertDef[j];
								if (VertDef[j] == '/' || j == VertDef.length() - 1) {
									wistringstream wstringToInt(vertPart);

									if (whichPart == 0) {
										wstringToInt >> vertPosIndexTemp;
										vertPosIndexTemp -= 1;

										//Check to see if the vert pos was the only thing specified
										if (j == VertDef.length() - 1) {
											vertTCIndexTemp = 0;
											vertNormIndexTemp = 0;
										}
									} else if (whichPart == 1) {
										if (vertPart != L"") {
											wstringToInt >> vertTCIndexTemp;
											vertTCIndexTemp -= 1;
										} else
											vertTCIndexTemp = 0;
										if (j == VertDef.length() - 1)
											vertNormIndexTemp = 0;

									} else if (whichPart == 2) {
										wistringstream wstringToInt(vertPart);

										wstringToInt >> vertNormIndexTemp;
										vertNormIndexTemp -= 1;
									}

									vertPart = L"";
									whichPart++;
								}
							}

							//Check for duplicate vertices
							bool vertAlreadyExists = false;
							if (totalVerts >= 3)	//Make sure we at least have one triangle to check
							{
								for (int iCheck = 0; iCheck < totalVerts; ++iCheck) {
									if (vertPosIndexTemp == vertPosIndex[iCheck] && !vertAlreadyExists) {
										if (vertTCIndexTemp == vertTCIndex[iCheck]) {
											indices.push_back(iCheck);			//Set index for this vertex
											vertAlreadyExists = true;		//If we've made it here, the vertex already exists
										}
									}
								}
							}

							if (!vertAlreadyExists) {
								vertPosIndex.push_back(vertPosIndexTemp);
								vertTCIndex.push_back(vertTCIndexTemp);
								vertNormIndex.push_back(vertNormIndexTemp);
								totalVerts++;					//New vertex created, add to total verts
								indices.push_back(totalVerts - 1);		//Set index for this vertex
							}

							//Set the second vertex for the next triangle to the last vertex we got		
							lastVIndex = indices[vIndex];	//The last vertex index of this TRIANGLE

							meshTriangles++;	//New triangle defined
							vIndex++;
						}
					}
				}
				break;

			case 'm':	//mtllib - material library _filename
				checkChar = fileIn.get();
				if (checkChar == 't') {
					checkChar = fileIn.get();
					if (checkChar == 'l') {
						checkChar = fileIn.get();
						if (checkChar == 'l') {
							checkChar = fileIn.get();
							if (checkChar == 'i') {
								checkChar = fileIn.get();
								if (checkChar == 'b') {
									checkChar = fileIn.get();
									if (checkChar == ' ') {
										//Store the material libraries file name
										fileIn >> meshMatLib;
									}
								}
							}
						}
					}
				}

				break;

			case 'u':	//usemtl - which material to use
				checkChar = fileIn.get();
				if (checkChar == 's') {
					checkChar = fileIn.get();
					if (checkChar == 'e') {
						checkChar = fileIn.get();
						if (checkChar == 'm') {
							checkChar = fileIn.get();
							if (checkChar == 't') {
								checkChar = fileIn.get();
								if (checkChar == 'l') {
									checkChar = fileIn.get();
									if (checkChar == ' ') {
										meshMaterialsTemp = L"";	//Make sure this is cleared

										fileIn >> meshMaterialsTemp; //Get next type (string)

										meshMaterials.push_back(meshMaterialsTemp);
									}
								}
							}
						}
					}
				}
				break;

			default:
				break;
			}
		}
	} else {//If we could not open the file
	
		_swapChain->SetFullscreenState(false, NULL);	//Make sure we are out of fullscreen
		//create message
		wstring message = L"Could not open: ";
		message += _filename;

		MessageBox(0, message.c_str(), L"Error", MB_OK);//display message
		return false;
	}

	_subsetIndexStart.push_back(vIndex); //There won't be another index start after our last subset, so set it here
	//sometimes "g" is defined at the very top of the file, then again before the first group of faces.
	//This makes sure the first subset does not conatain "0" indices.
	if (_subsetIndexStart[1] == 0) {
		_subsetIndexStart.erase(_subsetIndexStart.begin() + 1);
		_subsetCount--;
	}

	//Make sure we have a default for the tex coord and normal
	//if one or both are not specified
	if (!hasNorm)
		vertNorm.push_back(XMFLOAT3(0.0f, 0.0f, 0.0f));
	if (!hasTexCoord)
		vertTexCoord.push_back(XMFLOAT2(0.0f, 0.0f));

	//Close the obj file, and open the mtl file
	fileIn.close();
	wstring mtlFileName = L"Resources/Models/";
	mtlFileName += meshMatLib.c_str();
	fileIn.open(mtlFileName);

	wstring lastStringRead;
	size_t matCount = _materials.size();	//total materials
	//kdset - If our diffuse color was not set, we can use the ambient color (which is usually the same)
	//If the diffuse color WAS set, then we don't need to set our diffuse color to ambient
	bool kdset = false;

	if (fileIn) {
		while (fileIn) {
			checkChar = fileIn.get();	//Get next char

			switch (checkChar) {
				//Check for comment
			case '#':
				checkChar = fileIn.get();
				while (checkChar != '\n')
					checkChar = fileIn.get();
				break;

				//Set diffuse color
			case 'K':
				checkChar = fileIn.get();
				if (checkChar == 'd')	//Diffuse Color
				{
					checkChar = fileIn.get();	//remove space

					fileIn >> _materials[matCount - 1].difColor.x;
					fileIn >> _materials[matCount - 1].difColor.y;
					fileIn >> _materials[matCount - 1].difColor.z;

					kdset = true;
				}

				//Ambient Color (We'll store it in diffuse if there isn't a diffuse already)
				if (checkChar == 'a') {
					checkChar = fileIn.get();	//remove space
					if (!kdset) {
						fileIn >> _materials[matCount - 1].difColor.x;
						fileIn >> _materials[matCount - 1].difColor.y;
						fileIn >> _materials[matCount - 1].difColor.z;
					}
				}
				break;

				//Check for transparency
			case 'T':
				checkChar = fileIn.get();
				if (checkChar == 'r') {
					checkChar = fileIn.get();	//remove space
					float Transparency;
					fileIn >> Transparency;

					_materials[matCount - 1].difColor.w = Transparency;

					//if (Transparency > 0.0f) _materials[matCount - 1].transparent = 1.0f;
				}
				break;

				//Some obj files specify d for transparency
			case 'd':
				checkChar = fileIn.get();
				if (checkChar == ' ') {
					float Transparency;
					fileIn >> Transparency;

					//'d' - 0 being most transparent, and 1 being opaque, opposite of Tr
					Transparency = 1.0f - Transparency;

					_materials[matCount - 1].difColor.w = Transparency;

					//if (Transparency > 0.0f) _materials[matCount - 1].transparent = 1.0f;
				}
				break;

				//Get the diffuse map (texture)
			case 'm':
				checkChar = fileIn.get();
				if (checkChar == 'a') {
					checkChar = fileIn.get();
					if (checkChar == 'p') {
						checkChar = fileIn.get();
						if (checkChar == '_') {
							//map_Kd - Diffuse map
							checkChar = fileIn.get();
							if (checkChar == 'K') {
								checkChar = fileIn.get();
								if (checkChar == 'd') {
									wstring fileNamePath;

									fileIn.get();	//Remove whitespace between map_Kd and file

													//Get the file path - We read the pathname char by char since
													//pathnames can sometimes contain spaces, so we will read until
													//we find the file extension
									bool texFilePathEnd = false;
									while (!texFilePathEnd) {
										checkChar = fileIn.get();

										fileNamePath += checkChar;

										if (checkChar == '.') {
											for (int i = 0; i < 3; ++i)
												fileNamePath += fileIn.get();

											texFilePathEnd = true;
										}
									}

									//check if this texture has already been loaded
									bool alreadyLoaded = false;
									for (int i = 0; i < _textureNameArray.size(); ++i) {
										if (fileNamePath == _textureNameArray[i]) {
											alreadyLoaded = true;
											_materials[matCount - 1].texArrayIndex = i;
											//_materials[matCount - 1].hasTexture = 1.0f;
										}
									}

									//if the texture is not already loaded, load it now
									if (!alreadyLoaded) {
										ID3D11ShaderResourceView* tempMeshSRV;
										wstring fileName = L"Resources/Models/";
										fileName += fileNamePath.c_str();
										hr = CreateDDSTextureFromFile(_d3dDevice, fileName.c_str(), NULL, &tempMeshSRV);
										//hr = D3DX11CreateShaderResourceViewFromFile(d3d11Device, fileNamePath.c_str(), NULL, NULL, &tempMeshSRV, NULL);

										if (SUCCEEDED(hr)) {
											_textureNameArray.push_back(fileNamePath.c_str());
											_materials[matCount - 1].texArrayIndex = (int)_meshShaderResView.size();
											_meshShaderResView.push_back(tempMeshSRV);
											//_materials[matCount - 1].hasTexture = 1.0f;
										}
									}
								}
							}
							//map_d - alpha map
							else if (checkChar == 'd') {
								//Alpha maps are usually the same as the diffuse map
								//So we will assume that for now by only enabling
								//transparency for this material, as we will already
								//be using the alpha channel in the diffuse map
								//_materials[matCount - 1].transparent = 1.0f;
							}
							//map_bump - bump map (we're usinga normal map though)
							else if (checkChar == 'b') {
								checkChar = fileIn.get();
								if (checkChar == 'u') {
									checkChar = fileIn.get();
									if (checkChar == 'm') {
										checkChar = fileIn.get();
										if (checkChar == 'p') {
											wstring fileNamePath;

											fileIn.get();	//Remove whitespace between map_bump and file

															//Get the file path - We read the pathname char by char since
															//pathnames can sometimes contain spaces, so we will read until
															//we find the file extension
											bool texFilePathEnd = false;
											while (!texFilePathEnd) {
												checkChar = fileIn.get();

												fileNamePath += checkChar;

												if (checkChar == '.') {
													for (int i = 0; i < 3; ++i)
														fileNamePath += fileIn.get();

													texFilePathEnd = true;
												}
											}

											//check if this texture has already been loaded
											bool alreadyLoaded = false;
											for (int i = 0; i < _textureNameArray.size(); ++i) {
												if (fileNamePath == _textureNameArray[i]) {
													alreadyLoaded = true;
													_materials[matCount - 1].normMapTexArrayIndex = i;
													//_materials[matCount - 1].hasNormMap = 1.0f;
												}
											}

											//if the texture is not already loaded, load it now
											if (!alreadyLoaded) {
												ID3D11ShaderResourceView* tempMeshSRV;
												/*hr = D3DX11CreateShaderResourceViewFromFile(d3d11Device, fileNamePath.c_str(),
												NULL, NULL, &tempMeshSRV, NULL);*/
												wstring fileName = L"Resources/Models/";
												fileName += fileNamePath.c_str();
												hr = CreateDDSTextureFromFile(_d3dDevice, fileName.c_str(), NULL, &tempMeshSRV);
												if (SUCCEEDED(hr)) {
													_textureNameArray.push_back(fileNamePath.c_str());
													_materials[matCount - 1].normMapTexArrayIndex = (int)_meshShaderResView.size();
													_meshShaderResView.push_back(tempMeshSRV);
													//_materials[matCount - 1].hasNormMap = 1.0f;
												}
											}
										}
									}
								}
							}
						}
					}
				}
				break;

			case 'n':	//newmtl - Declare new material
				checkChar = fileIn.get();
				if (checkChar == 'e') {
					checkChar = fileIn.get();
					if (checkChar == 'w') {
						checkChar = fileIn.get();
						if (checkChar == 'm') {
							checkChar = fileIn.get();
							if (checkChar == 't') {
								checkChar = fileIn.get();
								if (checkChar == 'l') {
									checkChar = fileIn.get();
									if (checkChar == ' ') {
										//New material, set its defaults
										SurfaceMaterial tempMat;
										_materials.push_back(tempMat);
										fileIn >> _materials[matCount].matName;
										//_materials[matCount].transparent = 0.0f;
										//_materials[matCount].hasTexture = 0.0f;
										//_materials[matCount].hasNormMap = 0.0f;
										_materials[matCount].normMapTexArrayIndex = 0;
										_materials[matCount].texArrayIndex = 0;
										matCount++;
										kdset = false;
									}
								}
							}
						}
					}
				}
				break;

			default:
				break;
			}
		}
	} else {
		_swapChain->SetFullscreenState(false, NULL);	//Make sure we are out of fullscreen

		wstring message = L"Could not open: ";
		message += meshMatLib;

		MessageBox(0, message.c_str(), L"Error", MB_OK);
		return false;
	}

	//Set the subsets material to the index value
	//of the its material in our material array
	for (int i = 0; i < _subsetCount; ++i) {
		bool hasMat = false;
		for (int j = 0; j < _materials.size(); ++j) {
			if (meshMaterials[i] == _materials[j].matName) {
				_subsetMaterialArray.push_back(j);
				hasMat = true;
			}
		}
		if (!hasMat)
			_subsetMaterialArray.push_back(0); //Use first material in array
	}

	vector<Vertex3D> vertices;
	Vertex3D tempVert;

	//Create our vertices using the information we got 
	//from the file and store them in a vector
	for (int j = 0; j < totalVerts; ++j) {
		tempVert.pos = vertPos[vertPosIndex[j]];
		tempVert.normal = vertNorm[vertNormIndex[j]];
		tempVert.texCoord = vertTexCoord[vertTCIndex[j]];

		vertices.push_back(tempVert);
	}

	//////////////////////Compute Normals///////////////////////////
	//If computeNormals was set to true then we will create our own
	//normals, if it was set to false we will use the obj files normals
	if (_computeNormals) {
		vector<XMFLOAT3> tempNormal;

		//normalized and unnormalized normals
		XMFLOAT3 unnormalized = XMFLOAT3(0.0f, 0.0f, 0.0f);

		//tangent stuff
		vector<XMFLOAT3> tempTangent;
		XMFLOAT3 tangent = XMFLOAT3(0.0f, 0.0f, 0.0f);
		float tcU1, tcV1, tcU2, tcV2;

		//Used to get vectors (sides) from the position of the verts
		float vecX, vecY, vecZ;

		//Two edges of our triangle
		XMVECTOR edge1 = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
		XMVECTOR edge2 = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);

		//Compute face normals
		for (int i = 0; i < meshTriangles; ++i) {
			//Get the vector describing one edge of our triangle (edge 0,2)
			vecX = vertices[indices[(i * 3)]].pos.x - vertices[indices[(i * 3) + 2]].pos.x;
			vecY = vertices[indices[(i * 3)]].pos.y - vertices[indices[(i * 3) + 2]].pos.y;
			vecZ = vertices[indices[(i * 3)]].pos.z - vertices[indices[(i * 3) + 2]].pos.z;
			edge1 = XMVectorSet(vecX, vecY, vecZ, 0.0f);	//Create our first edge

			//Get the vector describing another edge of our triangle (edge 2,1)
			vecX = vertices[indices[(i * 3) + 2]].pos.x - vertices[indices[(i * 3) + 1]].pos.x;
			vecY = vertices[indices[(i * 3) + 2]].pos.y - vertices[indices[(i * 3) + 1]].pos.y;
			vecZ = vertices[indices[(i * 3) + 2]].pos.z - vertices[indices[(i * 3) + 1]].pos.z;
			edge2 = XMVectorSet(vecX, vecY, vecZ, 0.0f);	//Create our second edge

			//Cross multiply the two edge vectors to get the un-normalized face normal
			XMStoreFloat3(&unnormalized, XMVector3Cross(edge1, edge2));
			tempNormal.push_back(unnormalized);			//Save unormalized normal (for normal averaging)

			//Find first texture coordinate edge 2d vector
			tcU1 = vertices[indices[(i * 3)]].texCoord.x - vertices[indices[(i * 3) + 2]].texCoord.x;
			tcV1 = vertices[indices[(i * 3)]].texCoord.y - vertices[indices[(i * 3) + 2]].texCoord.y;

			//Find second texture coordinate edge 2d vector
			tcU2 = vertices[indices[(i * 3) + 2]].texCoord.x - vertices[indices[(i * 3) + 1]].texCoord.x;
			tcV2 = vertices[indices[(i * 3) + 2]].texCoord.y - vertices[indices[(i * 3) + 1]].texCoord.y;

			//Find tangent using both tex coord edges and position edges
			tangent.x = (tcV1 * XMVectorGetX(edge1) - tcV2 * XMVectorGetX(edge2)) * (1.0f / (tcU1 * tcV2 - tcU2 * tcV1));
			tangent.y = (tcV1 * XMVectorGetY(edge1) - tcV2 * XMVectorGetY(edge2)) * (1.0f / (tcU1 * tcV2 - tcU2 * tcV1));
			tangent.z = (tcV1 * XMVectorGetZ(edge1) - tcV2 * XMVectorGetZ(edge2)) * (1.0f / (tcU1 * tcV2 - tcU2 * tcV1));

			tempTangent.push_back(tangent);
		}

		//Compute vertex normals (normal Averaging)
		XMVECTOR normalSum = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
		XMVECTOR tangentSum = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
		int facesUsing = 0;
		float tX;
		float tY;
		float tZ;

		//Go through each vertex
		for (int i = 0; i < totalVerts; ++i) {
			//Check which triangles use this vertex
			for (int j = 0; j < meshTriangles; ++j) {
				if (indices[j * 3] == i ||
					indices[(j * 3) + 1] == i ||
					indices[(j * 3) + 2] == i) {
					tX = XMVectorGetX(normalSum) + tempNormal[j].x;
					tY = XMVectorGetY(normalSum) + tempNormal[j].y;
					tZ = XMVectorGetZ(normalSum) + tempNormal[j].z;

					normalSum = XMVectorSet(tX, tY, tZ, 0.0f);	//If a face is using the vertex, add the unormalized face normal to the normalSum

					//We can reuse tX, tY, tZ to sum up tangents
					tX = XMVectorGetX(tangentSum) + tempTangent[j].x;
					tY = XMVectorGetY(tangentSum) + tempTangent[j].y;
					tZ = XMVectorGetZ(tangentSum) + tempTangent[j].z;

					tangentSum = XMVectorSet(tX, tY, tZ, 0.0f); //sum up face tangents using this vertex
					facesUsing++;
				}
			}

			//Get the actual normal by dividing the normalSum by the number of faces sharing the vertex
			normalSum = normalSum / (float)facesUsing;
			tangentSum = tangentSum / (float)facesUsing;

			//Normalize the normalSum vector
			normalSum = XMVector3Normalize(normalSum);
			tangentSum = XMVector3Normalize(tangentSum);

			//Store the normal in our current vertex
			vertices[i].normal.x = XMVectorGetX(normalSum);
			vertices[i].normal.y = XMVectorGetY(normalSum);
			vertices[i].normal.z = XMVectorGetZ(normalSum);

			vertices[i].tangent.x = XMVectorGetX(tangentSum);
			vertices[i].tangent.y = XMVectorGetY(tangentSum);
			vertices[i].tangent.z = XMVectorGetZ(tangentSum);

			//Clear normalSum and facesUsing for next vertex
			normalSum = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
			tangentSum = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
			facesUsing = 0;
		}
	}

	//Create index buffer
	D3D11_BUFFER_DESC indexBufferDesc;
	ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));

	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(DWORD) * meshTriangles * 3;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA iinitData;

	iinitData.pSysMem = indices.data();
	HR(_d3dDevice->CreateBuffer(&indexBufferDesc, &iinitData, _indexBuff));

	//Create Vertex Buffer
	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));

	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(Vertex3D) * static_cast<UINT>(vertices.size());
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA vertexBufferData;

	ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
	vertexBufferData.pSysMem = vertices.data();
	HR(_d3dDevice->CreateBuffer(&vertexBufferDesc, &vertexBufferData, _vertBuff));

	return true;
}


