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

	class Effect_PosTex 
	{
	public:
		Effect_PosTex(ID3D11Device* pDevice, const std::wstring& assetFile);
		virtual ~Effect_PosTex();

		void Initialize();

		ID3DX11Effect* GetEffect() const { return m_pEffect; };
		ID3DX11EffectTechnique* GetTechnique() const { return m_pTechnique; };

		ID3DX11EffectMatrixVariable* GetWorldMatrix() { return m_pMatWorldVariable; };
		ID3DX11EffectMatrixVariable* GetWorldViewProjMatrix() { return m_pMatWorldViewProjVariable; };
		ID3DX11EffectMatrixVariable* GetInvViewMatrix() { return m_pMatInvViewVariable; };

		void CycleSampleState();

		enum class SamplerState
		{
			POINT,
			LINEAR,
			ANISOTROPIC
		};

		void CycleCullMode();

		enum class CullMode
		{
			BACK,
			FRONT,
			NONE
		};

	protected:
		virtual void SetTextures() = 0;
		static ID3DX11Effect* LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile);
		void SetSamplerState(SamplerState state);
		void SetCullMode(CullMode mode);

		ID3D11Device* m_pDevice;
		const std::wstring& m_assetFile;
		ID3DX11Effect* m_pEffect;
		ID3DX11EffectTechnique* m_pTechnique;

		ID3DX11EffectMatrixVariable* m_pMatWorldVariable{};
		//Camera
		ID3DX11EffectMatrixVariable* m_pMatWorldViewProjVariable{};
		ID3DX11EffectMatrixVariable* m_pMatInvViewVariable{};

		//Sampler
		ID3D11SamplerState* m_pPointSampler;
		ID3D11SamplerState* m_pLinearSampler;
		ID3D11SamplerState* m_pAnisotropicSampler;

		ID3DX11EffectSamplerVariable* m_pSamplerState;
		SamplerState m_currentSamplerState{ SamplerState::POINT };

		//Sampler
		ID3D11RasterizerState* m_pBackCullingMode;
		ID3D11RasterizerState* m_pFrontCullingMode;
		ID3D11RasterizerState* m_pNoCullingMode;

		ID3DX11EffectRasterizerVariable* m_pCullMode;
		CullMode m_currentCullMode{ CullMode::BACK };
	};

	class Effect_PosTexVehicle final : public Effect_PosTex
	{
	public:
		Effect_PosTexVehicle(ID3D11Device* pDevice, const std::wstring& assetFile) : Effect_PosTex(pDevice, assetFile){}
		virtual ~Effect_PosTexVehicle() = default;

		void SetDiffuseMap(Texture* pDiffuseMap);
		void SetNormalMap(Texture* pNormalMap);
		void SetSpecularMap(Texture* pSpecularMap);
		void SetGlossMap(Texture* pGlossMap);

	private:
		void SetTextures() override;

		static ID3DX11Effect* LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile);
		void SetSamplerState(SamplerState state);

		//Texture
		ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable;
		
		//Normal
		ID3DX11EffectShaderResourceVariable* m_pNormalMapVariable;

		//Phong
		ID3DX11EffectShaderResourceVariable* m_pSpecularMapVariable;
		ID3DX11EffectShaderResourceVariable* m_pGlossMapVariable;
	};

	class Effect_PosTexFire final : public Effect_PosTex
	{
	public:
		Effect_PosTexFire(ID3D11Device* pDevice, const std::wstring& assetFile) : Effect_PosTex(pDevice, assetFile) {}
		virtual ~Effect_PosTexFire() = default;

		void SetDiffuseMap(Texture* pDiffuseMap);
	private:
		void SetTextures() override;

		static ID3DX11Effect* LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile);
		void SetSamplerState(SamplerState state);

		//Texture
		ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable;
	};
}