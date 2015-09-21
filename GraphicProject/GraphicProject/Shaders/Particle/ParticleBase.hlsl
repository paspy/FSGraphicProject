
cbuffer cbPerFrame {
	float3 gCameraPosW;

	// for when the emit position/direction is varying
	float3 gEmitPosW;
	float3 gEmitDirW;

	float gGameTime;
	float gTimeStep;
	float4x4 gViewProj;
};

cbuffer cbFixed {
	// Net constant acceleration used to accerlate the particles.
	float3 gAccelW = { -1.0f, -9.8f, 0.0f };
};

// Array of textures for texturing the particles.
Texture2DArray gTexArray;

// Random texture used to generate random numbers in shaders.
Texture1D gRandomTex;

SamplerState samLinear {
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = WRAP;
	AddressV = WRAP;
};

DepthStencilState DisableDepth {
	DepthEnable = FALSE;
	DepthWriteMask = ZERO;
};

DepthStencilState NoDepthWrites {
	DepthEnable = TRUE;
	DepthWriteMask = ZERO;
};


//***********************************************
// HELPER FUNCTIONS                             *
//***********************************************
float3 RandUnitVec3(float offset) {
	// Use game time plus offset to sample random texture.
	float u = (gGameTime + offset);

	// coordinates in [-1,1]
	float3 v = gRandomTex.SampleLevel(samLinear, u, 0).xyz;

	// project onto unit sphere
	return normalize(v);
}

float3 RandVec3(float offset) {
	// Use game time plus offset to sample random texture.
	float u = (gGameTime + offset);

	// coordinates in [-1,1]
	float3 v = gRandomTex.SampleLevel(samLinear, u, 0).xyz;

	return v;
}

//***********************************************
// STREAM-OUT TECH                              *
//***********************************************

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
[maxvertexcount(6)]
void StreamOutGS(point Particle gsInput[1], inout PointStream<Particle> ptStream) {
	gsInput[0].Age += gTimeStep;

	if (gsInput[0].Type == PT_EMITTER) {
		// time to emit a new particle?
		if (gsInput[0].Age > 0.002f) {
			for (int i = 0; i < 5; ++i) {
				// Spread rain drops out above the camera.
				float3 vRandom = 35.0f*RandVec3((float)i / 5.0f);
				vRandom.y = 20.0f;

				Particle p;
				p.InitialPosW = gEmitPosW.xyz + vRandom;
				p.InitialVelW = float3(0.0f, 0.0f, 0.0f);
				p.SizeW = float2(1.0f, 1.0f);
				p.Age = 0.0f;
				p.Type = PT_FLARE;

				ptStream.Append(p);
			}

			// reset the time to emit
			gsInput[0].Age = 0.0f;
		}

		// always keep emitters
		ptStream.Append(gsInput[0]);
	} else {
		// Specify conditions to keep particle; this may vary from system to system.
		if (gsInput[0].Age <= 3.0f)
			ptStream.Append(gsInput[0]);
	}
}

struct VS_OUTPUT {
	float3 PositionW  : POSITION;
	uint   Type		  : TYPE;
};

VS_OUTPUT VSMain(Particle vsInput) {
	VS_OUTPUT vsOutput;

	float t = vsInput.Age;

	// constant acceleration equation
	vsOutput.PositionW = 0.5f*t*t*gAccelW + t*vsInput.InitialVelW + vsInput.InitialPosW;

	vsOutput.Type = vsInput.Type;

	return vsOutput;
}

struct GS_OUTPUT {
	float4 PositionH  : SV_Position;
	float2 TexCoord   : TEXCOORD;
};

// The draw GS just expands points into lines.
[maxvertexcount(2)]
void GSMain(point VS_OUTPUT gsInput[1], inout LineStream<GS_OUTPUT> lineStream) {
	// do not draw emitter particles.
	if (gsInput[0].Type != PT_EMITTER) {
		// Slant line in acceleration direction.
		float3 p0 = gsInput[0].PositionW;
		float3 p1 = gsInput[0].PositionW + 0.07f*gAccelW;

		GS_OUTPUT v0;
		v0.PositionH = mul(float4(p0, 1.0f), gViewProj);
		v0.TexCoord = float2(0.0f, 0.0f);
		lineStream.Append(v0);

		GS_OUTPUT v1;
		v1.PositionH = mul(float4(p1, 1.0f), gViewProj);
		v1.TexCoord = float2(1.0f, 1.0f);
		lineStream.Append(v1);
	}
}

float4 PSMain(GS_OUTPUT psInput) : SV_TARGET {
	return gTexArray.Sample(samLinear, float3(psInput.TexCoord, 0));
}
