#pragma once
#include <SDL_surface.h>
#include <string>
#include "ColorRGB.h"

namespace dae
{
	struct Vector2;

	class Texture
	{
	public:
		~Texture();
		static 	Texture* LoadFromFile(const std::string& path, ID3D11Device* pDevice);
		ColorRGB Sample(const Vector2& uv) const;
		ID3D11ShaderResourceView* GetSRV() const { return m_pShaderResourceView; }

	private:
		Texture(SDL_Surface* pSurface, ID3D11Device* pDevice);

		SDL_Surface* m_pSurface{ nullptr };
		uint32_t* m_pSurfacePixels{ nullptr };
		ID3D11ShaderResourceView* m_pShaderResourceView{ nullptr };
		ID3D11Texture2D* m_pTexture{ nullptr };
	};
}