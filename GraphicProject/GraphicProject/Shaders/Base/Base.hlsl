
struct DirectionLight {
	float3 direction;
	float pad;
	float4 ambient;
	float4 diffuse;
};

struct PointLight {
	float3 position;
	float  range;
	float3 attenuation;
	float pad;
	float4 diffuse;
};

struct SpotLight {
	float3 position;
	float  range;
	float3 direction;
	float  cone;
	float3 attenuation;
	float pad;
	float4 diffuse;
};

cbuffer ConstPerFrame {
	DirectionLight directionLight;
	PointLight pointLight;
	SpotLight spotLight;
};

cbuffer CBuffer {
	float4x4 WVP;
	float4x4 World;
	float4 difColor;
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
	output.TexCoord = inTexCoord;
	output.Normal   = mul(inNormal, (float3x3)World); //output.Normal = mul(float4(inNormal, 0.0), World).rgb;
	output.Tangent  = mul(inTangent, (float3x3)World);

	return output;
}

float4 PSMain(VS_OUTPUT input) : SV_TARGET {
	input.Normal = normalize(input.Normal);

	//Set diffuse color of material
	float4 diffuse = difColor;

	diffuse = ObjTexture.Sample(ObjSamplerState, input.TexCoord);

	//Load normal from normal map
	float4 normalMap = ObjNormMap.Sample(ObjSamplerState, input.TexCoord);

	//Change normal map range from [0, 1] to [-1, 1]
	normalMap = (2.0f*normalMap) - 1.0f;

	//Make sure tangent is completely orthogonal to normal
	input.Tangent = normalize(input.Tangent - dot(input.Tangent, input.Normal)*input.Normal);

	//Create the biTangent
	float3 biTangent = cross(input.Normal, input.Tangent);

	//Create the "Texture Space"
	float3x3 texSpace = float3x3(input.Tangent, biTangent, input.Normal);

	//Convert normal from normal map to texture space and store in input.normal
	input.Normal = (normalize(mul((float3)normalMap, texSpace)));


	float3 finalColor = float3(0.0f, 0.0f, 0.0f);

	//****calculating direction light****//
	float3 directionLightColor = float3(0.0f, 0.0f, 0.0f);
	float3 lightDir = -normalize(directionLight.direction);
	float3 wnrm = normalize(input.Normal);
	directionLightColor = (saturate(dot(lightDir, wnrm) * diffuse * directionLight.diffuse)).rgb + (diffuse * directionLight.ambient).rgb;
	//****calculating direction light****//


	//****calculating point light****//
	float3 pointLightColor = float3(0.0f, 0.0f, 0.0f);
	float3 pointLightDir = normalize(pointLight.position - input.WorldPos);
	float pointDis = length(pointLight.position - input.WorldPos);
	if (pointDis > pointLight.range) {
		pointLightColor = float3(0.0f, 0.0f, 0.0f);
	} else {
		float pointLightRatio = saturate(dot(pointLightDir, input.Normal));
		pointLightColor += pointLightRatio * pointLight.diffuse * diffuse;
		//pointLightColor += 1.0f - saturate(length(pointLight.position - input.WorldPos) / pointLight.range);
		pointLightColor /= pointLight.attenuation[0] + (pointLight.attenuation[1] * pointDis) + (pointLight.attenuation[2] * (pointDis*pointDis));
	}
	//****calculating point light****//

	//****calculating spot light****//
	float3 spotLightColor = float3(0.0f, 0.0f, 0.0f);
	float3 spotLightDir = spotLight.position - input.WorldPos;
	float spotDis = length(spotLight.position - input.WorldPos);
	if (spotDis > spotLight.range) {
		spotLightColor = float3(0.0f, 0.0f, 0.0f);
	} else {
		spotLightDir /= spotDis;
		float spotLightAmount = dot(spotLightDir, input.Normal);
		spotLightColor += spotLightAmount * diffuse * spotLight.diffuse;
		spotLightColor /= spotLight.attenuation[0] + (spotLight.attenuation[1] * spotDis) + (spotLight.attenuation[2] * (spotDis*spotDis));
		spotLightColor *= pow(max(dot(-spotLightDir, spotLight.direction), 0.0f), spotLight.cone);
		
		//spotLightColor = saturate(spotLightColor);
	}
	//****calculating spot light****//
	finalColor = saturate(spotLightColor + pointLightColor + directionLightColor);

	return float4(finalColor, diffuse.a);

}