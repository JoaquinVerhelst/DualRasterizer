#pragma once


namespace dae
{

	class Texture;


	class Effect
	{
	public:
		Effect(ID3D11Device* pDevice, const std::wstring& assetFile);
		virtual ~Effect();

		Effect(const Effect&) = delete;
		Effect(Effect&&) noexcept = delete;
		Effect& operator=(const Effect&) = delete;
		Effect& operator=(Effect&&) noexcept = delete;


		static ID3DX11Effect* LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile);


		ID3DX11Effect* GetEffect() const;
		ID3DX11EffectTechnique* GetTechnique() const;

		virtual ID3D11InputLayout* CreateInputLayout(ID3D11Device* pDevice) const = 0;

		void SetViewProjectionMatrix(const Matrix& matrix);
		void SetWorldMatrix(const Matrix& matrix);
		void SetViewInverseMatrix(const Matrix& matrix);

		virtual void SetDiffuseMap(const Texture* pDiffuseTexture) = 0;
		virtual void SetNormalMap(const Texture* pNormalTexture) = 0;
		virtual void SetSpecularMap(const Texture* pSpecularTexture) = 0;
		virtual void SetGlossinessMap(const Texture* pGlossinessTexture) = 0;



	protected:



		ID3DX11Effect* m_pEffect{ nullptr };
		ID3DX11EffectTechnique* m_pTechnique{ nullptr };
		ID3DX11EffectMatrixVariable* m_pMatWorldViewProjVar{ nullptr };
		ID3DX11EffectMatrixVariable* m_pMatWorldVar{ nullptr };
		ID3DX11EffectMatrixVariable* m_pMatViewInverseVar{ nullptr };
	};
}


