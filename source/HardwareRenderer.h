#pragma once
#include "BaseRenderer.h"
#include "Camera.h"
#include "Mesh.h"


struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	class HardwareRenderer final : public BaseRenderer
	{
	public:
		HardwareRenderer(SDL_Window* pWindow, Camera* pCamera);
		~HardwareRenderer();

		HardwareRenderer(const HardwareRenderer&) = delete;
		HardwareRenderer(HardwareRenderer&&) noexcept = delete;
		HardwareRenderer& operator=(const HardwareRenderer&) = delete;
		HardwareRenderer& operator=(HardwareRenderer&&) noexcept = delete;

		void Update(const Timer* pTimer) override;
		void Render() override;
		void CycleSamplerState();

		void SetTextures(std::vector<Texture*> pTextures, Texture* pFireTexture) {
			m_pTexture = pTextures[0];
			m_pNormalMap = pTextures[1];
			m_pSpecularMap = pTextures[2];
			m_pGlossMap = pTextures[3];

			m_pFireTexture = pFireTexture;
		};

		void SetMesh(Mesh_PosTexVehicle* pMesh) { m_pMesh = pMesh; };
		void SetFire(Mesh_PosTexFire* pFire) { m_pFire = pFire; };

		ID3D11Device* GetDevice() const { return m_pDevice; };

	private:
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

		Mesh_PosTexVehicle* m_pMesh{};
		Mesh_PosTexFire* m_pFire{};

		Texture* m_pTexture{};
		Texture* m_pNormalMap{};
		Texture* m_pSpecularMap{};
		Texture* m_pGlossMap{};

		Texture* m_pFireTexture{};
	};
}
