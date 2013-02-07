//--------------------------------------------------------------------------------------
// File: Test.cpp
//
// Empty starting point for new Direct3D 9 and/or Direct3D 11 applications
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#include "DXUT.h"
#include "DXUTcamera.h"
#include "DXUTsettingsdlg.h"
#include "DXUTgui.h"
#include "SDKmesh.h"
#include "SDKmisc.h"
#include "resource.h"
#include <D3DX11.h>
#include "Quadtree.h"
#include "TextureHandler.h"
#include "Tessellator.h"

//--------------------------------------------------------------------------------------
//Macros
//--------------------------------------------------------------------------------------
#define FLOAT_RANDOM(x)  ( ((x)*rand()) /RAND_MAX)


//--------------------------------------------------------------------------------------
//Structures
//--------------------------------------------------------------------------------------
struct CONSTANT_BUFFER_STRUCT
{
    D3DXMATRIXA16    mWorld;                         // World matrix
    D3DXMATRIXA16    mView;                          // View matrix
    D3DXMATRIXA16    mProjection;                    // Projection matrix
    D3DXMATRIXA16    mWorldViewProjection;           // WVP matrix
    D3DXMATRIXA16    mViewProjection;                // VP matrix
    D3DXMATRIXA16    mInvView;                       // Inverse of view matrix
    D3DXVECTOR4      vScreenResolution;              // Screen resolution
    D3DXVECTOR4      vMeshColor;                     // Mesh color
    D3DXVECTOR4      vTessellationFactor;            // Edge, inside, minimum tessellation factor and 1/desired triangle size
    D3DXVECTOR4      vDetailTessellationHeightScale; // Height scale for detail tessellation of grid surface
    D3DXVECTOR4      vGridSize;                      // Grid size
    D3DXVECTOR4      vDebugColorMultiply;            // Debug colors
    D3DXVECTOR4      vDebugColorAdd;                 // Debug colors
    D3DXVECTOR4      vFrustumPlaneEquation[4];       // View frustum plane equations
};

struct CB_PER_FRAME_CONSTANTS
{
	D3DXMATRIX mViewProjection;
	D3DXMATRIX mWorld;
	D3DXMATRIX mLightViewProjection;
	D3DXVECTOR4 vLightPos;
	D3DXVECTOR3 vCameraPosWorld;
	float fTessellationFactor;
};

struct DETAIL_TESSELLATION_TEXTURE_STRUCT
{
    WCHAR* DiffuseMap;                  // Diffuse texture map
    WCHAR* NormalHeightMap;             // Normal and height map (normal in .xyz, height in .w)
    WCHAR* DisplayName;                 // Display name of texture
    float  fHeightScale;                // Height scale when rendering
    float  fBaseTextureRepeat;          // Base repeat of texture coordinates (1.0f for no repeat)
    float  fDensityScale;               // Density scale (used for density map generation)
    float  fMeaningfulDifference;       // Meaningful difference (used for density map generation)
};

//--------------------------------------------------------------------------------------
//Globals
//--------------------------------------------------------------------------------------
//DXUT Stuff
CDXUTDialogResourceManager              g_DialogResourceManager;
CFirstPersonCamera						g_Camera;
CFirstPersonCamera						g_LightCamera;
CD3DSettingsDlg							g_D3DSettingsDlg;
CDXUTDialog								g_HUD;
CDXUTDialog								g_SampleUI;
CDXUTTextHelper*						g_pTxtHelper = NULL;

ID3D11Texture2D*						ShadowMap;		
ID3D11Texture2D*						g_tRenderTarget;
ID3D11RenderTargetView*					g_pRTV;
ID3D11RenderTargetView*					g_pRTVMap;
ID3D11Texture2D*						g_tDepthStencil;
ID3D11DepthStencilView*					g_pDSV;
ID3D11ShaderResourceView*				g_pSRV;

D3DXVECTOR3								g_vLightPosition1 = D3DXVECTOR3(1100.0f, 250.0f, 1100.0f);
D3DXMATRIX								g_mLightViewProjection;
ID3D11InputLayout*						g_pPatchLayout = NULL;
ID3D11Buffer*							g_pVertexBuffer;
ID3D11Buffer*							g_pIndexBuffer;
ID3D11RasterizerState*					g_pRasterizerStateSolid = NULL;
ID3D11RasterizerState*					g_pRasterizerStateWireFrame = NULL;
ID3D11RasterizerState*					g_pRasterizerStateCurrent = NULL;
ID3D11SamplerState*						g_pShadowSampler;

ID3D11Buffer*                       g_pcbPerFrame = NULL;
UINT                                g_iBindPerFrame = 0;

//Vertex List
VertexPositionColor* Vertices;
int* indices;

float									g_fTessellationFactor = 0.0f;
Quadtree								g_Helper("D:\\Downloads\\Heightmap.bmp", 1024, 1024);
TextureHandler							g_TexHandler;
//Shaders
ID3D11VertexShader*						g_pTessVS = NULL;
ID3D11HullShader*						g_pTessHS = NULL;
ID3D11DomainShader*						g_pTessDS = NULL;
ID3D11PixelShader*						g_pTessPS = NULL;

ID3D11VertexShader*						g_pShadowVS = NULL;
ID3D11PixelShader*						g_pShadowPS = NULL;
ID3D11HullShader*						g_pShadowHS = NULL;
ID3D11DomainShader*						g_pShadowDS = NULL;

ID3D11VertexShader*						g_pShadowedVS = NULL;
ID3D11PixelShader*						g_pShadowedPS = NULL;
ID3D11HullShader*						g_pShadowedHS = NULL;
ID3D11DomainShader*						g_pShadowedDS = NULL;

ID3D11VertexShader*						g_pParticleVS = NULL;
ID3D11GeometryShader*					g_pParticleGS = NULL;
ID3D11PixelShader*						g_pParticlePS = NULL;

ID3D11VertexShader*						g_pBaseVS = NULL;

Tessellator g_Tessellator;

//--------------------------------------------------------------------------------------
// Extern declarations 
//--------------------------------------------------------------------------------------
extern bool CALLBACK IsD3D9DeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat,
                                             bool bWindowed, void* pUserContext );
extern HRESULT CALLBACK OnD3D9CreateDevice( IDirect3DDevice9* pd3dDevice,
                                            const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext );
extern HRESULT CALLBACK OnD3D9ResetDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc,
                                           void* pUserContext );
extern void CALLBACK OnD3D9FrameRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime,
                                        void* pUserContext );
extern void CALLBACK OnD3D9LostDevice( void* pUserContext );
void CALLBACK OnD3D11ReleasingSwapChain(void* pUserContext);

extern void CALLBACK OnD3D9DestroyDevice( void* pUserContext );

void InitApp();

//--------------------------------------------------------------------------------------
// Reject any D3D11 devices that aren't acceptable by returning false
//--------------------------------------------------------------------------------------
bool CALLBACK IsD3D11DeviceAcceptable( const CD3D11EnumAdapterInfo *AdapterInfo, UINT Output, const CD3D11EnumDeviceInfo *DeviceInfo,
                                       DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext )
{
    return true;
}


//--------------------------------------------------------------------------------------
// Called right before creating a D3D9 or D3D11 device, allowing the app to modify the device settings as needed
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext )
{
    return true;
}

//--------------------------------------------------------------------------------------
//Find and compile a shader
//--------------------------------------------------------------------------------------
HRESULT CompileShaderFromFile(WCHAR* szFileName, D3D_SHADER_MACRO* pDefines, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut )
{
	HRESULT hr = S_OK;

	WCHAR str[MAX_PATH];
	V_RETURN( DXUTFindDXSDKMediaFileCch(str, MAX_PATH, szFileName));

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;

#if defined(DEBUG) || defined (_DEBUG)
	dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif
	ID3DBlob* pErrorBlob;
	hr = D3DX11CompileFromFile( str, pDefines, NULL, szEntryPoint, szShaderModel, dwShaderFlags, 0, NULL, ppBlobOut, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		if (pErrorBlob!=NULL)
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer() );
		SAFE_RELEASE(pErrorBlob);
		return hr;
	}
	SAFE_RELEASE( pErrorBlob );

	return S_OK;
}



//--------------------------------------------------------------------------------------
// Create any D3D11 resources that aren't dependant on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11CreateDevice( ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
                                      void* pUserContext )
{
	HRESULT hr;

	ID3D11DeviceContext* pd3dImmediateContext = DXUTGetD3D11DeviceContext();
	V_RETURN( g_DialogResourceManager.OnD3D11CreateDevice( pd3dDevice, pd3dImmediateContext));
	V_RETURN(g_D3DSettingsDlg.OnD3D11CreateDevice( pd3dDevice));
	V_RETURN( g_Tessellator.OnD3D11CreateDevice(pd3dDevice));
	g_pTxtHelper = new CDXUTTextHelper(pd3dDevice, pd3dImmediateContext, &g_DialogResourceManager, 15);

	ID3DBlob* pBlobBaseVS = NULL;
	ID3DBlob* pBlobVS = NULL;
	ID3DBlob* pBlobShadowVS = NULL;
	ID3DBlob* pBlobShadowedVS = NULL;

	ID3DBlob* pBlobHS = NULL;
	ID3DBlob* pBlobShadowHS = NULL;
	ID3DBlob* pBlobShadowedHS = NULL;

	ID3DBlob* pBlobDS = NULL;
	ID3DBlob* pBlobShadowDS = NULL;
	ID3DBlob* pBlobShadowedDS = NULL;

	ID3DBlob* pBlobPS = NULL;
	ID3DBlob* pBlobShadowPS = NULL;
	ID3DBlob* pBlobShadowedPS = NULL;

	g_TexHandler.Initialize(pd3dDevice, 1280, 960);
	D3D_SHADER_MACRO fracEvenPartitioning[] = { { "TERRAIN_HS_PARTITION", "\"fractional_even\""}, {0} };

	V_RETURN( CompileShaderFromFile(L"Shaders.hlsl", NULL, "TessVS", "vs_5_0", &pBlobVS ));
	V_RETURN( CompileShaderFromFile(L"ShadowMap.hlsl", NULL, "ShadowMapVS", "vs_5_0", &pBlobShadowVS));
	V_RETURN( CompileShaderFromFile(L"ShadowMap.hlsl", NULL, "ShadowedVS", "vs_5_0", &pBlobShadowedVS));

	V_RETURN( CompileShaderFromFile(L"Shaders.hlsl", fracEvenPartitioning, "TessHS", "hs_5_0", &pBlobHS ));
	V_RETURN( CompileShaderFromFile(L"ShadowMap.hlsl", NULL, "ShadowMapHS", "hs_5_0", &pBlobShadowHS));
	V_RETURN( CompileShaderFromFile(L"ShadowMap.hlsl", NULL, "ShadowedHS", "hs_5_0", &pBlobShadowedHS));

	V_RETURN( CompileShaderFromFile(L"Shaders.hlsl", NULL, "TessDS", "ds_5_0", &pBlobDS ));
	V_RETURN( CompileShaderFromFile(L"ShadowMap.hlsl", NULL, "ShadowMapDS", "ds_5_0", &pBlobShadowDS));
	V_RETURN( CompileShaderFromFile(L"ShadowMap.hlsl", NULL, "ShadowedDS", "ds_5_0", &pBlobShadowedDS));

	V_RETURN( CompileShaderFromFile(L"Shaders.hlsl", NULL, "TessPS", "ps_5_0", &pBlobPS ));
	V_RETURN( CompileShaderFromFile(L"ShadowMap.hlsl", NULL, "ShadowMapPS", "ps_5_0", &pBlobShadowPS));
	V_RETURN( CompileShaderFromFile(L"ShadowMap.hlsl", NULL, "ShadowedPS", "ps_5_0", &pBlobShadowedPS));

	V_RETURN( CompileShaderFromFile(L"Shaders.hlsl", NULL, "BaseVS", "vs_5_0", &pBlobBaseVS) );

	//Create Shaders
	V_RETURN( pd3dDevice->CreateVertexShader( pBlobVS->GetBufferPointer(), pBlobVS->GetBufferSize(), NULL, &g_pTessVS) );
	DXUT_SetDebugName( g_pTessVS, "TessVS");

	V_RETURN( pd3dDevice->CreateVertexShader( pBlobShadowVS->GetBufferPointer(), pBlobShadowVS->GetBufferSize(), NULL, &g_pShadowVS));
	DXUT_SetDebugName( g_pShadowVS, "Shadow Map VS");

	V_RETURN( pd3dDevice->CreateVertexShader( pBlobShadowedVS->GetBufferPointer(), pBlobShadowedVS->GetBufferSize(), NULL, &g_pShadowedVS));
	DXUT_SetDebugName( g_pShadowedVS, "Shadowed VS");

	V_RETURN( pd3dDevice->CreateHullShader( pBlobHS->GetBufferPointer(), pBlobHS->GetBufferSize(), NULL, &g_pTessHS) );
	DXUT_SetDebugName( g_pTessHS, "TessHS");

	V_RETURN( pd3dDevice->CreateHullShader( pBlobShadowHS->GetBufferPointer(), pBlobShadowHS->GetBufferSize(), NULL, &g_pShadowHS));
	DXUT_SetDebugName( g_pShadowHS, "Shadow HS");

	V_RETURN( pd3dDevice->CreateHullShader( pBlobShadowedHS->GetBufferPointer(), pBlobShadowedHS->GetBufferSize(), NULL, &g_pShadowedHS));
	DXUT_SetDebugName( g_pShadowedHS, "Shadowed HS");

	V_RETURN( pd3dDevice->CreateDomainShader( pBlobDS->GetBufferPointer(), pBlobDS->GetBufferSize(), NULL, &g_pTessDS) );
	DXUT_SetDebugName( g_pTessDS, "TessDS");

	V_RETURN( pd3dDevice->CreateDomainShader( pBlobShadowDS->GetBufferPointer(), pBlobShadowDS->GetBufferSize(), NULL, &g_pShadowDS) );
	DXUT_SetDebugName( g_pShadowDS, "Shadow DS");

	V_RETURN( pd3dDevice->CreateDomainShader( pBlobShadowedDS->GetBufferPointer(), pBlobShadowedDS->GetBufferSize(), NULL, &g_pShadowedDS) );
	DXUT_SetDebugName( g_pShadowedDS, "Shadowed DS");

	V_RETURN( pd3dDevice->CreatePixelShader( pBlobPS->GetBufferPointer(), pBlobPS->GetBufferSize(), NULL, &g_pTessPS) );
	DXUT_SetDebugName( g_pTessPS, "TessPS");

	V_RETURN( pd3dDevice->CreatePixelShader( pBlobShadowPS->GetBufferPointer(), pBlobShadowPS->GetBufferSize(), NULL, &g_pShadowPS) );
	DXUT_SetDebugName( g_pShadowPS, "Shadow Map PS");

	V_RETURN( pd3dDevice->CreatePixelShader( pBlobShadowedPS->GetBufferPointer(), pBlobShadowedPS->GetBufferSize(), NULL, &g_pShadowedPS) );
	DXUT_SetDebugName( g_pShadowedPS, "Shadowed PS");

	V_RETURN( pd3dDevice->CreateVertexShader( pBlobBaseVS->GetBufferPointer(), pBlobBaseVS->GetBufferSize(), NULL, &g_pBaseVS) );
	const D3D11_INPUT_ELEMENT_DESC patchLayout[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TANGENT", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0},

	};

	V_RETURN( pd3dDevice->CreateInputLayout(patchLayout, ARRAYSIZE(patchLayout), pBlobVS->GetBufferPointer(), pBlobVS->GetBufferSize(), &g_pPatchLayout));
	DXUT_SetDebugName(g_pPatchLayout, "Primary");

	SAFE_RELEASE(pBlobVS);
	SAFE_RELEASE(pBlobHS);
	SAFE_RELEASE(pBlobDS);
	SAFE_RELEASE(pBlobPS);

	SAFE_RELEASE(pBlobBaseVS);


	D3D11_BUFFER_DESC Desc;
	Desc.Usage = D3D11_USAGE::D3D11_USAGE_DYNAMIC;
	Desc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_CONSTANT_BUFFER;
	Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	Desc.MiscFlags = 0;

	Desc.ByteWidth = sizeof(CB_PER_FRAME_CONSTANTS);
	V_RETURN( pd3dDevice->CreateBuffer(&Desc, NULL, &g_pcbPerFrame ) );
	DXUT_SetDebugName( g_pcbPerFrame, "CB_PER_FRAME_CONSTANTS" );

	D3D11_RASTERIZER_DESC RasterDesc;
	ZeroMemory(&RasterDesc, sizeof(D3D11_RASTERIZER_DESC));

	RasterDesc.FillMode= D3D11_FILL_SOLID;
	RasterDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_BACK;
	RasterDesc.DepthClipEnable = FALSE;
	V_RETURN( pd3dDevice->CreateRasterizerState(&RasterDesc, &g_pRasterizerStateSolid ) );
	DXUT_SetDebugName( g_pRasterizerStateSolid, "Solid" );

	RasterDesc.FillMode = D3D11_FILL_WIREFRAME;
	V_RETURN(pd3dDevice->CreateRasterizerState( &RasterDesc, &g_pRasterizerStateWireFrame) );
	DXUT_SetDebugName(g_pRasterizerStateWireFrame, "Wireframe");

	D3D11_BUFFER_DESC vbDesc;
	ZeroMemory( &vbDesc, sizeof(D3D11_BUFFER_DESC) );
	vbDesc.ByteWidth = sizeof(VertexPositionColor) * g_Helper.numVertices;
	vbDesc.Usage = D3D11_USAGE_DEFAULT;
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA vbInitData;
	ZeroMemory(&vbInitData, sizeof(vbInitData));
	vbInitData.pSysMem = Vertices;
	V_RETURN( pd3dDevice->CreateBuffer( &vbDesc, &vbInitData, &g_pVertexBuffer) );
	DXUT_SetDebugName(g_pVertexBuffer, "Vertex Buffer");

	g_Tessellator.SetBaseMesh(pd3dDevice, pd3dImmediateContext, g_Helper.numVertices, g_pVertexBuffer);

	D3D11_BUFFER_DESC ibDesc;
	ZeroMemory(&ibDesc, sizeof(D3D11_BUFFER_DESC));
	ibDesc.ByteWidth = sizeof(int) * g_Helper.numIndices;
	ibDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
	ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA ibInitData;
	ZeroMemory(&ibInitData, sizeof(ibInitData));
	ibInitData.pSysMem = indices;
	V_RETURN( pd3dDevice->CreateBuffer(&ibDesc, &ibInitData, &g_pIndexBuffer) );
	DXUT_SetDebugName(g_pIndexBuffer, "Index Buffer");
	g_pRasterizerStateCurrent = g_pRasterizerStateSolid;


	//Create ShadowMap rendertargetview
	D3D11_TEXTURE2D_DESC textureDesc;
	textureDesc.Width = 1280;
	textureDesc.Height = 960;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	V_RETURN(pd3dDevice->CreateTexture2D(&textureDesc, NULL, &ShadowMap));

	D3D11_RENDER_TARGET_VIEW_DESC renderDesc;
	renderDesc.Format = textureDesc.Format;
	renderDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderDesc.Texture2D.MipSlice = 0;

	V_RETURN(pd3dDevice->CreateRenderTargetView(ShadowMap, &renderDesc, &g_pRTVMap));

	//Shader Resource for shadow map
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	shaderResourceViewDesc.Format = textureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	V_RETURN(pd3dDevice->CreateShaderResourceView(ShadowMap, &shaderResourceViewDesc, &g_pSRV));

	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER::D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	V_RETURN(pd3dDevice->CreateSamplerState(&sampDesc, &g_pShadowSampler));

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Create any D3D11 resources that depend on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                                          const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
	HRESULT hr;

	V_RETURN( g_DialogResourceManager.OnD3D11ResizedSwapChain( pd3dDevice, pBackBufferSurfaceDesc ) );
	V_RETURN( g_D3DSettingsDlg.OnD3D11ResizedSwapChain( pd3dDevice, pBackBufferSurfaceDesc ) );



	ID3D11Texture2D* pBackBuffer= NULL;
	V_RETURN(pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer));

	V_RETURN(pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_pRTV));

	D3D11_TEXTURE2D_DESC depthDesc;
	ZeroMemory(&depthDesc, sizeof(depthDesc));
	depthDesc.Width = 1280;
	depthDesc.Height = 960;
	depthDesc.MipLevels = 1;
	depthDesc.ArraySize = 1;
	depthDesc.Format = DXGI_FORMAT::DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthDesc.SampleDesc.Count = 1;
	depthDesc.SampleDesc.Quality = 0;
	depthDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
	depthDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_DEPTH_STENCIL;
	depthDesc.CPUAccessFlags = 0;
	depthDesc.MiscFlags = 0;

	V_RETURN(pd3dDevice->CreateTexture2D(&depthDesc, NULL, &g_tDepthStencil));

	D3D11_DEPTH_STENCIL_VIEW_DESC DSVDesc;
	ZeroMemory(&DSVDesc, sizeof(DSVDesc));
	DSVDesc.Format = depthDesc.Format;
	DSVDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	DSVDesc.Texture2D.MipSlice = 0;
	V_RETURN(pd3dDevice->CreateDepthStencilView(g_tDepthStencil, &DSVDesc, &g_pDSV));




	float fAspectRatio = pBackBufferSurfaceDesc->Width/(FLOAT)pBackBufferSurfaceDesc->Height;
	g_Camera.SetProjParams(D3DX_PI / 4, fAspectRatio, 0.1f, 2000.0f);
	g_LightCamera.SetProjParams(D3DX_PI/2, fAspectRatio, 0.1f, 2000.0f);

	g_HUD.SetLocation(pBackBufferSurfaceDesc->Width/2, -pBackBufferSurfaceDesc->Height/2);
	g_HUD.SetSize(1280, 960);
	
	
    return S_OK;
}


void UpdateLightData()
{
	float LightPower = 1.0f;
	D3DXMATRIX Helper;
	D3DXMATRIX Helper2;
	D3DXMatrixLookAtLH(&Helper2, &g_vLightPosition1, &D3DXVECTOR3(0.0f, 0.0f, 0.0f), &D3DXVECTOR3(0, 1, 0));
	D3DXMatrixPerspectiveFovLH(&Helper, D3DX_PI/2, 0.9f, 5.0f, 1000.0f); 
	g_mLightViewProjection = Helper2 * Helper;
}

//--------------------------------------------------------------------------------------
// Handle updates to the scene.  This is called regardless of which D3D API is used
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext )
{
	g_Camera.FrameMove( fElapsedTime);
	//g_LightCamera.FrameMove(fElapsedTime);
	//UpdateLightData();
}


//--------------------------------------------------------------------------------------
// Render the scene using the D3D11 device
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11FrameRender( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext,
                                  double fTime, float fElapsedTime, void* pUserContext )
{


	if (g_D3DSettingsDlg.IsActive())
	{
		g_D3DSettingsDlg.OnRender(fElapsedTime);
		return;
	}

	D3DXMATRIX mViewProjection;
	D3DXMATRIX mProj = *g_Camera.GetProjMatrix();
	D3DXMATRIX mView = *g_Camera.GetViewMatrix();

	D3DXMATRIX mLightViewProjection;
	D3DXMATRIX mLightProj = *g_LightCamera.GetProjMatrix();
	D3DXMATRIX mLightView = *g_LightCamera.GetViewMatrix();

	RECT* quad = new RECT();
	quad->bottom = 0;
	quad->top = -960/2;
	quad->right = -1280/2;
	quad->left = 0;
	D3DCOLOR bg = D3DCOLOR_RGBA(200, 200, 200, 255);
	//g_HUD.DrawRect(quad, bg);

	D3DXVECTOR3 vecEye(500.0f, 50.0f, 500.0f);
	D3DXMATRIX World = g_Helper.getWorldMatrix();
	mLightViewProjection = mLightView * mLightProj;
	mViewProjection = mView * mProj;

	D3D11_MAPPED_SUBRESOURCE MappedResource;
	pd3dImmediateContext->Map( g_pcbPerFrame, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
	CB_PER_FRAME_CONSTANTS* pData = (CB_PER_FRAME_CONSTANTS*)MappedResource.pData;

	D3DXMatrixTranspose( &pData->mViewProjection, &mViewProjection);
	pData->vCameraPosWorld = *(g_Camera.GetEyePt() );
	
	D3DXMatrixTranspose(&pData->mWorld, &World);
	//pData->mWorld = g_Helper.getWorldMatrix();
	pData->fTessellationFactor = (float)g_fTessellationFactor;
	pData->vLightPos = D3DXVECTOR4(g_vLightPosition1, 1);
	
	D3DXMATRIX Identity;
	D3DXMatrixIdentity(&Identity);
	D3DXMatrixTranspose( &pData->mLightViewProjection, &mLightViewProjection);
	pd3dImmediateContext->Unmap( g_pcbPerFrame, 0);

	pd3dImmediateContext->OMSetRenderTargets(1, &g_pRTVMap, g_pDSV);
    // Clear render target and the depth stencil 
    float ClearColor[4] = { 0.176f, 0.196f, 0.667f, 0.0f };
	float BlankColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f};
	//D3DXVECTOR4 ClearColor = D3DXVECTOR4(.176f, .196f, .667f, 0.0f); 
    //ID3D11RenderTargetView* pRTV = DXUTGetD3D11RenderTargetView();
    //ID3D11DepthStencilView* pDSV = DXUTGetD3D11DepthStencilView();
	//g_TexHandler.ClearTarget(pd3dImmediateContext, pDSV, ClearColor);
	pd3dImmediateContext->ClearRenderTargetView( g_pRTVMap, BlankColor );
    pd3dImmediateContext->ClearDepthStencilView( g_pDSV, D3D11_CLEAR_DEPTH, 1.0, 0 );
	//g_TexHandler.SetRenderTarget(pd3dImmediateContext, pDSV);
	//g_TexHandler.ClearTarget(pd3dImmediateContext, pDSV, ClearColor);

	pd3dImmediateContext->RSSetState( g_pRasterizerStateSolid);

	pd3dImmediateContext->VSSetConstantBuffers(g_iBindPerFrame, 1, &g_pcbPerFrame);
	pd3dImmediateContext->HSSetConstantBuffers(g_iBindPerFrame, 1, &g_pcbPerFrame);
	pd3dImmediateContext->DSSetConstantBuffers(g_iBindPerFrame, 1, &g_pcbPerFrame);
	pd3dImmediateContext->PSSetConstantBuffers(g_iBindPerFrame, 1, &g_pcbPerFrame);

	pd3dImmediateContext->VSSetShader(g_pShadowVS, NULL, 0);
	pd3dImmediateContext->HSSetShader(g_pShadowHS, NULL, 0);
	pd3dImmediateContext->DSSetShader(g_pShadowDS, NULL, 0);
	pd3dImmediateContext->GSSetShader(NULL, NULL, 0);
	pd3dImmediateContext->PSSetShader(g_pShadowPS, NULL, 0);

	pd3dImmediateContext->IASetInputLayout(g_pPatchLayout);
	UINT Stride = sizeof(VertexPositionColor);
	UINT Offset = 0;
	pd3dImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &Stride, &Offset);
	pd3dImmediateContext->IASetIndexBuffer(g_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);

	pd3dImmediateContext->DrawIndexed(g_Helper.numIndices, 0, 0);

	pd3dImmediateContext->OMSetRenderTargets(1, &g_pRTV, g_pDSV);
	
	//g_TexHandler.ClearTarget(pd3dImmediateContext, pDSV, ClearColor);
	//pRTV = DXUTGetD3D11RenderTargetView();
	//g_TexHandler.SetRenderTarget(pd3dImmediateContext, pDSV);
	
	
	pd3dImmediateContext->ClearRenderTargetView( g_pRTV, ClearColor );
	
    pd3dImmediateContext->ClearDepthStencilView( g_pDSV, D3D11_CLEAR_DEPTH, 1.0, 0 );

	ID3D11ShaderResourceView* In[] = {g_TexHandler.GetShaderResourceView()};
	ID3D11ShaderResourceView* nullView[] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};

	pd3dImmediateContext->VSSetShader(g_pShadowedVS, NULL, 0);
	pd3dImmediateContext->HSSetShader(g_pShadowedHS, NULL, 0);
	pd3dImmediateContext->DSSetShader(g_pShadowedDS, NULL, 0);
	pd3dImmediateContext->GSSetShader(NULL, NULL, 0);
	pd3dImmediateContext->PSSetShader(g_pShadowedPS, NULL, 0);
	pd3dImmediateContext->PSSetShaderResources(0, 1, &g_pSRV);
	pd3dImmediateContext->PSSetSamplers(0, 1, &g_pShadowSampler);

	pd3dImmediateContext->DrawIndexed(g_Helper.numIndices, 0, 0);

	//pd3dImmediateContext->PSGetShaderResources(0, 8, nullView);
	
	
	//DXUT_BeginPerfEvent(DXUT_PERFEVENTCOLOR, L"Screen");
	//g_HUD.OnRender(fElapsedTime);
	//DXUT_EndPerfEvent();

}


//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11ResizedSwapChain 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11ReleasingSwapChain( void* pUserContext )
{
	g_DialogResourceManager.OnD3D11ReleasingSwapChain();
}


//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11CreateDevice 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11DestroyDevice( void* pUserContext )
{

	SAFE_RELEASE(g_pPatchLayout);
	SAFE_RELEASE(g_pcbPerFrame);
	SAFE_RELEASE(g_pTessVS);
	SAFE_RELEASE(g_pTessHS);
	SAFE_RELEASE(g_pTessDS);
	SAFE_RELEASE(g_pTessPS);
	SAFE_RELEASE(g_pShadowVS);
	SAFE_RELEASE(g_pShadowHS);
	SAFE_RELEASE(g_pShadowDS);
	SAFE_RELEASE(g_pShadowPS);
	SAFE_RELEASE(g_pShadowedVS);
	SAFE_RELEASE(g_pShadowedHS);
	SAFE_RELEASE(g_pShadowedDS);
	SAFE_RELEASE(g_pShadowedPS);
	SAFE_RELEASE(g_pRasterizerStateSolid);
//	SAFE_RELEASE(g_pRasterizerStateCurrent);
	SAFE_RELEASE(g_pRasterizerStateWireFrame);
	
	SAFE_RELEASE(g_pVertexBuffer);
	SAFE_RELEASE(g_pIndexBuffer);
	g_DialogResourceManager.OnD3D11DestroyDevice();
	g_D3DSettingsDlg.OnD3D11DestroyDevice();
	DXUTGetGlobalResourceCache().OnDestroyDevice();
	g_TexHandler.Shutdown();

}


//--------------------------------------------------------------------------------------
// Handle messages to the application
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
                          bool* pbNoFurtherProcessing, void* pUserContext )
{
	*pbNoFurtherProcessing = g_DialogResourceManager.MsgProc( hWnd, uMsg, wParam, lParam);
	if (*pbNoFurtherProcessing)
		return 0;

	if ( g_D3DSettingsDlg.IsActive())
	{
		g_D3DSettingsDlg.MsgProc(hWnd, uMsg, wParam, lParam);
		return 0;
	}

	*pbNoFurtherProcessing = g_HUD.MsgProc(hWnd, uMsg, wParam, lParam );
	if (*pbNoFurtherProcessing)
		return 0;
	*pbNoFurtherProcessing = g_SampleUI.MsgProc(hWnd, uMsg, wParam, lParam);
	if (*pbNoFurtherProcessing)
		return 0;

	g_Camera.HandleMessages(hWnd, uMsg, wParam, lParam);
	g_LightCamera.HandleMessages(hWnd, uMsg, wParam, lParam);
	return 0;
}


//--------------------------------------------------------------------------------------
// Handle key presses
//--------------------------------------------------------------------------------------
void CALLBACK OnKeyboard( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext )
{
	switch(nChar)
	{
	case 'U':
		if (g_fTessellationFactor < 100)
			g_fTessellationFactor+=2;
	case 'Y':
		if (g_fTessellationFactor > 1)
			g_fTessellationFactor--;
	/*case 'K':
		if (g_pRasterizerStateCurrent == g_pRasterizerStateSolid)
			g_pRasterizerStateCurrent = g_pRasterizerStateWireFrame;
		else
			g_pRasterizerStateCurrent = g_pRasterizerStateSolid;*/
	}


}


//--------------------------------------------------------------------------------------
// Handle mouse button presses
//--------------------------------------------------------------------------------------
void CALLBACK OnMouse( bool bLeftButtonDown, bool bRightButtonDown, bool bMiddleButtonDown,
                       bool bSideButton1Down, bool bSideButton2Down, int nMouseWheelDelta,
                       int xPos, int yPos, void* pUserContext )
{

}


//--------------------------------------------------------------------------------------
// Call if device was removed.  Return true to find a new device, false to quit
//--------------------------------------------------------------------------------------
bool CALLBACK OnDeviceRemoved( void* pUserContext )
{
    return true;
}


//--------------------------------------------------------------------------------------
// Initialize everything and go into a render loop
//--------------------------------------------------------------------------------------
int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

    // DXUT will create and use the best device (either D3D9 or D3D11) 
    // that is available on the system depending on which D3D callbacks are set below

    // Set general DXUT callbacks
    DXUTSetCallbackFrameMove( OnFrameMove );
    DXUTSetCallbackKeyboard( OnKeyboard );
    DXUTSetCallbackMouse( OnMouse );
    DXUTSetCallbackMsgProc( MsgProc );
    DXUTSetCallbackDeviceChanging( ModifyDeviceSettings );
    DXUTSetCallbackDeviceRemoved( OnDeviceRemoved );

    // Set the D3D9 DXUT callbacks. Remove these sets if the app doesn't need to support D3D9
  /*  DXUTSetCallbackD3D9DeviceAcceptable( IsD3D9DeviceAcceptable );
    DXUTSetCallbackD3D9DeviceCreated( OnD3D9CreateDevice );
    DXUTSetCallbackD3D9DeviceReset( OnD3D9ResetDevice );
    DXUTSetCallbackD3D9FrameRender( OnD3D9FrameRender );
    DXUTSetCallbackD3D9DeviceLost( OnD3D9LostDevice );
    DXUTSetCallbackD3D9DeviceDestroyed( OnD3D9DestroyDevice );*/

    // Set the D3D11 DXUT callbacks. Remove these sets if the app doesn't need to support D3D11
    DXUTSetCallbackD3D11DeviceAcceptable( IsD3D11DeviceAcceptable );
    DXUTSetCallbackD3D11DeviceCreated( OnD3D11CreateDevice );
    DXUTSetCallbackD3D11SwapChainResized( OnD3D11ResizedSwapChain );
    DXUTSetCallbackD3D11FrameRender( OnD3D11FrameRender );
    DXUTSetCallbackD3D11SwapChainReleasing( OnD3D11ReleasingSwapChain );
    DXUTSetCallbackD3D11DeviceDestroyed( OnD3D11DestroyDevice );

    // Perform any application-level initialization here

    DXUTInit( true, true, NULL ); // Parse the command line, show msgboxes on error, no extra command line params
    DXUTSetCursorSettings( true, true ); // Show the cursor and clip it when in full screen
    DXUTCreateWindow( L"Test" );
	InitApp();
	Vertices = g_Helper.getVertices();
	indices = g_Helper.getIndices();
	g_Camera.SetScalers(.01f, 100.0f);
	g_LightCamera.SetScalers(.01f, 100.0f);
    // Only require 10-level hardware
    DXUTCreateDevice( D3D_FEATURE_LEVEL_11_0, true, 1280, 960 );
    DXUTMainLoop(); // Enter into the DXUT render loop

    // Perform any application-level cleanup here
	
    return DXUTGetExitCode();
}

void InitApp()
{
	g_D3DSettingsDlg.Init(&g_DialogResourceManager);
	g_HUD.Init(&g_DialogResourceManager);
	g_SampleUI.Init(&g_DialogResourceManager);


	RECT* quad = new RECT();
	quad->bottom = 0;
	quad->top = 100;
	quad->right = 100;
	quad->left = 0;
	D3DCOLOR bg = D3DCOLOR_RGBA(255, 255, 255, 255);
	g_HUD.DrawRect(quad, bg);
	quad = NULL;

	D3DXVECTOR3 vecEye(500.0f, 50.0f, 500.0f);
	D3DXVECTOR3 vecAt(0.0f, 0.0f, 0.0f);
	g_Camera.SetViewParams(&vecEye, &vecAt);
	g_LightCamera.SetViewParams(&g_vLightPosition1, &vecAt);
}



