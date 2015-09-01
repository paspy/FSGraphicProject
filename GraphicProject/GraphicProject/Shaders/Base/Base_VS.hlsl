
cbuffer ConstPerObject {
	float4x4 WVP;
	float4x4 World;
	int texIndex;

	float4 difColor;
	bool hasTexture;
	bool hasNormMap;
};

// textures!
Texture2D ObjTexture;
SamplerState ObjSamplerState;


struct VS_OUTPUT {
	float4 Position : SV_POSITION;
	float4 WorldPos : POSITION;
	float4 Color : COLOR;
	float2 TexCoord : TEXCOORD;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
};


VS_OUTPUT main(float4 inPos : POSITION,
			   float4 inColor : COLOR,
			   float2 inTexCoord : TEXCOORD,
			   float3 inNormal : NORMAL,
			   float3 inTangent : TANGENT) {

	VS_OUTPUT output;

	output.Position = mul(inPos, WVP);
	output.WorldPos = mul(inPos, World);
	output.Normal = mul(inNormal, (float3x3)World); //output.Normal = mul(float4(inNormal, 0.0), World).rgb;
	output.Tangent = mul(inTangent, (float3x3)World);

	output.Color = inColor;

	output.TexCoord = inTexCoord;

	output.TexCoord.x += 0.25 * texIndex;

	return output;
}

