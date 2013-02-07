float4x4 xWorld;
float4x4 xView;
float4x4 xProjection;
float3 xLightDirection;
bool xEnableLighting;
float xAmbient;

//Textures

Texture xRocks;
sampler GroundSampler = sampler_state
{
	texture = <xRocks>;
	magfilter = LINEAR;
	minfilter = LINEAR;
	mipfilter = LINEAR;
	AddressU = MIRROR;
	AddressV = MIRROR;
};
Texture xGrass;
sampler GrassSampler = sampler_state
{
	texture = <xGrass>;
	magfilter = LINEAR;
	minfilter = LINEAR;
	mipfilter = LINEAR;
	AddressU = MIRROR;
	AddressV = MIRROR;
};
Texture xDirt;
sampler DirtSampler = sampler_state
{
	texture = <xDirt>;
	magfilter = LINEAR;
	minfilter = LINEAR;
	mipfilter = LINEAR;
	AddressU = MIRROR;
	AddressV = MIRROR;
};

// TODO: add effect parameters here.

struct VertexToPixel
{
    float4 Position : POSITION;
    float4 Color    : COLOR0;
    float3 Normal   : TEXCOORD0;
    float2 TexCoord : TEXCOORD1;
    float4 LightDirection : TEXCOORD2;
    float4 TextureWeights : TEXCOORD3;

    // TODO: add input channels such as texture
    // coordinates and vertex colors here.
};

struct PixelToFrame
{
    float4 Color : COLOR0;

    // TODO: add vertex shader outputs such as colors and texture
    // coordinates here. These values will automatically be interpolated
    // over the triangle, and provided as input to your pixel shader.
};

VertexToPixel VertexShaderFunction(float4 inPos : POSITION, float2 inTexCoord : TEXCOORD0, float3 inNormal : NORMAL, float4 inTexWeights : TEXCOORD1)
{
    VertexToPixel output = (VertexToPixel)0;
	
	float4x4 preViewProjection = mul(xView, xProjection);
	float4x4 preWorldViewProjection = mul(xWorld, preViewProjection);
	
    output.Position = mul(inPos, preWorldViewProjection);
    output.Normal = mul(normalize(inNormal), xWorld);
    output.TexCoord = inTexCoord;
    output.LightDirection.xyz = -xLightDirection;
    output.LightDirection.w = 1;
    output.TextureWeights = inTexWeights;

    // TODO: add your vertex shader code here.

    return output;
}

PixelToFrame PixelShaderFunction(VertexToPixel PSIn)
{
    PixelToFrame Output = (PixelToFrame)0;
    
    float lightingFactor = saturate(saturate(dot(PSIn.Normal, PSIn.LightDirection)) + xAmbient);
    
    Output.Color = tex2D(GrassSampler, PSIn.TexCoord) * PSIn.TextureWeights.x;
    Output.Color += tex2D(GroundSampler, PSIn.TexCoord) * PSIn.TextureWeights.y;
    Output.Color += tex2D(DirtSampler, PSIn.TexCoord) * PSIn.TextureWeights.z;
    Output.Color *= lightingFactor;
    
    return Output;
}

technique Technique1
{
    pass Pass1
    {
        // TODO: set renderstates here.

        VertexShader = compile vs_2_0 VertexShaderFunction();
        PixelShader = compile ps_2_0 PixelShaderFunction();
    }
}
