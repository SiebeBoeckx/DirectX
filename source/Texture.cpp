#include "pch.h"
#include "Texture.h"
#include "Vector2.h"
#include <SDL_image.h>

namespace dae
{
	Texture::Texture(SDL_Surface* pSurface, ID3D11Device* pDevice)
		:m_pSurface{ pSurface },
		m_pSurfacePixels{ (uint32_t*)pSurface->pixels }
	{
		DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
		D3D11_TEXTURE2D_DESC desc{};
		desc.Width = pSurface->w;
		desc.Height = pSurface->h;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = format;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA initData{};
		initData.pSysMem = pSurface->pixels;
		initData.SysMemPitch = static_cast<UINT>(pSurface->pitch);
		initData.SysMemSlicePitch = static_cast<UINT>(pSurface->pitch * pSurface->h);

		HRESULT hr = pDevice->CreateTexture2D(&desc, &initData, &m_pTexture);

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.Format = format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;

		hr = pDevice->CreateShaderResourceView(m_pTexture, &srvDesc, &m_pShaderResourceView);

		if (m_pSurface)
		{
			SDL_FreeSurface(m_pSurface);
			m_pSurface = nullptr;
		}
	}

	Texture::~Texture()
	{

		m_pTexture->Release();
		m_pShaderResourceView->Release();
	}

	Texture* Texture::LoadFromFile(const std::string& path, ID3D11Device* pDevice)
	{
		return new Texture(IMG_Load(path.c_str()), pDevice);
	}

	ColorRGB Texture::Sample(const Vector2& uv) const
	{
		Uint32 pixelX = Uint32(m_pSurface->w * uv.x);
		Uint32 pixelY = Uint32(m_pSurface->h * uv.y);

		Uint32 pixelIndx = pixelX + pixelY * m_pSurface->w;
		Uint8 r{}, g{}, b{};
		SDL_GetRGB(m_pSurfacePixels[pixelIndx], m_pSurface->format, &r, &g, &b);
		ColorRGB texelColor{ float(r), float(g), float(b) };
		texelColor /= 255.0f;

		//Sample the correct texel for the given uv
		return texelColor;
	}
}