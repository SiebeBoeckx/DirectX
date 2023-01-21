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

	class Mesh_PosTexVehicle final
	{
	public:
		Mesh_PosTexVehicle(ID3D11Device* pDevice, std::vector<Vertex_PosTex> vertices, std::vector<uint32_t> indices, const Matrix& worldMatrix, std::vector<Texture*> pTextures);

		virtual ~Mesh_PosTexVehicle();

		void Render(ID3D11DeviceContext* pDeviceContext, const Camera& camera);
		void CycleSamplerState();

		std::vector<Vertex_PosTex> GetVertices() const { return m_VerticesTex; };
		std::vector<uint32_t> GetIndices() const { return m_Indices; };

		Matrix m_WorldMatrix{};

	private:
		ID3D11Device* m_pDevice{};
		std::vector<Vertex_PosTex> m_VerticesTex;
		std::vector<uint32_t> m_Indices;
		int m_NumIndices{};

		Effect_PosTexVehicle* m_pEffect{};
		ID3DX11EffectTechnique* m_pTechnique{};

		ID3D11Buffer* m_pVertexBuffer{};

		ID3D11InputLayout* m_pInputLayout{};
		ID3D11Buffer* m_pIndexBuffer{};
	};  
	//I could have used inheritance again here, but I was running short on time, so sorry
	class Mesh_PosTexSoftwareVehicle final
	{
	public:
		Mesh_PosTexSoftwareVehicle(std::vector<Vertex_PosTex> vertices, std::vector<uint32_t> indices, const Matrix& worldMatrix, PrimitiveTopology topology);

		std::vector<Vertex_PosTex> GetVertices() const { return m_VerticesTex; };
		std::vector<uint32_t> GetIndices() const { return m_Indices; };

		Matrix m_WorldMatrix{};
		std::vector<Vertex_Out> m_Vertices_out{};
		PrimitiveTopology m_topology{ PrimitiveTopology::TriangleList };

	private:
		std::vector<Vertex_PosTex> m_VerticesTex;
		std::vector<uint32_t> m_Indices;
		int m_NumIndices{};
	};
	//I could have used inheritance again here, but I was running short on time, so sorry
	class Mesh_PosTexFire final
	{
	public:
		Mesh_PosTexFire(ID3D11Device* pDevice, std::vector<dae::Vertex_PosTex> vertices, std::vector<uint32_t> indices, const Matrix& worldMatrix, Texture* pTexture);

		virtual ~Mesh_PosTexFire();

		void Render(ID3D11DeviceContext* pDeviceContext, const Camera& camera);
		void CycleSamplerState();

		Matrix m_WorldMatrix{};

	private:
		ID3D11Device* m_pDevice{};
		std::vector<dae::Vertex_PosTex> m_VerticesTex;
		std::vector<uint32_t> m_Indices;
		int m_NumIndices{};

		Effect_PosTexFire* m_pEffect{};
		ID3DX11EffectTechnique* m_pTechnique{};

		ID3D11Buffer* m_pVertexBuffer{};

		ID3D11InputLayout* m_pInputLayout{};
		ID3D11Buffer* m_pIndexBuffer{};
	};
}

