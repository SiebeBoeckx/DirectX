#pragma once
#include "HardwareRenderer.h"
#include "SoftwareRenderer.h"
#include "Texture.h"

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	class RenderManager final
	{
	public:
		RenderManager(SDL_Window* pWindow);
		~RenderManager();

		RenderManager(const RenderManager&) = delete;
		RenderManager(RenderManager&&) noexcept = delete;
		RenderManager& operator=(const RenderManager&) = delete;
		RenderManager& operator=(RenderManager&&) noexcept = delete;

		void Update(const Timer* pTimer);
		void Render() const;

		//Shared
		void ToggleRenderType();
		void CycleRotation();
		void CycleCullMode();
		void ToggleUniformColor();

		//Hardware
		void ToggleFireFx();
		void CycleSamplerState();

		//Software
		void CycleShadingMode();
		void ToggleNormalMap();
		void ToggleDepthBuffer();
		void ToggleBoundingBoxView();


		enum class RenderType {
			Software,
			Hardware
		};

	private:
		SDL_Window* m_pWindow{};
		SoftwareRenderer* m_pRendererSoftware{};
		HardwareRenderer* m_pRendererHardware{};

		BaseRenderer* m_pCurrentRenderer{};
		Camera* m_pCamera{};

		std::vector<Texture*> m_pSoftwareTextures;
		std::vector<Texture*> m_pHardwareTextures;
		Texture* m_pFireTexture;

		Mesh_PosTexSoftwareVehicle* m_pSoftwareMesh{};

		Mesh_PosTexVehicle* m_pHardwareMesh{};
		Mesh_PosTexFire* m_pFire{};

		RenderType m_CurrentRenderType{ RenderType::Hardware };
	};
}