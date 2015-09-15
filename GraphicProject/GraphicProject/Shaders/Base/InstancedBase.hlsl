#include <Lighting.hlsl>

cbuffer cbPerFrame {
	DirectionalLight gDirLight;
	PointLight gPointLight;
	SpotLight gSpotLight;
	float4 gCameraPos;
};

cbuffer CBuffer {
	float4x4 gWorldInvTranspose;
	float4x4 gView;
	float4x4 gProj;
	float4x4 gTexTransform;
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

	row_major float4x4 World  : WORLD;
	uint InstanceId : SV_InstanceID;
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

	VS_OUTPUT vsOutput = (VS_OUTPUT)0;

	// Transform to world space space.
	vsOutput.PositionW = mul(float4(vsInput.PositionL, 1.0f), vsInput.World).xyz;
	vsOutput.NormalW = normalize(mul(vsInput.NormalL, (float3x3)vsInput.World));
	vsOutput.TangentW = normalize(mul(vsInput.TangentL, (float3x3)vsInput.World));

	// Transform to homogeneous clip space.
	vsOutput.PositionH = mul(mul(mul(float4(vsInput.PositionL, 1.0f), vsInput.World), gView), gProj);

	vsOutput.TexCoord = mul(float4(vsInput.TexCoord, 0.0f, 1.0f), gTexTransform).xy;

	return vsOutput;
}


// Pixel Shader Entry Point
float4 PSMain(VS_OUTPUT psInput) : SV_TARGET {
	
	psInput.NormalW = normalize(psInput.NormalW);

	float4 texColor = ObjTexture.Sample(ObjSamplerState, psInput.TexCoord);
	clip(texColor.a - 0.1f);

	Material currMat;
	currMat.Ambient = texColor * gMaterial.Ambient;
	currMat.Diffuse = texColor;
	currMat.Specular = gMaterial.Specular;

	//Load normal from normal map
	float4 normalMap = ObjNormMap.Sample(ObjSamplerState, psInput.TexCoord);
	float3 bumpedNormalW = NormalSampleToWorldSpace(normalMap, psInput.NormalW, psInput.TangentW);

	//******* Apply lighting *******//

	// Directional Light
	float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 specular = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float4 A, D, S; // temp Ambient, Diffuse, and Specular

	float3 toCameraW = normalize(gCameraPos.xyz - psInput.PositionW);

	// comput the directional light A D S
	ComputeDirectionalLight(currMat, gDirLight, bumpedNormalW, toCameraW, A, D, S);
	ambient += A /** psInput.Color*/;
	diffuse += D /** psInput.Color*/;
	specular += S;

	float4 finalColor = ambient + diffuse + specular;
	return float4(finalColor.rgb, gMaterial.Diffuse.a);
}