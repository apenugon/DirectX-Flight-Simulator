//-------------------------------------------------------------------------------------
//File: Shaders.hlsl
//
//Created by Akul Penugonda
//-------------------------------------------------------------------------------------

#ifndef TERRAIN_HS_PARTITION
#define TERRAIN_HS_PARTITION "fractional_even"
#endif

#define TESS_NUM	4
#define	OUT_NUM		4

//----------------------------------------------
//Textures and Samplers
//----------------------------------------------
Texture2D txShadow : register( t0 );

SamplerState samLinear : register( s0 );


//-----------------------------------------------
//CBuffers
//-----------------------------------------------
cbuffer cbPerFrame : register( b0 )
{
	matrix g_mViewProjection;
	matrix g_mWorld;
	matrix g_mLightViewProjection;
	float3 g_vLightPos;
	float3 g_vCameraPosWorld;
	float g_fTessellationFactor;
};

//------------------------------------------------
//Vertex Shader
//------------------------------------------------
struct VS_INPUT
{
	float3 vPosition		: POSITION;
	float4 Color			: COLOR;
	float3 Normal			: NORMAL;
	float3 Tangent			: TANGENT;
	float3 Bitangent		: BITANGENT;
};

struct VS_OUT
{
	float3 vPosition		: POSITION;
	float4 Color			: COLOR;
	float3 Normal			: NORMAL;
	float3 vLightTS			: LIGHTVECTORTS;
};

VS_OUT TessVS(VS_INPUT Input)
{
	VS_OUT output;
	float4 vPositionWS = mul(Input.vPosition.xyz, g_mWorld);
	float3 vLightWS = float3(500.0f, 100.0f, 500.0f) - vPositionWS.xyz;

	vLightWS.z = -vLightWS.z;

	float3 Normal = mul(Input.Normal, (float3x3)g_mWorld);
	float3 Tangent = mul(Input.Tangent, (float3x3)g_mWorld);
	float3 Bitangent = mul(Input.Bitangent, (float3x3)g_mWorld);

	Normal = normalize(Normal);
	Tangent = normalize(Tangent);
	Bitangent = normalize(Bitangent);

	float3x3 mWorldToTangent = float3x3(Tangent, Bitangent, Normal);

	float3 vLightTS = mul(mWorldToTangent, vLightWS);
	output.vLightTS = vLightTS;

	output.Color = Input.Color;
	output.Normal = Normal;

	output.vPosition = float3(vPositionWS.xyz);
	return output;
}

//-------------------------------------------------
//Hull Shader
//-------------------------------------------------
struct HS_DATA_OUT
{
	float Edges[4]		: SV_TessFactor;
	float Inside[2]		: SV_InsideTessFactor;
};

struct HS_OUTPUT
{
	float3 vPosition	: POSITION;
	float4 Color		: COLOR;
	float3 Normal		: NORMAL;
	float3 vLightTS		: LIGHTVECTORTS;
};

HS_DATA_OUT DefaultHS( InputPatch<VS_OUT, TESS_NUM > ip, uint PatchID : SV_PrimitiveID )
{
	HS_DATA_OUT Output;

	float TessAmount = g_fTessellationFactor;

	Output.Edges[0] = Output.Edges[1] = Output.Edges[2] = Output.Edges[3] = TessAmount;
	Output.Inside[0] = Output.Inside[1] = TessAmount;

	return Output;
}

[domain("quad")]
[partitioning(TERRAIN_HS_PARTITION)]
[outputtopology("triangle_ccw")]
[outputcontrolpoints(OUT_NUM)]
[patchconstantfunc("DefaultHS")]
HS_OUTPUT TessHS(InputPatch<VS_OUT, TESS_NUM> p, uint i : SV_OutputControlPointID, uint PatchID : SV_PrimitiveID )
{
	HS_OUTPUT Output;
	Output.vPosition = p[i].vPosition;
	Output.Color = p[i].Color;
	Output.Normal = p[i].Normal;
	Output.vLightTS = p[i].vLightTS;

	return Output;
}

//----------------------------------------------
//Domain Shader
//----------------------------------------------



struct DS_OUTPUT
{
    float4 vPosition        : SV_POSITION;
	float4 Color			: COLOR;
  //  float3 vWorldPos        : WORLDPOS;
    float3 vNormal          : NORMAL;
	float3 vLightTS			: LIGHTVECTORTS;			
};

[domain("quad")]
DS_OUTPUT TessDS( HS_DATA_OUT input, float2 UV : SV_DomainLocation, const OutputPatch<HS_OUTPUT, OUT_NUM> patch )
{

	DS_OUTPUT Output;
	float3 verticalPos1 = lerp(patch[0].vPosition, patch[1].vPosition, UV.y);
	float3 verticalPos2 = lerp(patch[2].vPosition, patch[3].vPosition, UV.y);
	float3 finalPos = lerp(verticalPos1, verticalPos2, UV.x);

	Output.vPosition = mul( float4(finalPos, 1), g_mViewProjection);
	Output.Color = patch[0].Color;

	float3 verticalNorm1 = lerp(patch[0].Normal, patch[1].Normal, UV.y);
	float3 verticalNorm2 = lerp(patch[2].Normal, patch[3].Normal, UV.y);
	float3 finalNorm = lerp(verticalPos1, verticalPos2, UV.x);

	finalNorm = normalize(finalNorm);

	float3 vLightTS1 = lerp(patch[0].vLightTS, patch[1].vLightTS, UV.y);
	float3 vLightTS2 = lerp(patch[2].vLightTS, patch[3].vLightTS, UV.y);
	float3 vLightTS = lerp(vLightTS1, vLightTS2, UV.x);

	finalPos += finalNorm * (100.0f);

	//Output.vNormal = mul( float4(finalNorm, 1), g_mViewProjection);
	Output.vNormal = finalNorm;
	Output.vPosition = mul(float4(finalPos.xyz, 1.0f), g_mViewProjection);
	Output.vLightTS = vLightTS;


	return Output;
}
float4 ComputeIllumination( float3 vLightTS, float3 vNormal, float4 Color )
{
   // Sample the normal from the normal map for the given texture sample:
   float3 vNormalTS = vNormal;
   
   // Sample base map
   float4 cBaseColor = Color;
   
   // Compute diffuse color component:
   float4 cDiffuse = saturate( dot( vNormalTS, vLightTS ) ) * float4(.5, .1, .5, 1.0f);
   
   // Compute the specular component if desired:  
   float4 cSpecular = .0001;


   
   // Composite the final color:
   float4 cFinalColor = ( float4(.1, .1, .1, .1) + cDiffuse ) * cBaseColor + cSpecular; 
   
   return cFinalColor;  
}  
//-----------------------------------------------
//Pixel Shader
//-----------------------------------------------
float4 TessPS (DS_OUTPUT Input) : SV_TARGET
{
	float4 cResultColor = float4(0, 0, 0, 1);
	float3 vViewTS = float3(0, 0, 0);

	float3 vLightTS = normalize(Input.vLightTS);
	
	cResultColor = ComputeIllumination(vLightTS, Input.vNormal, Input.Color);
	//float3 N = normalize(Input.vNormal);
	//float3 L = float3(-1.0f, 1.0f, 1.0f);
	float lightingFactor = saturate(saturate(dot(normalize(Input.vNormal), float4(-100.0f, 100.0f, 100.0f, 100.0f))) + 0.0f);
	//return lightingFactor * Input.Color;
	return cResultColor;
}
//------------BaseVS-------------------------------
struct BaseVertex
{
	float4 pos : POSITION;
};


float4 BaseVS(BaseVertex input) : SV_POSITION
{
	return mul( input.pos, mul(g_mWorld, g_mViewProjection));
}