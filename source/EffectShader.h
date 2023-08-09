#pragma once
#include "Effect.h"
namespace dae
{
	class Texture;
	class EffectShader final : public Effect
	{
	public:

		EffectShader(ID3D11Device* pDevice, const std::wstring& assetFile);
		virtual ~EffectShader();
		EffectShader(const EffectShader&) = delete;
		EffectShader(EffectShader&&) noexcept = delete;
		EffectShader& operator=(const EffectShader&) = delete;
		EffectShader& operator=(EffectShader&&) noexcept = delete;

		void SetDiffuseMap(const Texture* pDiffuseTexture) override;
		void SetNormalMap(const Texture* pNormalTexture) override;
		void SetSpecularMap(const Texture* pSpecularTexture) override;
		void SetGlossinessMap(const Texture* pGlossinessTexture) override;

		virtual ID3D11InputLayout* CreateInputLayout(ID3D11Device* pDevice) const;

	private:
		ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVar{ nullptr };
		ID3DX11EffectShaderResourceVariable* m_pNormalMapVar{ nullptr };
		ID3DX11EffectShaderResourceVariable* m_pSpecularMapVar{ nullptr };
		ID3DX11EffectShaderResourceVariable* m_pGlossinessMapVar{ nullptr };
	};
}

