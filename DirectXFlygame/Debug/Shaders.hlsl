#ifndef TERRAIN_HS_PARTITION
#define TERRAIN_HS_PARTITION "fractional_even"
#endif

//-----------------------------------------------
//CBuffers
//-----------------------------------------------
cbuffer cbPerFrame : register( b0 )
{
	matrix g_mViewProjection;
	float3 g_vCameraPosWorld;
	float g_fTessellationFactor;
};

//------------------------------------------------
//Vertex Shader
//------------------------------------------------
struct VS_INPUT
{
	float3 vPosition		: POSITION;
};

struct VS_OUT
{
	float3 vPosition		: POSITION;
};

VS_OUT TessVS(VS_INPUT Input)
{
	VS_OUT output;
	output.vPosition = Input.vPosition;

	return output;
}

//-------------------------------------------------
//Hull Shader
//-------------------------------------------------
struct HS_DATA_OUT
{
	float Edges[4]		: SV_Tess_Factor;
	float Inside[2]		: SV_InsideTessFactor;
};

struct HS_OUTPUT
{
	float3 vPosition	: FINALPOS;
};

HS_DATA_OUT DefaultHS( InputPatch<VS_OUT, 16 > ip, uint PatchID : SV_PrimitiveID )
{
	HS_DATA_OUT Output;

	float TessAmount = g_fTessellationFactor;

	Output.Edges[0] = Output.Edges[1] = Output.Edges[2] = Output.Edges[3] = TessAmount;
	Output.Inside[0] = Output.Inside[1] = TessAmount'

	return Output;
}

[domain("quad")]
[partitioning(TERRAIN_HS_PARTITION)]
[outputtopology("triangle_cw")]
[outputcontrolpoints(16)]
[patchconstantfunc("DefaultHS")]
HS_OUTPUT TessHS(InputPatch<VS_OUT, 16> p, uint i : SV_OutputControlPointID, uint PatchID : SV_PrimitiveID )
{
	HS_OUTPUT Output;
	Output.vPosition = p[i].vPosition;
	return Output;
}

//----------------------------------------------
//Domain Shader
//----------------------------------------------
struct DS_OUTPUT
{
    float4 vPosition        : SV_POSITION;
    float3 vWorldPos        : WORLDPOS;
    float3 vNormal            : NORMAL;
};

[domain("quad")]
DS_OUTPUT TessDS( HS_DATA_OUT input, float2 UV : SV_DomainLocation, const OutputPatch<HS_OUTPUT, OUTPUT_PATCH_SIZE> bezpatch )
{
	DS_OUTPUT Output;
	Output.vPosition = mul(float4(UV, 1), g_mViewProjection);
	Output.vWorldPos = UV;
	Output.vNormal = float3(1, 1, 1);

	return Output;
}

//-----------------------------------------------
//Pixel Shader
//-----------------------------------------------
float4 TessPS (DS_OUTPUT Input) : SV_TARGET
{
	float3 N = normalize(Input.vNormal);
	float3 L = normalize(Input.vWorldPos - g_vCameraPosWorld);
	return abs(dot(N, L)) * float4(1, 0, 0, 1);
}
