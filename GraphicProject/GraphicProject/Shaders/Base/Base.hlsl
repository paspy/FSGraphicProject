#include <Lighting.hlsl>

cbuffer cbPerFrame {
	DirectionalLight gDirLight;
	PointLight gPointLight;
	SpotLight gSpotLight;
	float4 gCameraPos;
};

cbuffer CBuffer {
	float4x4 gWorld;
	float4x4 gWorldViewProj;
	Material gMaterial;
};

Texture2D ObjTexture;
Texture2D ObjNormMap;
SamplerState ObjSamplerState;

struct VS_INPUT {
	float3 PositionL : POSITION;
	float2 TexCoord : TEXCOORD;
	float3 NormalL : NORMAL;
	float3 TangentL : TANGENT;
};

struct VS_OUTPUT {
	float4 PositionH : SV_POSITION;
	float3 PositionW : POSITION;
	float2 TexCoord : TEXCOORD;
	float3 NormalW : NORMAL;
	float3 TangentW : TANGENT;
};


// Vertex Shader Entry Point
VS_OUTPUT VSMain(VS_INPUT vsInput) {

	VS_OUTPUT vsOutput = (VS_OUTPUT)0;;

	// Transform to homogeneous clip space.
	vsOutput.PositionH = mul(float4(vsInput.PositionL, 1.0f), gWorldViewProj);

	// Transform to world space space.
	vsOutput.PositionW = mul(float4(vsInput.PositionL, 1.0f), gWorld).xyz;
	vsOutput.NormalW   = normalize(mul(vsInput.NormalL, (float3x3)gWorld));
	vsOutput.TangentW  = normalize(mul(vsInput.TangentL, (float3x3)gWorld));

	vsOutput.TexCoord = vsInput.TexCoord;

	return vsOutput;
}


// Pixel Shader Entry Point
float4 PSMain(VS_OUTPUT psInput) : SV_TARGET {
	psInput.NormalW = normalize(psInput.NormalW);

	float4 textColor = ObjTexture.Sample(ObjSamplerState, psInput.TexCoord);
	
	Material currMat;
	currMat.Ambient = gMaterial.Ambient;
	currMat.Diffuse = textColor;
	currMat.Specular = gMaterial.Specular;

	//Load normal from normal map
	float4 normalMap = ObjNormMap.Sample(ObjSamplerState, psInput.TexCoord);

	//Change normal map range from [0, 1] to [-1, 1]
	normalMap = (2.0f*normalMap) - 1.0f;

	//Make sure TangentU is normalized after interpolation
	psInput.TangentW = normalize(psInput.TangentW);

	//Create the biTangent
	float3 biTangent = cross(psInput.NormalW, psInput.TangentW);

	//Create the "Texture Space" - T B N - Yeah!
	float3x3 texSpace = float3x3(psInput.TangentW, biTangent, psInput.NormalW);

	//Convert normal from normal map to texture space and store in psInput.normal
	psInput.NormalW = normalize(mul((float3)normalMap, texSpace));

	//******* Apply lighting *******//

	// Directional Light
	float4 ambient  = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 diffuse  = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 specular = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float4 A, D, S; // temp Ambient, Diffuse, and Specular

	float3 toCameraW = normalize(gCameraPos.xyz - psInput.PositionW);

	// comput the directional light A D S
	ComputeDirectionalLight(currMat, gDirLight, psInput.NormalW, toCameraW, A, D, S);
	ambient  += A;
	diffuse  += D;
	specular += S;

	// comput the point light A D S
	ComputePointLight(currMat, gPointLight, psInput.PositionW, psInput.NormalW, toCameraW, A, D, S);
	ambient += A;
	diffuse += D;
	specular += S;

	// comput the spot light A D S
	ComputeSpotLight(currMat, gSpotLight, psInput.PositionW, psInput.NormalW, toCameraW, A, D, S);
	ambient += A;
	diffuse += D;
	specular += S;

	float4 finalColor = ambient + diffuse + specular;
	return float4(finalColor.rgb, gMaterial.Diffuse.a);

}