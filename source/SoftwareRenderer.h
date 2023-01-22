#pragma once

#include <cstdint>
#include <vector>

#include "BaseRenderer.h"
#include "Camera.h"
#include "DataTypes.h"
#include "Mesh.h"

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	class SoftwareRenderer final : public BaseRenderer
	{
	public:
		SoftwareRenderer(SDL_Window* pWindow, Camera* pCamera, std::vector<Texture*> pTextures);
		~SoftwareRenderer();

		SoftwareRenderer(const SoftwareRenderer&) = delete;
		SoftwareRenderer(SoftwareRenderer&&) noexcept = delete;
		SoftwareRenderer& operator=(const SoftwareRenderer&) = delete;
		SoftwareRenderer& operator=(SoftwareRenderer&&) noexcept = delete;

		void Update(const Timer* pTimer) override;
		void Render() override;

		bool SaveBufferToImage() const;

		//==== Extra

		void CycleRenderState();

		void ToggleNormalMap();

		void SetMesh(Mesh_PosTexSoftwareVehicle* pMesh) { m_pMesh = pMesh; };

	private:
		SDL_Surface* m_pFrontBuffer{ nullptr };
		SDL_Surface* m_pBackBuffer{ nullptr };
		uint32_t* m_pBackBufferPixels{};

		float* m_pDepthBufferPixels{};

		Texture* m_pTexture{};
		Texture* m_pNormals{};
		Texture* m_pSpecular{};
		Texture* m_pPhongExponent{};

		Mesh_PosTexSoftwareVehicle* m_pMesh{};

		ColorRGB m_ClearColor{ 99.f, 99.f, 99.f }; //99 / 255 = 0.36

		enum class RenderState
		{
			texture = 0,
			depth = 1,
			observedArea = 2,
			phong = 3
		};
		RenderState m_State{ RenderState::texture };

		bool m_UsingNormalMap{ true };

		struct Light
		{
			Vector3 direction;
			float intensity;
			ColorRGB ambientColor;
		};
		const Light m_Light{ Vector3{.577f, -.577f, .577f}.Normalized(), 7.f, ColorRGB{.025f, .025f, .025f}};

		//Function that transforms the vertices from the mesh from World space to Screen space
		void VertexTransformationFunction(const std::vector<Vertex_PosTex>& vertices_in, std::vector<Vertex_Out>& vertices_out, const Matrix& meshWorldMatrix); //W3 Version

		void RenderTriangle(const Vertex_Out& v0, const Vertex_Out& v1, const Vertex_Out& v2) const; //W4
		//=========

		void RenderMesh(); //Vehicle
		ColorRGB PixelShading(const Vertex_Out& vertex) const;
		ColorRGB Phong(float specular, float exp, const Vector3& l, const Vector3& v, const Vector3& n) const;

		Mesh_PosTexSoftwareVehicle* InitializeMesh();
	};
}
