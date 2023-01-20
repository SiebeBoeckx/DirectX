#pragma once

#include "DataTypes.h"
#include "Effect.h"
#include "Camera.h"

namespace dae
{
	class Mesh_PosCol final
	{
	public:
		Mesh_PosCol(ID3D11Device* pDevice, std::vector<dae::Vertex_PosCol> vertices, std::vector<uint32_t> indices);
		
		virtual ~Mesh_PosCol();

		void Render(ID3D11DeviceContext* pDeviceContext, const Camera& camera);

	private:
		ID3D11Device* m_pDevice{};
		std::vector<dae::Vertex_PosCol> m_VerticesCol;
		std::vector<uint32_t> m_Indices;
		int m_NumIndices{};

		Effect_PosCol* m_pEffect{};
		ID3DX11EffectTechnique* m_pTechnique{};

		ID3D11Buffer* m_pVertexBuffer{};

		ID3D11InputLayout* m_pInputLayout{};
		ID3D11Buffer* m_pIndexBuffer{};
	};

	class Mesh_PosTex final
	{
	public:
		Mesh_PosTex(ID3D11Device* pDevice, std::vector<dae::Vertex_PosTex> vertices, std::vector<uint32_t> indices, const Matrix& worldMatrix, Texture* pTexture, Texture* pNormal, Texture* pSpecular, Texture* pGloss);

		virtual ~Mesh_PosTex();

		void Render(ID3D11DeviceContext* pDeviceContext, const Camera& camera);
		void CycleSamplerState();

		Matrix m_WorldMatrix{};

	private:
		ID3D11Device* m_pDevice{};
		std::vector<dae::Vertex_PosTex> m_VerticesTex;
		std::vector<uint32_t> m_Indices;
		int m_NumIndices{};

		Effect_PosTex* m_pEffect{};
		ID3DX11EffectTechnique* m_pTechnique{};

		ID3D11Buffer* m_pVertexBuffer{};

		ID3D11InputLayout* m_pInputLayout{};
		ID3D11Buffer* m_pIndexBuffer{};
	};
}

