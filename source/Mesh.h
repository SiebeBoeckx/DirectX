#pragma once

#include "DataTypes.h"
#include "Effect.h"
#include "Camera.h"

namespace dae
{
	class Mesh
	{
	public:
		Mesh(ID3D11Device* pDevice, std::vector<dae::Vertex_PosCol> vertices, std::vector<uint32_t> indices);
		virtual ~Mesh();

		void Render(ID3D11DeviceContext* pDeviceContext, const Camera& camera);

	private:
		ID3D11Device* m_pDevice{};
		std::vector<dae::Vertex_PosCol> m_Vertices;
		std::vector<uint32_t> m_Indices;
		int m_NumIndices{};

		Effect* m_pEffect{};
		ID3DX11EffectTechnique* m_pTechnique{};

		ID3D11Buffer* m_pVertexBuffer{};

		ID3D11InputLayout* m_pInputLayout{};
		ID3D11Buffer* m_pIndexBuffer{};
	};
}

