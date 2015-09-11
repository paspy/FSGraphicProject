
cbuffer ConstPerObject {
	float4x4 WorldViewProj;
	float4x4 World;
};

TextureCube SkyMap;
SamplerState ObjSamplerState;

struct VS_OUTPUT {
	float4 Position : SV_POSITION;
	float3 TexCoord : TEXCOORD;
};


VS_OUTPUT VSMain(float3 inPos : POSITION, float2 inTexCoord : TEXCOORD) {
	VS_OUTPUT output = (VS_OUTPUT)0;

	//Set Pos to xyww instead of xyzw, so that z will always be 1 (furthest from camera)
	output.Position = mul(float4(inPos, 1.0f), WorldViewProj).xyww;

	output.TexCoord = inPos;

	return output;
}

float4 PSMain(VS_OUTPUT input) : SV_Target {
	return SkyMap.Sample(ObjSamplerState, input.TexCoord);
}
