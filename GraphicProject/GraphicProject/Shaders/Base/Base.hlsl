
struct BaseLight {
	float3 direction;
	float4 ambient;
	float4 diffuse;
};

cbuffer CBuffer {
	float4x4 WVP;
	float4x4 World;
	float4 difColor;
	int hasTexture;
	int hasNormMap;
};

cbuffer ConstPerFrame {
	BaseLight baseLight;
};

Texture2D ObjTexture;
Texture2D ObjNormMap;
SamplerState ObjSamplerState;

struct VS_OUTPUT {
	float4 Position : SV_POSITION;
	float4 WorldPos : POSITION;
	float2 TexCoord : TEXCOORD;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
};


VS_OUTPUT VSMain(float4 inPos : POSITION, float2 inTexCoord : TEXCOORD, float3 inNormal : NORMAL, float3 inTangent : TANGENT) {

	VS_OUTPUT output;

	output.Position = mul(inPos, WVP);
	output.WorldPos = mul(inPos, World);
	output.Normal   = mul(inNormal, (float3x3)World); //output.Normal = mul(float4(inNormal, 0.0), World).rgb;
	output.Tangent  = mul(inTangent, (float3x3)World);

	output.TexCoord = inTexCoord;

	return output;
}

float4 PSMain(VS_OUTPUT input) : SV_TARGET {
	input.Normal = normalize(input.Normal);

	//Set diffuse color of material
	float4 diffuse = difColor;

	//If material has a diffuse texture map, set it now
	//if (hasTexture > 0.5f)
		diffuse = ObjTexture.Sample(ObjSamplerState, input.TexCoord);

	//If material has a normal map, we can set it now
	//if (hasNormMap == true) {
		//Load normal from normal map
		float4 normalMap = ObjNormMap.Sample(ObjSamplerState, input.TexCoord);

		//Change normal map range from [0, 1] to [-1, 1]
		normalMap = (2.0f*normalMap) - 1.0f;

		//Make sure tangent is completely orthogonal to normal
		//input.Tangent = normalize(input.Tangent - dot(input.Tangent, input.Normal)*input.Normal);

		//Create the biTangent
		float3 biTangent = cross(input.Normal, input.Tangent);

		//Create the "Texture Space"
		float3x3 texSpace = float3x3(input.Tangent, biTangent, input.Normal);

		//Convert normal from normal map to texture space and store in input.normal
		input.Normal = (normalize(mul((float3)normalMap, texSpace)));
	//}

	float3 finalColor = float3(0.0f, 0.0f, 0.0f);

	finalColor = (diffuse * baseLight.ambient).rgb;
	finalColor += (saturate(dot(baseLight.direction, input.Normal) * baseLight.diffuse * diffuse)).rgb;
	//finalColor = input.Tangent;
	return float4(finalColor, diffuse.a);

}