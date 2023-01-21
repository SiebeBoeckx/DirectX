#pragma once
#include "Camera.h"
#include "Mesh.h"


struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Update(const Timer* pTimer);
		void Render() const;
		void CycleSamplerState();
		void CycleRotation();

	private:
		SDL_Window* m_pWindow{};

		int m_Width{};
		int m_Height{};

		bool m_IsInitialized{ false };

		//DIRECTX
		HRESULT InitializeDirectX();

		ID3D11Device* m_pDevice{};
		ID3D11DeviceContext* m_pDeviceContext{};
		IDXGISwapChain* m_pSwapChain{};
		ID3D11Texture2D* m_pDepthStencilBuffer{};
		ID3D11DepthStencilView* m_pDepthStencilView{};
		ID3D11Resource* m_pRenderTargetBuffer{};
		ID3D11RenderTargetView* m_pRenderTargetView{};
		//...

		Mesh_PosTexVehicle* InitializeMesh();
		Mesh_PosTexVehicle* m_pMesh{};

		Mesh_PosTexFire* InitializeFire();
		Mesh_PosTexFire* m_pFire{};

		bool m_IsRotating{ true };

		Camera m_Camera{};

		Texture* m_pTexture{};
		Texture* m_pNormalMap{};
		Texture* m_pSpecularMap{};
		Texture* m_pGlossMap{};

		Texture* m_pFireTexture{};
	};
}
