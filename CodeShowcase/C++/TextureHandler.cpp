#include "DXUT.h"
#include "TextureHandler.h"

TextureHandler::TextureHandler()
{
	m_renderTargetTexture = 0;
	m_renderTargetView = 0;
	m_shaderResourceView = 0;
}

bool TextureHandler::Initialize(ID3D11Device* pd3dDevice, int textureWidth, int textureHeight)
{
	D3D11_TEXTURE2D_DESC textureDesc;
	HRESULT hr;
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetDesc;
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceDesc;

	ZeroMemory(&textureDesc, sizeof(textureDesc));
	textureDesc.Width = textureWidth;
	textureDesc.Height = textureHeight;
	textureDesc.ArraySize = 1;
	textureDesc.MipLevels = 1;
	textureDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	V_RETURN(pd3dDevice->CreateTexture2D(&textureDesc, NULL, &m_renderTargetTexture));
	renderTargetDesc.Format = textureDesc.Format;
	renderTargetDesc.ViewDimension = D3D11_RTV_DIMENSION::D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetDesc.Texture2D.MipSlice = 0;

	V_RETURN(pd3dDevice->CreateRenderTargetView(m_renderTargetTexture, &renderTargetDesc, &m_renderTargetView));

	shaderResourceDesc.Format = renderTargetDesc.Format;
	shaderResourceDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceDesc.Texture2D.MipLevels = 1;

	V_RETURN(pd3dDevice->CreateShaderResourceView(m_renderTargetTexture, &shaderResourceDesc, &m_shaderResourceView));

	return true;
	
}

void TextureHandler::Shutdown()
{
	if (m_shaderResourceView)
	{
		m_shaderResourceView->Release();
		m_shaderResourceView = 0;
	}
	if (m_renderTargetView)
	{
		m_renderTargetView->Release();
		m_renderTargetView = 0;
	}
	if (m_renderTargetTexture)
	{
		m_renderTargetTexture->Release();
		m_renderTargetTexture = 0;
	}
}

ID3D11ShaderResourceView* TextureHandler::GetShaderResourceView()
{
	return m_shaderResourceView;
}

void TextureHandler::SetRenderTarget(ID3D11DeviceContext* deviceContext, ID3D11DepthStencilView* DSV)
{
	deviceContext->OMSetRenderTargets(1, &m_renderTargetView, DSV);
	return;
}

void TextureHandler::ClearTarget(ID3D11DeviceContext* deviceContext, ID3D11DepthStencilView* DSV, D3DXVECTOR4 Color)
{
	deviceContext->ClearRenderTargetView(m_renderTargetView, Color);

	deviceContext->ClearDepthStencilView(DSV, D3D11_CLEAR_FLAG::D3D11_CLEAR_DEPTH, 1.0f, 0);

	return;
}