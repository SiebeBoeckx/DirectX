#include "pch.h"
#include "Mesh.h"

namespace dae
{
	Mesh_PosCol::Mesh_PosCol(ID3D11Device* pDevice, std::vector<Vertex_PosCol> vertices, std::vector<uint32_t> indices)
		:m_pDevice{ pDevice }
		, m_VerticesCol{ vertices }
		, m_Indices{ indices }
	{
		m_pEffect = new Effect_PosCol{ m_pDevice, std::wstring{L"Resources/PosCol3D.fx"} };
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

	Mesh_PosCol::~Mesh_PosCol()
	{
		m_pIndexBuffer->Release();

		m_pInputLayout->Release();

		m_pVertexBuffer->Release();

		delete m_pEffect;
		m_pEffect = nullptr;
	}

	void Mesh_PosCol::Render(ID3D11DeviceContext* pDeviceContext, const Camera& camera)
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

	//===================================================================================================================================
	//
	//===================================================================================================================================

	Mesh_PosTexVehicle::Mesh_PosTexVehicle(ID3D11Device* pDevice, std::vector<Vertex_PosTex> vertices, std::vector<uint32_t> indices, const Matrix& worldMatrix, std::vector<Texture*> pTextures)
		:m_pDevice{ pDevice }
		, m_VerticesTex{ vertices }
		, m_Indices{ indices }
		, m_WorldMatrix{ worldMatrix }
	{
		const std::wstring& assetFile{ L"./Resources/PosTex3D.fx" };
		m_pEffect = new Effect_PosTexVehicle{ m_pDevice,  assetFile};
		m_pEffect->Initialize();
		m_pTechnique = m_pEffect->GetTechnique();
		m_pEffect->SetDiffuseMap(pTextures[0]);
		m_pEffect->SetNormalMap(pTextures[1]);
		m_pEffect->SetSpecularMap(pTextures[2]);
		m_pEffect->SetGlossMap(pTextures[3]);

		//Create vertex layout
		static constexpr uint32_t numElements{ 4 };
		D3D11_INPUT_ELEMENT_DESC vertexDesc[numElements]{};

		vertexDesc[0].SemanticName = "POSITION";
		vertexDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		vertexDesc[0].AlignedByteOffset = 0;
		vertexDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

		vertexDesc[1].SemanticName = "TEXTCOORD";
		vertexDesc[1].Format = DXGI_FORMAT_R32G32_FLOAT;
		vertexDesc[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		vertexDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

		vertexDesc[2].SemanticName = "NORMAL";
		vertexDesc[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		vertexDesc[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		vertexDesc[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

		vertexDesc[3].SemanticName = "TANGENT";
		vertexDesc[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		vertexDesc[3].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		vertexDesc[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

		//Create vertex buffer
		D3D11_BUFFER_DESC bd{};
		bd.Usage = D3D11_USAGE_IMMUTABLE;
		bd.ByteWidth = sizeof(Vertex_PosTex) * static_cast<uint32_t>(vertices.size());
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA initData{};
		initData.pSysMem = vertices.data();

		HRESULT result = pDevice->CreateBuffer(&bd, &initData, &m_pVertexBuffer);
		if (FAILED(result))
		{
			std::cout << "failed at 1\n";
			return;
		};


		//Create input layout
		D3DX11_PASS_DESC passDesc{};
		m_pTechnique->GetPassByIndex(0)->GetDesc(&passDesc);

		const HRESULT resultInputLayout = m_pDevice->CreateInputLayout(
			vertexDesc,
			numElements,
			passDesc.pIAInputSignature,
			passDesc.IAInputSignatureSize,
			&m_pInputLayout);
		if (FAILED(resultInputLayout))
		{
			std::wcout << L"Index layout creating failed in mesh" << std::endl;
			return;
		}


		//Create index buffer
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

	Mesh_PosTexVehicle::~Mesh_PosTexVehicle()
	{
		m_pIndexBuffer->Release();

		m_pInputLayout->Release();

		m_pVertexBuffer->Release();

		delete m_pEffect;
		m_pEffect = nullptr;
	}

	void Mesh_PosTexVehicle::Render(ID3D11DeviceContext* pDeviceContext, const Camera& camera)
	{
		//1. Set primitive topology
		pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//2. Set input layout
		pDeviceContext->IASetInputLayout(m_pInputLayout);

		const Matrix viewProjmatrix = camera.GetViewProjMatrix();
		const Matrix invCameraViewMatrix = camera.GetInvViewMatrix();

		const Matrix worldViewProjectionMatrix = m_WorldMatrix * viewProjmatrix;

		DirectX::XMMATRIX worldMatrix
		{
			m_WorldMatrix[0][0],
			m_WorldMatrix[0][1],
			m_WorldMatrix[0][2],
			m_WorldMatrix[0][3],

			m_WorldMatrix[1][0],
			m_WorldMatrix[1][1],
			m_WorldMatrix[1][2],
			m_WorldMatrix[1][3],

			m_WorldMatrix[2][0],
			m_WorldMatrix[2][1],
			m_WorldMatrix[2][2],
			m_WorldMatrix[2][3],

			m_WorldMatrix[3][0],
			m_WorldMatrix[3][1],
			m_WorldMatrix[3][2],
			m_WorldMatrix[3][3]
		};

		DirectX::XMMATRIX worldViewProjMatrix
		{
			worldViewProjectionMatrix[0][0],
			worldViewProjectionMatrix[0][1],
			worldViewProjectionMatrix[0][2],
			worldViewProjectionMatrix[0][3],

			worldViewProjectionMatrix[1][0],
			worldViewProjectionMatrix[1][1],
			worldViewProjectionMatrix[1][2],
			worldViewProjectionMatrix[1][3],

			worldViewProjectionMatrix[2][0],
			worldViewProjectionMatrix[2][1],
			worldViewProjectionMatrix[2][2],
			worldViewProjectionMatrix[2][3],

			worldViewProjectionMatrix[3][0],
			worldViewProjectionMatrix[3][1],
			worldViewProjectionMatrix[3][2],
			worldViewProjectionMatrix[3][3]
		};

		DirectX::XMMATRIX invViewMatrix
		{
			invCameraViewMatrix[0][0],
			invCameraViewMatrix[0][1],
			invCameraViewMatrix[0][2],
			invCameraViewMatrix[0][3],

			invCameraViewMatrix[1][0],
			invCameraViewMatrix[1][1],
			invCameraViewMatrix[1][2],
			invCameraViewMatrix[1][3],

			invCameraViewMatrix[2][0],
			invCameraViewMatrix[2][1],
			invCameraViewMatrix[2][2],
			invCameraViewMatrix[2][3],

			invCameraViewMatrix[3][0],
			invCameraViewMatrix[3][1],
			invCameraViewMatrix[3][2],
			invCameraViewMatrix[3][3]
		};

		const float* worldMatFloatPtr = reinterpret_cast<const float*>(&worldMatrix);
		m_pEffect->GetWorldMatrix()->SetMatrix(worldMatFloatPtr);

		const float* worldViewProjMatFloatPtr = reinterpret_cast<const float*>(&worldViewProjMatrix);
		m_pEffect->GetWorldViewProjMatrix()->SetMatrix(worldViewProjMatFloatPtr);

		const float* invViewMatFloatPtr = reinterpret_cast<const float*>(&invViewMatrix);
		m_pEffect->GetInvViewMatrix()->SetMatrix(invViewMatFloatPtr);

		//3. Set vertex buffer
		constexpr UINT stride = sizeof(Vertex_PosTex);
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

	void Mesh_PosTexVehicle::CycleSamplerState()
	{
		m_pEffect->CycleSampleState();
	}

	void Mesh_PosTexVehicle::CycleCullingMode()
	{
		m_pEffect->CycleCullMode();
	}

	//===================================================================================================================================
	//
	//===================================================================================================================================

	Mesh_PosTexSoftwareVehicle::Mesh_PosTexSoftwareVehicle(std::vector<Vertex_PosTex> vertices, std::vector<uint32_t> indices, const Matrix& worldMatrix, PrimitiveTopology topology)
		: m_VerticesTex{ vertices }
		, m_Indices{ indices }
		, m_WorldMatrix{ worldMatrix }
		, m_topology{topology}
	{
	}

	//===================================================================================================================================
	//
	//===================================================================================================================================

	Mesh_PosTexFire::Mesh_PosTexFire(ID3D11Device* pDevice, std::vector<Vertex_PosTex> vertices, std::vector<uint32_t> indices, const Matrix& worldMatrix, Texture* pTexture)
		:m_pDevice{ pDevice }
		, m_VerticesTex{ vertices }
		, m_Indices{ indices }
		, m_WorldMatrix{ worldMatrix }
	{
		const std::wstring& assetFile{ L"./Resources/Fire.fx" };
		m_pEffect = new Effect_PosTexFire{ m_pDevice, assetFile };
		m_pEffect->Initialize();
		m_pTechnique = m_pEffect->GetTechnique();
		m_pEffect->SetDiffuseMap(pTexture);

		//Create vertex layout
		static constexpr uint32_t numElements{ 4 };
		D3D11_INPUT_ELEMENT_DESC vertexDesc[numElements]{};

		vertexDesc[0].SemanticName = "POSITION";
		vertexDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		vertexDesc[0].AlignedByteOffset = 0;
		vertexDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

		vertexDesc[1].SemanticName = "TEXTCOORD";
		vertexDesc[1].Format = DXGI_FORMAT_R32G32_FLOAT;
		vertexDesc[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		vertexDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

		vertexDesc[2].SemanticName = "NORMAL";
		vertexDesc[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		vertexDesc[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		vertexDesc[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

		vertexDesc[3].SemanticName = "TANGENT";
		vertexDesc[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		vertexDesc[3].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		vertexDesc[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

		//Create vertex buffer
		D3D11_BUFFER_DESC bd{};
		bd.Usage = D3D11_USAGE_IMMUTABLE;
		bd.ByteWidth = sizeof(Vertex_PosTex) * static_cast<uint32_t>(vertices.size());
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA initData{};
		initData.pSysMem = vertices.data();

		HRESULT result = pDevice->CreateBuffer(&bd, &initData, &m_pVertexBuffer);
		if (FAILED(result))
		{
			std::cout << "failed at 1\n";
			return;
		};


		//Create input layout
		D3DX11_PASS_DESC passDesc{};
		m_pTechnique->GetPassByIndex(0)->GetDesc(&passDesc);

		const HRESULT resultInputLayout = m_pDevice->CreateInputLayout(
			vertexDesc,
			numElements,
			passDesc.pIAInputSignature,
			passDesc.IAInputSignatureSize,
			&m_pInputLayout);
		if (FAILED(resultInputLayout))
		{
			std::wcout << L"Index layout creating failed in mesh" << std::endl;
			return;
		}


		//Create index buffer
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

	Mesh_PosTexFire::~Mesh_PosTexFire()
	{
		m_pIndexBuffer->Release();

		m_pInputLayout->Release();

		m_pVertexBuffer->Release();

		delete m_pEffect;
		m_pEffect = nullptr;
	}

	void Mesh_PosTexFire::Render(ID3D11DeviceContext* pDeviceContext, const Camera& camera)
	{
		//1. Set primitive topology
		pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//2. Set input layout
		pDeviceContext->IASetInputLayout(m_pInputLayout);

		const Matrix viewProjmatrix = camera.GetViewProjMatrix();
		const Matrix invCameraViewMatrix = camera.GetInvViewMatrix();

		const Matrix worldViewProjectionMatrix = m_WorldMatrix * viewProjmatrix;

		DirectX::XMMATRIX worldMatrix
		{
			m_WorldMatrix[0][0],
			m_WorldMatrix[0][1],
			m_WorldMatrix[0][2],
			m_WorldMatrix[0][3],

			m_WorldMatrix[1][0],
			m_WorldMatrix[1][1],
			m_WorldMatrix[1][2],
			m_WorldMatrix[1][3],

			m_WorldMatrix[2][0],
			m_WorldMatrix[2][1],
			m_WorldMatrix[2][2],
			m_WorldMatrix[2][3],

			m_WorldMatrix[3][0],
			m_WorldMatrix[3][1],
			m_WorldMatrix[3][2],
			m_WorldMatrix[3][3]
		};

		DirectX::XMMATRIX worldViewProjMatrix
		{
			worldViewProjectionMatrix[0][0],
			worldViewProjectionMatrix[0][1],
			worldViewProjectionMatrix[0][2],
			worldViewProjectionMatrix[0][3],

			worldViewProjectionMatrix[1][0],
			worldViewProjectionMatrix[1][1],
			worldViewProjectionMatrix[1][2],
			worldViewProjectionMatrix[1][3],

			worldViewProjectionMatrix[2][0],
			worldViewProjectionMatrix[2][1],
			worldViewProjectionMatrix[2][2],
			worldViewProjectionMatrix[2][3],

			worldViewProjectionMatrix[3][0],
			worldViewProjectionMatrix[3][1],
			worldViewProjectionMatrix[3][2],
			worldViewProjectionMatrix[3][3]
		};

		DirectX::XMMATRIX invViewMatrix
		{
			invCameraViewMatrix[0][0],
			invCameraViewMatrix[0][1],
			invCameraViewMatrix[0][2],
			invCameraViewMatrix[0][3],

			invCameraViewMatrix[1][0],
			invCameraViewMatrix[1][1],
			invCameraViewMatrix[1][2],
			invCameraViewMatrix[1][3],

			invCameraViewMatrix[2][0],
			invCameraViewMatrix[2][1],
			invCameraViewMatrix[2][2],
			invCameraViewMatrix[2][3],

			invCameraViewMatrix[3][0],
			invCameraViewMatrix[3][1],
			invCameraViewMatrix[3][2],
			invCameraViewMatrix[3][3]
		};

		const float* worldMatFloatPtr = reinterpret_cast<const float*>(&worldMatrix);
		m_pEffect->GetWorldMatrix()->SetMatrix(worldMatFloatPtr);

		const float* worldViewProjMatFloatPtr = reinterpret_cast<const float*>(&worldViewProjMatrix);
		m_pEffect->GetWorldViewProjMatrix()->SetMatrix(worldViewProjMatFloatPtr);

		const float* invViewMatFloatPtr = reinterpret_cast<const float*>(&invViewMatrix);
		m_pEffect->GetInvViewMatrix()->SetMatrix(invViewMatFloatPtr);

		//3. Set vertex buffer
		constexpr UINT stride = sizeof(Vertex_PosTex);
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

	void Mesh_PosTexFire::CycleSamplerState()
	{
		m_pEffect->CycleSampleState();
	}
}