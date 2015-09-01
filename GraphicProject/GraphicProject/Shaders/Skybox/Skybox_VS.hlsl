
cbuffer ConstPerObject {
	float4x4 WVP;
	float4x4 World;
	int texIndex;
};

TextureCube SkyMap;

struct SKYMAP_VS_OUTPUT	{
	float4 Position : SV_POSITION;
	float3 texCoord : TEXCOORD;
};


SKYMAP_VS_OUTPUT main(float3 inPos : POSITION,
					  float2 inTexCoord : TEXCOORD) {
	SKYMAP_VS_OUTPUT output = (SKYMAP_VS_OUTPUT)0;

	//Set Pos to xyww instead of xyzw, so that z will always be 1 (furthest from camera)
	output.Position = mul(float4(inPos, 1.0f), WVP).xyww;

	output.texCoord = inPos;

	return output;
}
