struct DirectionalLight {
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;
	float3 Direction;
	float Pad;
};

struct Material {
	float4 Ambient;
	float4 Diffuse;
	float4 Specular; // w = SpecPower
	float4 Reflect;
};

void ComputeDirectionalLight(
	in Material mat, in DirectionalLight light, in float3 normal, in float3 toCamera,
	out float4 ambient, out float4 diffuse, out float4 specular) {
	// Initialize outputs.
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	specular = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// The light vector aims opposite the direction the light rays travel.
	float3 lightVec = -light.Direction;

	// Add ambient term.
	ambient = mat.Ambient * light.Ambient;

	float diffuseRatio = saturate(dot(lightVec, normal));

	if (diffuseRatio > 0.0f) {
		float3 v = reflect(-lightVec, normal);
		float specFactor = pow(max(dot(v, toCamera), 0.0f), mat.Specular.w);

		diffuse = diffuseRatio * mat.Diffuse * light.Diffuse;
		specular = specFactor * mat.Specular * light.Specular;
	}
}

// Transforms a normal map sample to world space.
float3 NormalSampleToWorldSpace(float3 normalMapSample, float3 unitNormalW, float3 tangentW) {
	// Uncompress each component from [0,1] to [-1,1].
	float3 normalT = 2.0f*normalMapSample - 1.0f;

	// Build orthonormal basis.
	float3 N = unitNormalW;
	float3 T = normalize(tangentW - dot(tangentW, N)*N);
	float3 B = cross(N, T);

	float3x3 TBN = float3x3(T, B, N);

	// Transform from tangent space to world space.
	float3 bumpedNormalW = mul(normalT, TBN);

	return bumpedNormalW;
}