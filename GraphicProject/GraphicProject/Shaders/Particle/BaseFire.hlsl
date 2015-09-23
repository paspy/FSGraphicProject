cbuffer cbPerFrame: register(b0) {
	float4 gCameraPosW;

	// for when the emit position/direction is varying
	float4 gEmitPosW;
	float4 gEmitDirW;

	float gGameTime;
	float gTimeStep;
	float Pad1;
	float Pad2;

	float4x4 gViewProj;
};

// Array of textures for texturing the particles.
Texture2DArray gTexArray: register(t0);

// Random texture used to generate random numbers in shaders.
Texture1D gRandomTex: register(t1);

SamplerState LinearSamplerState: register(s0);

//DepthStencilState DisableDepth {
//	DepthEnable = FALSE;
//	DepthWriteMask = ZERO;
//};
//
//DepthStencilState NoDepthWrites {
//	DepthEnable = TRUE;
//	DepthWriteMask = ZERO;
//};
//
//BlendState AdditiveBlending {
//	AlphaToCoverageEnable = FALSE;
//	BlendEnable[0] = TRUE;
//	SrcBlend = SRC_ALPHA;
//	DestBlend = ONE;
//	BlendOp = ADD;
//	SrcBlendAlpha = ZERO;
//	DestBlendAlpha = ZERO;
//	BlendOpAlpha = ADD;
//	RenderTargetWriteMask[0] = 0x0F;
//};

//***********************************************
// HELPER FUNCTIONS                             *
//***********************************************
float3 RandUnitVec3(float offset) {
	// Use game time plus offset to sample random texture.
	float u = (gGameTime + offset);

	// coordinates in [-1,1]
	float3 v = gRandomTex.SampleLevel(LinearSamplerState, u, 0).xyz;

	// project onto unit sphere
	return normalize(v);
}

// STREAM-OUT

#define PT_EMITTER 0
#define PT_FLARE 1

struct Particle {
	float3 InitialPosW : POSITION;
	float3 InitialVelW : VELOCITY;
	float2 SizeW       : SIZE;
	float Age		   : AGE;
	uint Type          : TYPE;
};

Particle StreamOutVS(Particle vsInput) {
	return vsInput;
}

// The stream-out GS is just responsible for emitting 
// new particles and destroying old particles.  The logic
// programed here will generally vary from particle system
// to particle system, as the destroy/spawn rules will be 
// different.
[maxvertexcount(2)]
void StreamOutGS(point Particle gsInput[1], inout PointStream<Particle> ptStream) {
	gsInput[0].Age += gTimeStep;

	if (gsInput[0].Type == PT_EMITTER) {
		// time to emit a new particle?
		if (gsInput[0].Age > 0.005f) {
			float3 vRandom = RandUnitVec3(0.0f);
			vRandom.x *= 0.5f;
			vRandom.z *= 0.5f;

			Particle p;
			p.InitialPosW = gEmitPosW.xyz;
			p.InitialVelW = 4.0f*vRandom;
			p.SizeW = float2(3.0f, 3.0f);
			p.Age = 0.0f;
			p.Type = PT_FLARE;

			ptStream.Append(p);

			// reset the time to emit
			gsInput[0].Age = 0.0f;
		}

		// always keep emitters
		ptStream.Append(gsInput[0]);
	} else {
		// Specify conditions to keep particle; this may vary from system to system.
		if (gsInput[0].Age <= 1.0f)
			ptStream.Append(gsInput[0]);
	}
}

struct VS_OUTPUT {
	float3 PositionW  : POSITION;
	float2 SizeW	  : SIZE;
	float4 Color	  : COLOR;
	uint   Type		  : TYPE;
};

VS_OUTPUT VSMain(Particle vsInput) {
	float3 gAccelW = { 0.0f, 7.8f, 0.0f };
	VS_OUTPUT vout;

	float t = vsInput.Age;

	// constant acceleration equation
	vout.PositionW = 0.5f*t*t*gAccelW + t*vsInput.InitialVelW + vsInput.InitialPosW;

	// fade color with time
	float opacity = 1.0f - smoothstep(0.0f, 1.0f, t / 1.0f);
	vout.Color = float4(1.0f, 1.0f, 1.0f, opacity);

	vout.SizeW = vsInput.SizeW;
	vout.Type = vsInput.Type;

	return vout;
}

struct GS_OUTPUT {
	float4 PositionH  : SV_Position;
	float4 Color	  : COLOR;
	float2 TexCoord   : TEXCOORD;
};

// The draw GS just expands points into camera facing quads.
[maxvertexcount(4)]
void GSMain(point VS_OUTPUT gsInput[1], inout TriangleStream<GS_OUTPUT> triStream) {
	float2 gQuadTexC[4] = {
		float2(0.0f, 1.0f),
		float2(1.0f, 1.0f),
		float2(0.0f, 0.0f),
		float2(1.0f, 0.0f)
	};
	// do not draw emitter particles.
	if (gsInput[0].Type != PT_EMITTER) {

		// Compute world matrix so that billboard faces the camera.
		float3 look = normalize(gCameraPosW.xyz - gsInput[0].PositionW);
		float3 right = normalize(cross(float3(0, 1, 0), look));
		float3 up = cross(look, right);

		// Compute triangle strip vertices (quad) in world space.
		float halfWidth = 0.5f*gsInput[0].SizeW.x;
		float halfHeight = 0.5f*gsInput[0].SizeW.y;

		float4 v[4];
		v[0] = float4(gsInput[0].PositionW + halfWidth*right - halfHeight*up, 1.0f);
		v[1] = float4(gsInput[0].PositionW + halfWidth*right + halfHeight*up, 1.0f);
		v[2] = float4(gsInput[0].PositionW - halfWidth*right - halfHeight*up, 1.0f);
		v[3] = float4(gsInput[0].PositionW - halfWidth*right + halfHeight*up, 1.0f);

		// Transform quad vertices to world space and output them as a triangle strip.
		GS_OUTPUT gout;
		[unroll]
		for (int i = 0; i < 4; ++i) {
			gout.PositionH = mul(v[i], gViewProj);
			gout.TexCoord = gQuadTexC[i];
			gout.Color = gsInput[0].Color;
			triStream.Append(gout);
		}
	}
}

float4 PSMain(GS_OUTPUT psInput) : SV_TARGET {
	return gTexArray.Sample(LinearSamplerState, float3(psInput.TexCoord, 0))*psInput.Color;
}
