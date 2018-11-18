
#define MINF asfloat(0xff800000)

#define DEPTH_WORLD_MIN 0.1f
#define DEPTH_WORLD_MAX 10.0f

float cameraToKinectProjZ(float z)
{
	return (z - DEPTH_WORLD_MIN) / (DEPTH_WORLD_MAX - DEPTH_WORLD_MIN);
}

float kinectProjZToCamera(float z)
{
	return DEPTH_WORLD_MIN + z*(DEPTH_WORLD_MAX - DEPTH_WORLD_MIN);
}


Buffer<float> g_depth : register(t0);

cbuffer ConstantBuffer : register(b0)
{
	matrix g_mIntrinsicInverse;
	matrix g_mIntrinsicNew;
	matrix g_mExtrinsic;
	uint g_uWidth;
	uint g_uHeight;
	uint g_uWidthNew;
	uint g_uHeightNew;
	float g_fDepthThreshOffset;
	float g_fDepthThreshLin;
	float2 g_dummy123;
}


void vertexShaderMain()
{
}

struct GS_INPUT
{
};

struct PS_INPUT
{
	float4 vPosition : SV_POSITION;
	float  fDepth : DE;
};

struct PS_OUTPUT
{
	float  depth;
};

float4 getWorldSpacePosition(uint x, uint y)
{
	float d = g_depth[y*g_uWidth + x];
	float4 posCam = mul(float4((float)x*d, (float)y*d, d, d), g_mIntrinsicInverse);
		posCam = float4(posCam.x, posCam.y, posCam.z, 1.0f);

	float4 posWorld = mul(posCam, g_mExtrinsic);
		posWorld /= posWorld.w;

	return posWorld;
}

PS_INPUT ComputeQuadVertex(uint x, uint y)
{
	float d = g_depth[y*g_uWidth + x];
	float4 posWorldCC = getWorldSpacePosition(x, y);

	//float4 posWorlMC = getWorldSpacePosition(x - 1, y + 0);
	//float4 posWorlCM = getWorldSpacePosition(x + 0, y - 1);
	//float4 posWorlCP = getWorldSpacePosition(x + 0, y + 1);
	//float4 posWorlPC = getWorldSpacePosition(x + 1, y + 0);

	//float3 normal = cross(posWorlCP.xyz - posWorlCM.xyz, posWorlPC.xyz - posWorlMC.xyz);
	//	normal = normalize(normal);

	float4 posClip = mul(float4(posWorldCC.x, posWorldCC.y, posWorldCC.z, 1.0f), g_mIntrinsicNew);
		posClip = float4(posClip.x / posClip.z, posClip.y / posClip.z, posClip.z, 1.0f);

	// NOTE: The aspect ratio is not 2.0!
	float fx = ((float)posClip.x / (float)(g_uWidthNew - 1))*2.0f - 1.0f;
	//float fy = ((float)posClip.y / (float)(g_uHeightNew-1))*2.0f - 1.0f;
	float fy = 1.0f - ((float) posClip.y / (float) (g_uHeightNew - 1.0f))*2.0f;
	float fz = cameraToKinectProjZ(posClip.z);
	posClip.x = fx;
	posClip.y = fy;
	posClip.z = fz;
	posClip.w = 1.0f;

	PS_INPUT Out;
	//Out.camSpacePosition = posWorldCC;
	Out.vPosition = posClip;
	//Out.vNormal = float4(normal, 1.0f);
	//Out.vTexCoord = float2(0.0f, 0.0f);
	Out.fDepth = d;

	return Out;
}

PS_INPUT ComputeDebugQuadVertex(uint x, uint y) {
	float d = g_depth[y*g_uWidth + x];

	float4 posClip;
	float fx = ((float)x / (float)(g_uWidth - 1))*2.0f - 1.0f;
	float fy = 1.0f - ((float)y / (float)(g_uHeight - 1))*2.0f;
	float fz = cameraToKinectProjZ(d);
	
	posClip.x = fx;
	posClip.y = fy;
	posClip.z = fz;
	posClip.w = 1.0f;

	PS_INPUT Out;
	Out.vPosition = posClip;
	Out.fDepth = d;
	return Out;
}

bool isValidVertex(PS_INPUT v) {
	if (v.vPosition.x < -1.0f || v.vPosition.x > 1.0f) return false;
	if (v.vPosition.y < -1.0f || v.vPosition.y > 1.0f) return false;
	if (v.vPosition.z < 0.0f || v.vPosition.z > 1.0f) return false;

	return true;
}

[maxvertexcount(4)]
void geometryShaderMain(point GS_INPUT fake[1], uint quadIdx : SV_PrimitiveID, inout TriangleStream<PS_INPUT> OutStream)
{
	PS_INPUT Out = (PS_INPUT)0;

	uint x = quadIdx % g_uWidth;
	uint y = quadIdx / g_uWidth;

	if (x >= g_uWidth - 1) return;
	if (y >= g_uHeight - 1) return;

	float d0 = g_depth[(x + 0) + g_uWidth*(y + 0)];
	float d1 = g_depth[(x + 0) + g_uWidth*(y + 1)];
	float d2 = g_depth[(x + 1) + g_uWidth*(y + 0)];
	float d3 = g_depth[(x + 1) + g_uWidth*(y + 1)];

	if (d0 <= DEPTH_WORLD_MIN || d1 <= DEPTH_WORLD_MIN || d2 <= DEPTH_WORLD_MIN || d3 <= DEPTH_WORLD_MIN)	return;
	if (d0 == MINF || d1 == MINF || d2 == MINF || d3 == MINF)	return;

	float dmax = max(max(d0, d1), max(d2, d3));
	float dmin = min(min(d0, d1), min(d2, d3));

	float d = 0.5f*(dmax + dmin);
	if (dmax - dmin > g_fDepthThreshOffset + g_fDepthThreshLin*d)	return;

	//be aware of the order for CULLING
	PS_INPUT v0 = ComputeQuadVertex(x + 0, y + 1);
	PS_INPUT v1 = ComputeQuadVertex(x + 0, y + 0);
	PS_INPUT v2 = ComputeQuadVertex(x + 1, y + 1);
	PS_INPUT v3 = ComputeQuadVertex(x + 1, y + 0);

	if (isValidVertex(v0) && isValidVertex(v1) && isValidVertex(v2) && isValidVertex(v3)) {
		OutStream.Append(v0);
		OutStream.Append(v1);
		OutStream.Append(v2);
		OutStream.Append(v3);
	}

	//OutStream.Append(ComputeQuadVertex(x + 0, y + 1));
	//OutStream.Append(ComputeQuadVertex(x + 0, y + 0));
	//OutStream.Append(ComputeQuadVertex(x + 1, y + 1));
	//OutStream.Append(ComputeQuadVertex(x + 1, y + 0));
}

float4 pixelShaderMain(PS_INPUT input) : SV_Target
{
	PS_OUTPUT res;
	res.depth = input.fDepth;

	return float4(res, res, res, 1.0f);
}

