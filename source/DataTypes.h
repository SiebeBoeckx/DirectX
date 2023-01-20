#pragma once
#include "pch.h"

namespace dae
{
	struct Vertex_PosCol
	{
		Vector3 position{};
		ColorRGB color{ colors::White };
	};

	struct Vertex_PosTex
	{
		Vector3 position{};
		Vector2 TexCoord{};
		Vector3 normal{};
		Vector3 tangent{};
	};
}