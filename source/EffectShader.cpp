#include "pch.h"
#include "EffectShader.h"
#include "Texture.h"

namespace dae
{
	EffectShader::EffectShader(ID3D11Device* pDevice, const std::wstring& assetFile)
		:Effect(pDevice, assetFile)
	{
		m_pDiffuseMapVar = m_pEffect->GetVariableByName("gDiffuseMap")->AsShaderResource();
		if (!m_pDiffuseMapVar->IsValid())
		{
			std::wcout << L"m_pDiffuseMapVar not valid\n";
		}

		m_pNormalMapVar = m_pEffect->GetVariableByName("gNormalMap")->AsShaderResource();
		if (!m_pNormalMapVar->IsValid())
		{
			std::wcout << L"m_pNormalMapVar not valid\n";
		}

		m_pSpecularMapVar = m_pEffect->GetVariableByName("gSpecularMap")->AsShaderResource();
		if (!m_pSpecularMapVar->IsValid())
		{
			std::wcout << L"m_pSpecularMapVar not valid\n";
		}

		m_pGlossinessMapVar = m_pEffect->GetVariableByName("gGlossinessMap")->AsShaderResource();
		if (!m_pGlossinessMapVar->IsValid())
		{
			std::wcout << L"m_pGlossinessMapVar not valid\n";
		}
	}

	EffectShader::~EffectShader()
	{
		if (m_pDiffuseMapVar) m_pDiffuseMapVar->Release();
		if (m_pNormalMapVar) m_pNormalMapVar->Release();
		if (m_pSpecularMapVar) m_pSpecularMapVar->Release();
		if (m_pGlossinessMapVar) m_pGlossinessMapVar->Release();
	}

	void EffectShader::SetDiffuseMap(const Texture* pDiffuseTexture)
	{
		if (m_pDiffuseMapVar)
		{
			m_pDiffuseMapVar->SetResource(pDiffuseTexture->GetSRV());
		}

		//delete pDiffuseTexture;
	}

	void EffectShader::SetNormalMap(const Texture* pNormalTexture)
	{
		if (m_pNormalMapVar)
		{
			m_pNormalMapVar->SetResource(pNormalTexture->GetSRV());
		}


		//delete pNormalTexture;
	}

	void EffectShader::SetSpecularMap(const Texture* pSpecularTexture)
	{
		if (m_pSpecularMapVar)
		{
			m_pSpecularMapVar->SetResource(pSpecularTexture->GetSRV());
		}


		//delete pSpecularTexture;
	}

	void EffectShader::SetGlossinessMap(const Texture* pGlossinessTexture)
	{
		if (m_pGlossinessMapVar)
		{
			m_pGlossinessMapVar->SetResource(pGlossinessTexture->GetSRV());
		}


		//delete pGlossinessTexture;
	}

	ID3D11InputLayout* EffectShader::CreateInputLayout(ID3D11Device* pDevice) const
	{
		//Create Vertex Layout
		static constexpr uint32_t numElements{ 4 };
		D3D11_INPUT_ELEMENT_DESC vertexDesc[numElements]{};
		vertexDesc[0].SemanticName = "POSITION";
		vertexDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		vertexDesc[0].AlignedByteOffset = 0;
		vertexDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

		vertexDesc[3].SemanticName = "TEXCOORD";
		vertexDesc[3].Format = DXGI_FORMAT_R32G32_FLOAT;
		vertexDesc[3].AlignedByteOffset = 12;
		vertexDesc[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

		vertexDesc[1].SemanticName = "NORMAL";
		vertexDesc[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		vertexDesc[1].AlignedByteOffset = 20;
		vertexDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

		vertexDesc[2].SemanticName = "TANGENT";
		vertexDesc[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		vertexDesc[2].AlignedByteOffset = 32;
		vertexDesc[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

		

		//Create Input Layout
		D3DX11_PASS_DESC passDesc{};
		GetTechnique()->GetPassByIndex(0)->GetDesc(&passDesc);

		ID3D11InputLayout* pInputLayout{ nullptr };

		HRESULT result = pDevice->CreateInputLayout(
			vertexDesc,
			numElements,
			passDesc.pIAInputSignature,
			passDesc.IAInputSignatureSize,
			&pInputLayout
		);

		if (FAILED(result))
		{
			std::wcout << L"Input Layout creation failed!\n";
			return nullptr;
		}

		return pInputLayout;
	}
}
