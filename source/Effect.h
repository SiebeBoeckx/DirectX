#pragma once

using namespace dae;

class Effect
{
public:
	Effect(ID3D11Device* pDevice, const std::wstring& assetFile);
	virtual ~Effect();


	ID3DX11Effect* GetEffect() const { return m_pEffect; } ;
	ID3DX11EffectTechnique* GetTechnique() const { return m_pTechnique; } ;
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

