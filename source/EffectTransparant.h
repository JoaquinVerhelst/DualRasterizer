#pragma once
#include "Effect.h"
namespace dae
{
	class Texture;
	class EffectTransparant final: public Effect
	{
	public:

		EffectTransparant(ID3D11Device* pDevice, const std::wstring& assetFile);
		virtual ~EffectTransparant();
		EffectTransparant(const EffectTransparant&) = delete;
		EffectTransparant(EffectTransparant&&) noexcept = delete;
		EffectTransparant& operator=(const EffectTransparant&) = delete;
		EffectTransparant& operator=(EffectTransparant&&) noexcept = delete;


		void SetDiffuseMap(const Texture* pDiffuseTexture) override;
		void SetNormalMap(const Texture* pNormalTexture) override {};
		void SetSpecularMap(const Texture* pSpecularTexture) override {};
		void SetGlossinessMap(const Texture* pGlossinessTexture) override {};


		virtual ID3D11InputLayout* CreateInputLayout(ID3D11Device* pDevice) const;

	private:
		ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVar{ nullptr };
	};
}
