#pragma once

#include <vector>
#include "Timer.h"
#include "Camera.h"
#include "DataTypes.h"

struct SDL_Window;

class Timer;
class Camera;

namespace dae
{
	class BaseRenderer
	{
	public:
		BaseRenderer(SDL_Window* pWindow, Camera* pCamera);
		virtual ~BaseRenderer() = default;

		//Rule of 5
		BaseRenderer(const BaseRenderer&) = delete;
		BaseRenderer(BaseRenderer&&) noexcept = delete;
		BaseRenderer& operator=(const BaseRenderer&) = delete;
		BaseRenderer& operator=(BaseRenderer&&) noexcept = delete;

		void CycleRotation();

		//Pure virtual functions
		virtual void Update(const Timer* pTimer) = 0;
		virtual void Render() = 0;

	protected:
		SDL_Window* m_pWindow{};

		int m_Width{};
		int m_Height{};

		float m_AspectRatio{};

		Camera* m_pCamera{};

		ColorRGB m_RendererColor{};
		ColorRGB m_UniformColor{ 0.1f, 0.1f, 0.1f };

		bool m_ShouldUseUniformColor{ false };
		bool m_IsRotating{ true };
	};
}