
struct BaseLight {
	float3 direction;
	float3 position;
	float  range;
	float3 spotLightDir;
	float cone;
	float3 attenuation;
	float4 ambient;
	float4 diffuse;
};

cbuffer ConstPerObject {
	float4x4 WVP;
	float4x4 World;

	float4 difColor;
	bool hasTexture;
	bool hasNormMap;
};

cbuffer ConstPerFrame {
	BaseLight baseLight;
};

Texture2D ObjNormMap;
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

float4 main(VS_OUTPUT input) : SV_TARGET {
	input.Normal = normalize(input.Normal);

	//float4 diffuse = ObjTexture.Sample(ObjSamplerState, input.TexCoord);

	//Set diffuse color of material
	float4 diffuse = difColor;

	//If material has a diffuse texture map, set it now
	if (hasTexture == true)
		diffuse = ObjTexture.Sample(ObjSamplerState, input.TexCoord);

	//If material has a normal map, we can set it now
	if (hasNormMap == true) {
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
	}

	
	float3 finalColor = float3(0.0f, 0.0f, 0.0f);

	//Create the vector between light position and pixels position
	float3 lightToPixelVec = (float4(baseLight.position, 1.0) - input.WorldPos).xyz;

	//Find the distance between the light pos and pixel pos
	float d = length(lightToPixelVec);

	//Create the ambient light
	float3 finalAmbient = (diffuse * baseLight.ambient).xyz;

	//If pixel is too far, return pixel color with ambient light
	if (d > baseLight.range)
		return float4(finalAmbient, diffuse.a);

	//Turn lightToPixelVec into a unit length vector describing
	//the pixels direction from the lights position
	lightToPixelVec /= d;

	//Calculate how much light the pixel gets by the angle
	//in which the light strikes the pixels surface
	float howMuchLight = dot(lightToPixelVec, input.Normal);

	//If light is striking the front side of the pixel
	if (howMuchLight > 0.0f) {
		//Add light to the finalColor of the pixel

		finalColor += (howMuchLight * diffuse * baseLight.diffuse).rgb;

		//Calculate Light's Falloff factor
		finalColor /= baseLight.attenuation[0] + (baseLight.attenuation[1] * d) + (baseLight.attenuation[2] * (d*d));

		//Calculate falloff from center to edge of pointlight cone
		finalColor *= pow(max(dot(-lightToPixelVec, baseLight.spotLightDir), 0.0f), baseLight.cone);
	}

	//make sure the values are between 1 and 0, and add the ambient
	finalColor = saturate(finalColor + finalAmbient);

	//finalColor = (diffuse * baseLight.ambient).rgb;
	//finalColor += (saturate(dot(baseLight.direction, input.Normal) * baseLight.diffuse * diffuse)).rgb;

	if (input.Color.x == 0 &&
		input.Color.y == 0 &&
		input.Color.z == 0 &&
		input.Color.w == 1 ) {

		//Return Final Color
		return float4(finalColor, diffuse.a);
	} else {
		return input.Color;
	}

}