#include <D3DX11.h>
#include <D3D11.h>
#include <d3d11sdklayers.h>
#include <D3D11Shader.h>
#include "DXUT.h"

class TextureHandler
{
public:
	TextureHandler();
	
	bool Initialize(ID3D11Device*, int, int);

	void Shutdown();

	void SetRenderTarget(ID3D11DeviceContext*, ID3D11DepthStencilView*);
	void ClearTarget(ID3D11DeviceContext*, ID3D11DepthStencilView*, D3DXVECTOR4 );
	ID3D11ShaderResourceView* GetShaderResourceView();
	ID3D11Texture2D* m_renderTargetTexture;

public:

	ID3D11RenderTargetView* m_renderTargetView;
	ID3D11ShaderResourceView* m_shaderResourceView;
};