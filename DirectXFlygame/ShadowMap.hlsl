//--------------------------------------
//Code for a shadow map pass
//--------------------------------------

#ifndef TERRAIN_HS_PARTITION
#define TERRAIN_HS_PARTITION "fractional_even"
#endif

#define TESS_NUM	4
#define	OUT_NUM		4


//----------------------------------------------
//Textures and Samplers
//----------------------------------------------
Texture2D txShadow : register( t0 );

SamplerComparisonState ShadowSampler : register( s0 );

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

struct VertexIn
{
	float3 Position		: POSITION;
};
struct VertexOut
{
	float4 Position		: POSITION;
	float3 Position2D	: TEXCOORD0;
};

struct HS_OUT
{
	float4 Position		: POSITION;
	float3 Position2D	: TEXCOORD0;
};
struct HS_DATA_OUT
{
	float Edges[4]		: SV_TessFactor;
	float Inside[2]		: SV_InsideTessFactor;
};

struct DS_OUTPUT
{
	float4 Position		: SV_POSITION;
};
//-----------------------------------------------
//Shadow Map Vertex Shader
//-----------------------------------------------
VertexOut ShadowMapVS(VertexIn Input)
{
	VertexOut Output;
	Output.Position = mul(Input.Position.xyz, g_mWorld);
	Output.Position2D = float3(Output.Position.xyz);
	return Output;
}
//----------------------------------------------
//Shadow Map Hull Shader
//---------------------------------------------

HS_DATA_OUT DefaultHS( InputPatch<VertexOut, TESS_NUM > ip, uint PatchID : SV_PrimitiveID )
{
	HS_DATA_OUT Output;
	

	//float4 Position = mul(float4(ip[0].Position2D, 1), g_mLightViewProjection);

	//float Scalar = Position.z/Position.w;

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
HS_OUT ShadowMapHS(InputPatch<VertexOut, TESS_NUM> p, uint i : SV_OutputControlPointID, uint PatchID : SV_PrimitiveID )
{
	HS_OUT Output;
	Output.Position = p[i].Position;
	Output.Position2D = p[i].Position2D;

	return Output;
}

//------------------------------------------------
//Shadow Map Domain Shader
//------------------------------------------------
[domain("quad")]
DS_OUTPUT ShadowMapDS( HS_DATA_OUT input, float2 UV : SV_DomainLocation, const OutputPatch<HS_OUT, OUT_NUM> patch )
{
	DS_OUTPUT Output;
	float3 verticalPos1 = lerp(patch[0].Position2D, patch[1].Position2D, UV.y);
	float3 verticalPos2 = lerp(patch[2].Position2D, patch[3].Position2D, UV.y);
	float3 finalPos = lerp(verticalPos1, verticalPos2, UV.x);
	Output.Position = mul(float4(finalPos, 1), g_mLightViewProjection);
	return Output;
}

//------------------------------------------------
//Shadow Map Pixel Shader
//------------------------------------------------
float4 ShadowMapPS(DS_OUTPUT Input) : SV_TARGET
{
	float4 Color = Input.Position.z/Input.Position.w;
	//return float4(0, 1, 0, 0) * 50f;
	return Color * 200.0f + 0.1f;
}
//---------------------------------------------------------------
//Shadowed Landscape
//---------------------------------------------------------------
struct SVIn
{
	float3 Position		: POSITION;
	float4 Color		: COLOR;
	float3 Normal		: NORMAL;
};
struct SVOut
{
	float4 Position		: POSITION;
	float4 Color		: COLOR;
	float3 Normal		: NORMAL;
};
struct SHSOut
{
	float3 Position		: POSITION;
	float4 Color		: COLOR;
	float3 Normal		: NORMAL;
};
struct SDSOut
{
	float4 Position		: SV_POSITION;
	float3 finalPos		: POSITION;
	float4 PosAsSeenLight : TEXCOORD0;
	float4 Color		: COLOR;
	float3 Normal		: NORMAL;
};
//------------------------------------------------
//Shadowed Vertex Shader
//------------------------------------------------
SVOut ShadowedVS(SVIn Input)
{
	SVOut Output;
	Output.Position.xyz = mul(Input.Position.xyz, g_mWorld);
	Output.Color = Input.Color;
	Output.Normal = Input.Normal;
	float fDistance = distance(Output.Position, g_vCameraPosWorld);
	float fMinDistance = 1.0f;
	float fMaxDistance = 250.0f;
	float4 Factors = float4(1.0f, 1.0f, 2.0f, .05f);
	float distFactor = 1.0f - clamp( ( ( fDistance - fMinDistance ) / ( fMaxDistance - fMinDistance ) ), 
                                             0.0f, 1.0f - Factors.z/Factors.x);
	Output.Position.w = distFactor;

	return Output;
}
//-------------------------------------------------
//Shadowed Hull Shader
//-------------------------------------------------
HS_DATA_OUT DefaultHSTwo( InputPatch<SVOut, TESS_NUM > ip, uint PatchID : SV_PrimitiveID )
{
	HS_DATA_OUT Output;
	

	float4 Factors = float4(1.0f, 1.0f, 2.0f, .05f);
	float4 vEdgeTessFactors = Factors.xxxx;

	
	//vEdgeTessFactors.x = .5f * ( ip[0].Position.w + ip[1].Position.w);
	vEdgeTessFactors.y = .5f * ( ip[1].Position.w + ip[2].Position.w);
	vEdgeTessFactors.z = .5f * ( ip[2].Position.w + ip[3].Position.w);
	vEdgeTessFactors.w = .5f * ( ip[3].Position.w + ip[0].Position.w);
	
	
	vEdgeTessFactors = 1.0f * vEdgeTessFactors ;

	float Inside1 = vEdgeTessFactors.x * Factors.y;
	float Inside2 = vEdgeTessFactors.z * Factors.y;

	float TessAmount = g_fTessellationFactor;



	Output.Edges[0] = vEdgeTessFactors.x;
	Output.Edges[1] = vEdgeTessFactors.y;
	Output.Edges[2] = vEdgeTessFactors.z;
	Output.Edges[3] = vEdgeTessFactors.w;
	Output.Inside[0] = Inside1;
	Output.Inside[1] = Inside2;

	return Output;
}

[domain("quad")]
[partitioning(TERRAIN_HS_PARTITION)]
[outputtopology("triangle_ccw")]
[outputcontrolpoints(OUT_NUM)]
[patchconstantfunc("DefaultHSTwo")]
SHSOut ShadowedHS(InputPatch<SVOut, TESS_NUM> p, uint i : SV_OutputControlPointID, uint PatchID : SV_PrimitiveID )
{
	SHSOut Output;
	Output.Position = p[i].Position.xyz;
	Output.Color = p[i].Color;
	Output.Normal = p[i].Normal;

	return Output;
}
//--------------------------------------------------
//Shadowed Domain SHader
//--------------------------------------------------
[domain("quad")]
SDSOut ShadowedDS( HS_DATA_OUT input, float2 UV : SV_DomainLocation, const OutputPatch<SHSOut, OUT_NUM> patch )
{
	SDSOut Output;
	float3 verticalPos1 = lerp(patch[0].Position, patch[1].Position, UV.y);
	float3 verticalPos2 = lerp(patch[2].Position, patch[3].Position, UV.y);
	float3 finalPos = lerp(verticalPos1, verticalPos2, UV.x);
	
	Output.PosAsSeenLight = mul(float4(finalPos, 1), g_mLightViewProjection);
	Output.Color = float4(normalize(finalPos), 1);
	
	float3 vertNorm = lerp(patch[0].Normal, patch[1].Normal, UV.y);
	float3 vertNorm2 = lerp(patch[2].Normal, patch[3].Normal, UV.y);
	float3 finalNorm = lerp(vertNorm, vertNorm2, UV.x);
	Output.Normal = normalize(finalNorm);

	finalPos = finalPos + finalNorm;

	Output.Position = mul(float4(finalPos, 1), g_mViewProjection);
	Output.finalPos = finalPos;
	return Output;
}
//---------------------------------------------------
//Shadowed Pixel SHader
//---------------------------------------------------

float3 DotProduct(float3 One, float3 Two, float3 Normal)
{
	float3 Direction = normalize(Two - One);
	return dot(-Direction, Normal);
}

float Exponent(float In, float Power)
{
	float result = 1;
	for (int x = 0; x < Power; x++)
	{
		result = result * In;
	}

	return result;
}

float CalcSpecular(float4 Color, float4 LightSpec, float3 Normal, float3 Position, float Power)
{
	float3 newPos = g_vLightPos.xyz;
	float3 Direction = normalize(Position - newPos);
	float3 Halfway = normalize(normalize(g_vCameraPosWorld.xyz - Position) + Direction);

	float d = abs(distance(Position, newPos));
	float Atten = 1/(5 * pow(d, 2) + 34 * d + 15);

	float rho = dot(normalize(-Direction), normalize(Direction));
	float Spot = (cos(.43) - cos(.34))/(cos(.39) - cos(.34));

	return Color * LightSpec * pow(dot(Normal, Halfway), Power) * Atten * Spot;

}

float4 ShadowedPS(SDSOut Input) : SV_TARGET
{
	float4 Output;
	Output = Input.Color;
	float diffFactor = DotProduct(g_vLightPos.xyz, Input.finalPos.xyz, Input.Normal);
	diffFactor = saturate(diffFactor);
	diffFactor *= 1.0f;
	float4 diffColor = diffFactor * Output;

	//float specFactor = CalcSpecular(Output, float4(1, 1, 1, 1), Input.Normal, Input.Position.xyz, 100.0f);

	float3 Reflection = normalize(2 * dot(Input.Position.xyz, Input.Normal) * Input.Normal - Input.Position);
	float RdotL = saturate(dot(Reflection, Input.PosAsSeenLight.xyz));
	float specFactor = pow(RdotL, 1.0f)/12.5;
	float4 specColor = specFactor * Output;

	float2 ProjectedCoords;
	ProjectedCoords[0] = Input.PosAsSeenLight.x/Input.PosAsSeenLight.w/2.0f + 0.5f;
	ProjectedCoords[1] = -Input.PosAsSeenLight.y/Input.PosAsSeenLight.w/2.0f + 0.5f;
	
	float Depth = 1.5f;
	return  (Output * .02 + diffColor + specColor) * (1- 2.0 * txShadow.SampleCmpLevelZero(ShadowSampler, ProjectedCoords, 10.5f));
}

