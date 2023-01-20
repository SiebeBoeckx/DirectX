#pragma once
#include "Texture.h"

namespace dae
{

	class Effect_PosCol final
	{
	public:
		Effect_PosCol(ID3D11Device* pDevice, const std::wstring& assetFile);
		virtual ~Effect_PosCol();


		ID3DX11Effect* GetEffect() const { return m_pEffect; };
		ID3DX11EffectTechnique* GetTechnique() const { return m_pTechnique; };
		ID3DX11EffectMatrixVariable* GetMatrixVariable() { return m_pMatWorldViewProjVariable; };

	private:
		static ID3DX11Effect* LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile);

		ID3D11Device* m_pDevice;
		const std::wstring& m_assetFile;
		ID3DX11Effect* m_pEffect;
		ID3DX11EffectTechnique* m_pTechnique;

		//Camera
		ID3DX11EffectMatrixVariable* m_pMatWorldViewProjVariable{};
	};

	class Effect_PosTex final
	{
	public:
		Effect_PosTex(ID3D11Device* pDevice, const std::wstring& assetFile);
		virtual ~Effect_PosTex();


		ID3DX11Effect* GetEffect() const { return m_pEffect; };
		ID3DX11EffectTechnique* GetTechnique() const { return m_pTechnique; };

		ID3DX11EffectMatrixVariable* GetWorldMatrix() { return m_pMatWorldVariable; };
		ID3DX11EffectMatrixVariable* GetWorldViewProjMatrix() { return m_pMatWorldViewProjVariable; };
		ID3DX11EffectMatrixVariable* GetInvViewMatrix() { return m_pMatInvViewVariable; };

		void SetDiffuseMap(Texture* pDiffuseMap);
		void SetNormalMap(Texture* pNormalMap);
		void SetSpecularMap(Texture* pSpecularMap);
		void SetGlossMap(Texture* pGlossMap);
		void CycleSampleState();

		enum class SamplerState
		{
			POINT,
			LINEAR,
			ANISOTROPIC
		};

	private:
		static ID3DX11Effect* LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile);
		void SetSamplerState(SamplerState state);

		ID3D11Device* m_pDevice;
		const std::wstring& m_assetFile;
		ID3DX11Effect* m_pEffect;
		ID3DX11EffectTechnique* m_pTechnique;

		ID3DX11EffectMatrixVariable* m_pMatWorldVariable{};
		//Camera
		ID3DX11EffectMatrixVariable* m_pMatWorldViewProjVariable{};
		ID3DX11EffectMatrixVariable* m_pMatInvViewVariable{};

		//Texture
		ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable;
		ID3D11SamplerState* m_pPointSampler;
		ID3D11SamplerState* m_pLinearSampler;
		ID3D11SamplerState* m_pAnisotropicSampler;

		//Normal
		ID3DX11EffectShaderResourceVariable* m_pNormalMapVariable;

		//Phong
		ID3DX11EffectShaderResourceVariable* m_pSpecularMapVariable;
		ID3DX11EffectShaderResourceVariable* m_pGlossMapVariable;

		//Sampler
		ID3DX11EffectSamplerVariable* m_pSamplerState;
		SamplerState m_currentSamplerState{ SamplerState::POINT };
	};

}