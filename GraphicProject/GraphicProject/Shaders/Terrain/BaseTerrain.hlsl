#include <DirLight.hlsl>

cbuffer cbPerFrame : register(b0) {
	DirectionalLight gDirLights;
	float3 gCameraPosW;

	float gMinDist; // When distance is minimum, the tessellation is maximum.
	float gMaxDist; // When distance is maximum, the tessellation is minimum.

	// Exponents for power of 2 tessellation.  The tessellation
	// range is [2^(gMinTess), 2^(gMaxTess)].  Since the maximum
	// tessellation is 64, this means gMaxTess can be at most 6
	// since 2^6 = 64.
	float gMinTess;
	float gMaxTess;

	float gTexelCellSpaceU;
	float gTexelCellSpaceV;
	float gWorldCellSpace;
	float2 gTexScale = 50.0f;

	float4 gWorldFrustumPlanes[6];
};

cbuffer cbPerObject : register(b1) {
	// Terrain coordinate specified directly 
	// at center of world space.

	float4x4 gViewProj;
	Material gMaterial;
};

// Nonnumeric values cannot be added to a cbuffer.
Texture2DArray gLayerMapArray : register(t0);
Texture2D gBlendMap: register(t1);
Texture2D gHeightMap: register(t2);

SamplerState LinearSamplerState: register(s0);
SamplerState HeightmapSamplerState: register(s1);

struct VS_INPUT {
	float3 PositionL : POSITION;
	float2 TexCoord  : TEXCOORD0;
	float2 BoundsY   : TEXCOORD1;
};

struct VS_OUTPUT {
	float3 PositionW : POSITION;
	float2 TexCoord  : TEXCOORD0;
	float2 BoundsY   : TEXCOORD1;
};

// Vertex Shader Entry Point
VS_OUTPUT VSMain(VS_INPUT vsInput) {

	VS_OUTPUT vsOutput = (VS_OUTPUT)0;

	// Terrain specified directly in world space.
	vsOutput.PositionW = vsInput.PositionL;

	// Displace the patch corners to world space.  This is to make 
	// the eye to patch distance calculation more accurate.
	vsOutput.PositionW.y = gHeightMap.SampleLevel(HeightmapSamplerState, vsInput.TexCoord, 0).r;

	// Output vertex attributes to next stage.
	vsOutput.TexCoord = vsInput.TexCoord;
	vsOutput.BoundsY = vsInput.BoundsY;

	return vsOutput;
}

float CalcTessFactor(float3 p) {
	float d = distance(p, gCameraPosW);

	// max norm in xz plane (useful to see detail levels from a bird's eye).
	//d = max( abs(p.x-gCameraPosW.x), abs(p.z-gCameraPosW.z) );

	float s = saturate((d - gMinDist) / (gMaxDist - gMinDist));

	return (pow(2, (lerp(gMaxTess, gMinTess, s))));
}

// Returns true if the box is completely behind (in negative half space) of plane.
bool AabbBehindPlaneTest(float3 center, float3 extents, float4 plane) {
	float3 n = abs(plane.xyz);

	// This is always positive.
	float r = dot(extents, n);

	// signed distance from center point to plane.
	float s = dot(float4(center, 1.0f), plane);

	// If the center point of the box is a distance of e or more behind the
	// plane (in which case s is negative since it is behind the plane),
	// then the box is completely in the negative half space of the plane.
	return ((s + r) < 0.0f);
}

// Returns true if the box is completely outside the frustum.
bool AabbOutsideFrustumTest(float3 center, float3 extents, float4 frustumPlanes[6]) {
	for (int i = 0; i < 6; ++i) {
		// If the box is completely behind any of the frustum planes
		// then it is outside the frustum.
		if (AabbBehindPlaneTest(center, extents, frustumPlanes[i])) {
			return true;
		}
	}

	return false;
}

struct PatchTess {
	float EdgeTess[4]   : SV_TessFactor;
	float InsideTess[2] : SV_InsideTessFactor;
};

PatchTess ConstantHS(InputPatch<VS_OUTPUT, 4> patch, uint patchID : SV_PrimitiveID) {
	PatchTess pt;

	// Frustum cull

	// We store the patch BoundsY in the first control point.
	float minY = patch[0].BoundsY.x;
	float maxY = patch[0].BoundsY.y;

	// Build axis-aligned bounding box.  patch[2] is lower-left corner
	// and patch[1] is upper-right corner.
	float3 vMin = float3(patch[2].PositionW.x, minY, patch[2].PositionW.z);
	float3 vMax = float3(patch[1].PositionW.x, maxY, patch[1].PositionW.z);

	float3 boxCenter = 0.5f*(vMin + vMax);
	float3 boxExtents = 0.5f*(vMax - vMin);

	if (AabbOutsideFrustumTest(boxCenter, boxExtents, gWorldFrustumPlanes)) {
		pt.EdgeTess[0] = 0.0f;
		pt.EdgeTess[1] = 0.0f;
		pt.EdgeTess[2] = 0.0f;
		pt.EdgeTess[3] = 0.0f;

		pt.InsideTess[0] = 0.0f;
		pt.InsideTess[1] = 0.0f;

		return pt;

	// Do normal tessellation based on distance.
	} else {
		// It is important to do the tess factor calculation based on the
		// edge properties so that edges shared by more than one patch will
		// have the same tessellation factor.  Otherwise, gaps can appear.

		// Compute midpoint on edges, and patch center
		float3 e0 = 0.5f*(patch[0].PositionW + patch[2].PositionW);
		float3 e1 = 0.5f*(patch[0].PositionW + patch[1].PositionW);
		float3 e2 = 0.5f*(patch[1].PositionW + patch[3].PositionW);
		float3 e3 = 0.5f*(patch[2].PositionW + patch[3].PositionW);
		float3  c = 0.25f*(patch[0].PositionW + patch[1].PositionW + patch[2].PositionW + patch[3].PositionW);

		pt.EdgeTess[0] = CalcTessFactor(e0);
		pt.EdgeTess[1] = CalcTessFactor(e1);
		pt.EdgeTess[2] = CalcTessFactor(e2);
		pt.EdgeTess[3] = CalcTessFactor(e3);

		pt.InsideTess[0] = CalcTessFactor(c);
		pt.InsideTess[1] = pt.InsideTess[0];

		return pt;
	}
}

// Hull Shader struct
struct HS_OUTPUT {
	float3 PositionW   : POSITION;
	float2 TexCoord    : TEXCOORD0;
};

// Hull Shader Entry Point
[domain("quad")]
[partitioning("fractional_even")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("ConstantHS")]
[maxtessfactor(64.0f)]
HS_OUTPUT HSMain(InputPatch<VS_OUTPUT, 4> p, uint i : SV_OutputControlPointID, uint patchId : SV_PrimitiveID) {
	HS_OUTPUT hsOutput;

	// Pass through shader.
	hsOutput.PositionW = p[i].PositionW;
	hsOutput.TexCoord = p[i].TexCoord;

	return hsOutput;
}


// Domain Shader struct
struct DS_OUTPUT {
	float4 PositionH  : SV_POSITION;
	float3 PositionW  : POSITION;
	float2 TexCoord   : TEXCOORD0;
	float2 TiledTex	  : TEXCOORD1;
};

// The domain shader is called for every vertex created by the tessellator.  
// It is like the vertex shader after tessellation.
[domain("quad")]
// Domain Shader Entry Point
DS_OUTPUT DSMain(PatchTess patchTess, float2 uv : SV_DomainLocation, const OutputPatch<HS_OUTPUT, 4> quad) {
	DS_OUTPUT dsOutput;

	// Bilinear interpolation.
	dsOutput.PositionW = lerp(
		lerp(quad[0].PositionW, quad[1].PositionW, uv.x),
		lerp(quad[2].PositionW, quad[3].PositionW, uv.x),
		uv.y
	);

	dsOutput.TexCoord = lerp(
		lerp(quad[0].TexCoord, quad[1].TexCoord, uv.x),
		lerp(quad[2].TexCoord, quad[3].TexCoord, uv.x),
		uv.y
	);

	// Tile layer textures over terrain.
	dsOutput.TiledTex = dsOutput.TexCoord*gTexScale;

	// Displacement mapping
	dsOutput.PositionW.y = gHeightMap.SampleLevel(HeightmapSamplerState, dsOutput.TexCoord, 0).r;

	// NOTE: We tried computing the normal in the shader using finite difference, 
	// but the vertices move continuously with fractional_even which creates
	// noticable light shimmering artifacts as the normal changes.  Therefore,
	// we moved the calculation to the pixel shader.  

	// Project to homogeneous clip space.
	dsOutput.PositionH = mul(float4(dsOutput.PositionW, 1.0f), gViewProj);

	return dsOutput;
}


// Pixel Shader Entry Point
float4 PSMain(DS_OUTPUT psInput) : SV_Target {
	//
	// Estimate normal and tangent using central differences.
	//
	float2 leftTex = psInput.TexCoord + float2(-gTexelCellSpaceU, 0.0f);
	float2 rightTex = psInput.TexCoord + float2(gTexelCellSpaceU, 0.0f);
	float2 bottomTex = psInput.TexCoord + float2(0.0f, gTexelCellSpaceV);
	float2 topTex = psInput.TexCoord + float2(0.0f, -gTexelCellSpaceV);

	float leftY = gHeightMap.SampleLevel(HeightmapSamplerState, leftTex, 0).r;
	float rightY = gHeightMap.SampleLevel(HeightmapSamplerState, rightTex, 0).r;
	float bottomY = gHeightMap.SampleLevel(HeightmapSamplerState, bottomTex, 0).r;
	float topY = gHeightMap.SampleLevel(HeightmapSamplerState, topTex, 0).r;

	float3 tangent = normalize(float3(2.0f*gWorldCellSpace, rightY - leftY, 0.0f));
	float3 biTangent = normalize(float3(0.0f, bottomY - topY, -2.0f*gWorldCellSpace));
	float3 normalW = cross(tangent, biTangent);

	// The toCamera vector is used in lighting.
	float3 toCamera = gCameraPosW - psInput.PositionW;

	// Cache the distance to the eye from this surface point.
	float distToCam = length(toCamera);

	// Normalize.
	toCamera /= distToCam;

	// Texturing

	// Sample layers in texture array.
	float4 c0 = gLayerMapArray.Sample(LinearSamplerState, float3(psInput.TiledTex, 0.0f));
	float4 c1 = gLayerMapArray.Sample(LinearSamplerState, float3(psInput.TiledTex, 1.0f));
	float4 c2 = gLayerMapArray.Sample(LinearSamplerState, float3(psInput.TiledTex, 2.0f));
	float4 c3 = gLayerMapArray.Sample(LinearSamplerState, float3(psInput.TiledTex, 3.0f));
	float4 c4 = gLayerMapArray.Sample(LinearSamplerState, float3(psInput.TiledTex, 4.0f));

	// Sample the blend map.
	float4 t = gBlendMap.Sample(LinearSamplerState, psInput.TexCoord);

	// Blend the layers on top of each other.
	float4 texColor = c0;
	texColor = lerp(texColor, c1, t.r);
	texColor = lerp(texColor, c2, t.g);
	texColor = lerp(texColor, c3, t.b);
	texColor = lerp(texColor, c4, t.a);

	// Lighting.

	float4 litColor = texColor;
	float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 specular = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float4 A, D, S;
	ComputeDirectionalLight(gMaterial, gDirLights, normalW, toCamera, A, D, S);

	ambient += A;
	diffuse += D;
	specular += S;

	litColor = texColor*(ambient + diffuse) + specular;

	return texColor;


}