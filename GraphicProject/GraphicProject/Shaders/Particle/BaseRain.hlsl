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

//***********************************************
// HELPER FUNCTIONS                             *
//***********************************************
float3 RandVec3(float offset) {
	// Use game time plus offset to sample random texture.
	float u = (gGameTime + offset);

	// coordinates in [-1,1]
	float3 v = gRandomTex.SampleLevel(LinearSamplerState, u, 0).xyz;

	return v;
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
	float3 accelW = { -1.0f, -9.8f, 0.0f };

	float t = vsInput.Age;

	// constant acceleration equation
	vsOutput.PositionW = 0.5f*t*t*accelW + t*vsInput.InitialVelW + vsInput.InitialPosW;

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
	float3 accelW = { -1.0f, -9.8f, 0.0f };
	// do not draw emitter particles.
	if (gsInput[0].Type != PT_EMITTER) {
		// Slant line in acceleration direction.
		float3 p0 = gsInput[0].PositionW;
		float3 p1 = gsInput[0].PositionW + 0.07f*accelW;

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
	return gTexArray.Sample(LinearSamplerState, float3(psInput.TexCoord, 0));
}
