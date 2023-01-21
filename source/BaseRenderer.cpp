#include "pch.h"
#include "BaseRenderer.h"
#include "DataTypes.h"

namespace dae
{
	BaseRenderer::BaseRenderer(SDL_Window* pWindow, Camera* pCamera)
		:m_pWindow{ pWindow }
		,m_pCamera{ pCamera }
	{
		//Initialize
		SDL_GetWindowSize(pWindow, &m_Width, &m_Height);
		m_AspectRatio = static_cast<float>(m_Width) / static_cast<float>(m_Height);	
	}

	void BaseRenderer::CycleRotation()
	{
		m_IsRotating = !m_IsRotating;
	}
}