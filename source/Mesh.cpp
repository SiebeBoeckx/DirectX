#include "pch.h"
#include "Mesh.h"


namespace dae
{
	Mesh::Mesh(ID3D11Device* pDevice, std::vector<Vertex_PosCol> vertices, std::vector<uint32_t> indices)
		:m_pDevice{ pDevice }
		, m_Vertices{ vertices }
		, m_Indices{ indices }
	{
		m_pEffect = new Effect{ m_pDevice, std::wstring{L"Resources/PosCol3D.fx"} };
		m_pTechnique = m_pEffect->GetTechnique();

		//Create vertex layout
		static constexpr uint32_t numElements{ 2 };
		D3D11_INPUT_ELEMENT_DESC vertexDesc[numElements]{};

		vertexDesc[0].SemanticName = "POSITION";
		vertexDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		vertexDesc[0].AlignedByteOffset = 0;
		vertexDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

		vertexDesc[1].SemanticName = "COLOR";
		vertexDesc[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		vertexDesc[1].AlignedByteOffset = 12;
		vertexDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;


		//Create vertex buffer
		D3D11_BUFFER_DESC bd{};
		bd.Usage = D3D11_USAGE_IMMUTABLE;
		bd.ByteWidth = sizeof(Vertex_PosCol) * static_cast<uint32_t>(vertices.size());
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA initData{};
		initData.pSysMem = vertices.data();

		HRESULT result = pDevice->CreateBuffer(&bd, &initData, &m_pVertexBuffer);
		if (FAILED(result)) return;


		//Create input layout
		D3DX11_PASS_DESC passDesc{};
		m_pTechnique->GetPassByIndex(0)->GetDesc(&passDesc);

		result = pDevice->CreateInputLayout(
			vertexDesc,
			numElements,
			passDesc.pIAInputSignature,
			passDesc.IAInputSignatureSize,
			&m_pInputLayout);

		if (FAILED(result)) return;


		//Create input buffer
		m_NumIndices = static_cast<uint32_t>(indices.size());
		bd.Usage = D3D11_USAGE_IMMUTABLE;
		bd.ByteWidth = sizeof(uint32_t) * m_NumIndices;
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;
		initData.pSysMem = indices.data();
		result = pDevice->CreateBuffer(&bd, &initData, &m_pIndexBuffer);
		if (FAILED(result)) return;
	}

	Mesh::~Mesh()
	{
		m_pIndexBuffer->Release();

		m_pInputLayout->Release();

		m_pVertexBuffer->Release();

		delete m_pEffect;
		m_pEffect = nullptr;
	}

	void Mesh::Render(ID3D11DeviceContext* pDeviceContext, const Camera& camera)
	{
		//1. Set primitive topology
		pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//2. Set input layout
		pDeviceContext->IASetInputLayout(m_pInputLayout);

		Matrix viewProjmatrix = camera.GetViewProjMatrix();
		DirectX::XMMATRIX worldViewProjMatrix
		{
			viewProjmatrix[0][0],
			viewProjmatrix[0][1],
			viewProjmatrix[0][2],
			viewProjmatrix[0][3],

			viewProjmatrix[1][0],
			viewProjmatrix[1][1],
			viewProjmatrix[1][2],
			viewProjmatrix[1][3],

			viewProjmatrix[2][0],
			viewProjmatrix[2][1],
			viewProjmatrix[2][2],
			viewProjmatrix[2][3],

			viewProjmatrix[3][0],
			viewProjmatrix[3][1],
			viewProjmatrix[3][2],
			viewProjmatrix[3][3]
		};

		const float* matFloatPtr = reinterpret_cast<const float*>(&worldViewProjMatrix);
		m_pEffect->GetMatrixVariable()->SetMatrix(matFloatPtr);

		//3. Set vertex buffer
		constexpr UINT stride = sizeof(Vertex_PosCol);
		constexpr UINT offset = 0;
		pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

		//4. Set index buffer
		pDeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		//5. Draw
		D3DX11_TECHNIQUE_DESC techDesc{};
		m_pEffect->GetTechnique()->GetDesc(&techDesc);

		for (UINT p{}; p < techDesc.Passes; ++p)
		{
			m_pEffect->GetTechnique()->GetPassByIndex(p)->Apply(0, pDeviceContext);
			pDeviceContext->DrawIndexed(m_NumIndices, 0, 0);
		}
	}
}