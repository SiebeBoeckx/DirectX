#include "pch.h"
#include "Effect.h"

namespace dae
{

	Effect_PosCol::Effect_PosCol(ID3D11Device* pDevice, const std::wstring& assetFile)
		:m_pDevice{ pDevice }
		, m_assetFile{ assetFile }
	{
		m_pEffect = LoadEffect(pDevice, assetFile);
		m_pTechnique = m_pEffect->GetTechniqueByName("DefaultTechnique");
		if (!m_pTechnique->IsValid())
		{
			std::wcout << L"Technique not valid\n";
		}

		m_pMatWorldViewProjVariable = m_pEffect->GetVariableByName("gWorldViewProj")->AsMatrix();
		if (!m_pMatWorldViewProjVariable->IsValid())
		{
			std::wcout << L"m_pMatWorldViewProjVariable not valid\n";
		}
	}

	Effect_PosCol::~Effect_PosCol()
	{
		m_pTechnique->Release();

		m_pEffect->Release();
	}

	ID3DX11Effect* Effect_PosCol::LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile)
	{
		HRESULT result;
		ID3D10Blob* pErrorBlob{ nullptr };
		ID3DX11Effect* pEffect;

		DWORD shaderFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
		shaderFlags |= D3DCOMPILE_DEBUG;
		shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

		result = D3DX11CompileEffectFromFile(assetFile.c_str(),
			nullptr,
			nullptr,
			shaderFlags,
			0,
			pDevice,
			&pEffect,
			&pErrorBlob);

		if (FAILED(result))
		{
			if (pErrorBlob != nullptr)
			{
				const char* pErrors = static_cast<char*>(pErrorBlob->GetBufferPointer());

				std::wstringstream ss;
				for (unsigned int i{ 0 }; i < pErrorBlob->GetBufferSize(); ++i)
				{
					ss << pErrors[i];
				}

				OutputDebugStringW(ss.str().c_str());
				pErrorBlob->Release();
				pErrorBlob = nullptr;

				std::wcout << ss.str() << '\n';
			}
			else
			{
				std::wstringstream ss;
				ss << "EffectLoader: Failed to CreateEffectFromFile!\nPath: " << assetFile << '\n';
				return nullptr;
			}
		}

		return pEffect;
	}

	//==============================================================================
	//Base
	//==============================================================================

	Effect_PosTex::Effect_PosTex(ID3D11Device* pDevice, const std::wstring& assetFile)
		:m_pDevice{ pDevice }
		, m_assetFile{ assetFile }
	{
	}

	Effect_PosTex::~Effect_PosTex()
	{
		m_pSamplerState->Release();
		m_pAnisotropicSampler->Release();
		m_pLinearSampler->Release();
		m_pPointSampler->Release();

		m_pMatInvViewVariable->Release();
		m_pMatWorldViewProjVariable->Release();
		m_pMatWorldVariable->Release();

		m_pTechnique->Release();

		m_pEffect->Release();
	}

	void Effect_PosTex::Initialize()
	{
		m_pEffect = LoadEffect(m_pDevice, m_assetFile);

		m_pTechnique = m_pEffect->GetTechniqueByIndex(0);
		if (!m_pTechnique->IsValid())
		{
			std::wcout << L"Technique not valid\n";
		}

		m_pMatWorldVariable = m_pEffect->GetVariableByName("gWorldMatrix")->AsMatrix();
		if (!m_pMatWorldVariable->IsValid())
		{
			std::wcout << L"m_pMatWorldVariable not valid\n";
		}

		m_pMatWorldViewProjVariable = m_pEffect->GetVariableByName("gWorldViewProj")->AsMatrix();
		if (!m_pMatWorldViewProjVariable->IsValid())
		{
			std::wcout << L"m_pMatWorldViewProjVariable not valid\n";
		}

		m_pMatInvViewVariable = m_pEffect->GetVariableByName("gInvViewMatrix")->AsMatrix();
		if (!m_pMatInvViewVariable->IsValid())
		{
			std::wcout << L"m_pMatInvViewVariable not valid\n";
		}

		m_pSamplerState = m_pEffect->GetVariableByName("gSampler")->AsSampler();
		if (!m_pSamplerState->IsValid())
		{
			std::wcout << L"m_pSamplerState not valid\n";
		}
		SetTextures();

		//Create states  ==========================
		D3D11_SAMPLER_DESC samplerDesc{};
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

		samplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		samplerDesc.MaxAnisotropy = 1;

		HRESULT hr = m_pDevice->CreateSamplerState(&samplerDesc, &m_pPointSampler);

		samplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
		hr = m_pDevice->CreateSamplerState(&samplerDesc, &m_pLinearSampler);

		samplerDesc.Filter = D3D11_FILTER_COMPARISON_ANISOTROPIC;
		hr = m_pDevice->CreateSamplerState(&samplerDesc, &m_pAnisotropicSampler);
	}

	ID3DX11Effect* Effect_PosTex::LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile)
	{
		HRESULT result;
		ID3D10Blob* pErrorBlob{ nullptr };
		ID3DX11Effect* pEffect;

		DWORD shaderFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
		shaderFlags |= D3DCOMPILE_DEBUG;
		shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

		result = D3DX11CompileEffectFromFile(assetFile.c_str(),
			nullptr,
			nullptr,
			shaderFlags,
			0,
			pDevice,
			&pEffect,
			&pErrorBlob);

		if (FAILED(result))
		{
			if (pErrorBlob != nullptr)
			{
				const char* pErrors = static_cast<char*>(pErrorBlob->GetBufferPointer());

				std::wstringstream ss;
				for (unsigned int i{ 0 }; i < pErrorBlob->GetBufferSize(); ++i)
				{
					ss << pErrors[i];
				}

				OutputDebugStringW(ss.str().c_str());
				pErrorBlob->Release();
				pErrorBlob = nullptr;

				std::wcout << ss.str() << '\n';
			}
			else
			{
				std::wstringstream ss;
				ss << "EffectLoader: Failed to CreateEffectFromFile!\nPath: " << assetFile << '\n';
				return nullptr;
			}
		}

		return pEffect;
	}

	void Effect_PosTex::CycleSampleState()
	{
		switch (m_currentSamplerState)
		{
		case dae::Effect_PosTex::SamplerState::POINT:
			m_currentSamplerState = SamplerState::LINEAR;
			SetSamplerState(SamplerState::LINEAR);
			break;
		case dae::Effect_PosTex::SamplerState::LINEAR:
			m_currentSamplerState = SamplerState::ANISOTROPIC;
			SetSamplerState(SamplerState::ANISOTROPIC);
			break;
		case dae::Effect_PosTex::SamplerState::ANISOTROPIC:
			m_currentSamplerState = SamplerState::POINT;
			SetSamplerState(SamplerState::POINT);
			break;
		}
	}

	void Effect_PosTex::SetSamplerState(SamplerState state)
	{
		switch (state)
		{
		case SamplerState::POINT:
			std::cout << "Point sampler set\n";
			m_pSamplerState->SetSampler(0, m_pPointSampler);
			break;
		case SamplerState::LINEAR:
			std::cout << "Linear sampler set\n";
			m_pSamplerState->SetSampler(0, m_pLinearSampler);
			break;
		case SamplerState::ANISOTROPIC:
			std::cout << "Anisotropic sampler set\n";
			m_pSamplerState->SetSampler(0, m_pAnisotropicSampler);
			break;
		}
	}

	//==============================================================================
	//Vehicle
	//==============================================================================

	void Effect_PosTexVehicle::SetTextures()
	{
		m_pDiffuseMapVariable = m_pEffect->GetVariableByName("gDiffuseMap")->AsShaderResource();
		if (!m_pDiffuseMapVariable->IsValid())
		{
			std::wcout << L"m_pDiffuseMapVariable not valid\n";
		}

		m_pNormalMapVariable = m_pEffect->GetVariableByName("gNormalMap")->AsShaderResource();
		if (!m_pNormalMapVariable->IsValid())
		{
			std::wcout << L"m_pNormalMapVariable not valid\n";
		}

		m_pSpecularMapVariable = m_pEffect->GetVariableByName("gSpecularMap")->AsShaderResource();
		if (!m_pSpecularMapVariable->IsValid())
		{
			std::wcout << L"m_pSpecularMapVariable not valid\n";
		}

		m_pGlossMapVariable = m_pEffect->GetVariableByName("gGlossMap")->AsShaderResource();
		if (!m_pGlossMapVariable->IsValid())
		{
			std::wcout << L"m_pGlossMapVariable not valid\n";
		}
	}

	void Effect_PosTexVehicle::SetDiffuseMap(Texture* pDiffuseMap)
	{
		if (m_pDiffuseMapVariable)
		{
			m_pDiffuseMapVariable->SetResource(pDiffuseMap->GetSRV());
		}
	}

	void Effect_PosTexVehicle::SetNormalMap(Texture* pNormalMap)
	{
		if (m_pNormalMapVariable)
		{
			m_pNormalMapVariable->SetResource(pNormalMap->GetSRV());
		}
	}

	void Effect_PosTexVehicle::SetSpecularMap(Texture* pSpecularMap)
	{
		if (m_pSpecularMapVariable)
		{
			m_pSpecularMapVariable->SetResource(pSpecularMap->GetSRV());
		}
	}

	void Effect_PosTexVehicle::SetGlossMap(Texture* pGlossMap)
	{
		if (m_pGlossMapVariable)
		{
			m_pGlossMapVariable->SetResource(pGlossMap->GetSRV());
		}
	}

	ID3DX11Effect* Effect_PosTexVehicle::LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile)
	{
		HRESULT result;
		ID3D10Blob* pErrorBlob{ nullptr };
		ID3DX11Effect* pEffect;

		DWORD shaderFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
		shaderFlags |= D3DCOMPILE_DEBUG;
		shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

		result = D3DX11CompileEffectFromFile(assetFile.c_str(),
			nullptr,
			nullptr,
			shaderFlags,
			0,
			pDevice,
			&pEffect,
			&pErrorBlob);

		if (FAILED(result))
		{
			if (pErrorBlob != nullptr)
			{
				const char* pErrors = static_cast<char*>(pErrorBlob->GetBufferPointer());

				std::wstringstream ss;
				for (unsigned int i{ 0 }; i < pErrorBlob->GetBufferSize(); ++i)
				{
					ss << pErrors[i];
				}

				OutputDebugStringW(ss.str().c_str());
				pErrorBlob->Release();
				pErrorBlob = nullptr;

				std::wcout << ss.str() << '\n';
			}
			else
			{
				std::wstringstream ss;
				ss << "EffectLoader: Failed to CreateEffectFromFile!\nPath: " << assetFile << '\n';
				return nullptr;
			}
		}

		return pEffect;
	}

	void Effect_PosTexVehicle::SetSamplerState(SamplerState state)
	{
		switch (state)
		{
		case SamplerState::POINT:
			std::cout << "Point sampler set\n";
			m_pSamplerState->SetSampler(0, m_pPointSampler);
			break;
		case SamplerState::LINEAR:
			std::cout << "Linear sampler set\n";
			m_pSamplerState->SetSampler(0, m_pLinearSampler);
			break;
		case SamplerState::ANISOTROPIC:
			std::cout << "Anisotropic sampler set\n";
			m_pSamplerState->SetSampler(0, m_pAnisotropicSampler);
			break;
		}
	}

	//==============================================================================
	//Fire
	//==============================================================================

	void Effect_PosTexFire::SetTextures()
	{
		m_pDiffuseMapVariable = m_pEffect->GetVariableByName("gDiffuseMap")->AsShaderResource();
		if (!m_pDiffuseMapVariable->IsValid())
		{
			std::wcout << L"m_pDiffuseMapVariable not valid\n";
		}
	}

	void Effect_PosTexFire::SetDiffuseMap(Texture* pDiffuseMap)
	{
		if (m_pDiffuseMapVariable)
		{
			m_pDiffuseMapVariable->SetResource(pDiffuseMap->GetSRV());
		}
	}

	ID3DX11Effect* Effect_PosTexFire::LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile)
	{
		HRESULT result;
		ID3D10Blob* pErrorBlob{ nullptr };
		ID3DX11Effect* pEffect;

		DWORD shaderFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
		shaderFlags |= D3DCOMPILE_DEBUG;
		shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

		result = D3DX11CompileEffectFromFile(assetFile.c_str(),
			nullptr,
			nullptr,
			shaderFlags,
			0,
			pDevice,
			&pEffect,
			&pErrorBlob);

		if (FAILED(result))
		{
			if (pErrorBlob != nullptr)
			{
				const char* pErrors = static_cast<char*>(pErrorBlob->GetBufferPointer());

				std::wstringstream ss;
				for (unsigned int i{ 0 }; i < pErrorBlob->GetBufferSize(); ++i)
				{
					ss << pErrors[i];
				}

				OutputDebugStringW(ss.str().c_str());
				pErrorBlob->Release();
				pErrorBlob = nullptr;

				std::wcout << ss.str() << '\n';
			}
			else
			{
				std::wstringstream ss;
				ss << "EffectLoader: Failed to CreateEffectFromFile!\nPath: " << assetFile << '\n';
				return nullptr;
			}
		}

		return pEffect;
	}

	void Effect_PosTexFire::SetSamplerState(SamplerState state)
	{
		switch (state)
		{
		case SamplerState::POINT:
			std::cout << "Point sampler set\n";
			m_pSamplerState->SetSampler(0, m_pPointSampler);
			break;
		case SamplerState::LINEAR:
			std::cout << "Linear sampler set\n";
			m_pSamplerState->SetSampler(0, m_pLinearSampler);
			break;
		case SamplerState::ANISOTROPIC:
			std::cout << "Anisotropic sampler set\n";
			m_pSamplerState->SetSampler(0, m_pAnisotropicSampler);
			break;
		}
	}
}

