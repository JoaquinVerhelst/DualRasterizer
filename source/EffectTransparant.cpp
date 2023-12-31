#include "pch.h"
#include "EffectTransparant.h"
#include "Texture.h"

namespace dae
{
	EffectTransparant::EffectTransparant(ID3D11Device* pDevice, const std::wstring& assetFile)
		:Effect(pDevice, assetFile)
	{
		m_pDiffuseMapVar = m_pEffect->GetVariableByName("gDiffuseMap")->AsShaderResource();

		if (!m_pDiffuseMapVar->IsValid())
		{
			std::wcout << L"m_pDiffuseMapVar not valid\n";
		}
	}

	EffectTransparant::~EffectTransparant()
	{
		if (m_pDiffuseMapVar) m_pDiffuseMapVar->Release();
	}

	void EffectTransparant::SetDiffuseMap(const Texture* pDiffuseTexture)
	{
		if (m_pDiffuseMapVar)
		{
			m_pDiffuseMapVar->SetResource(pDiffuseTexture->GetSRV());
		}

		//delete pDiffuseTexture;
	}

	ID3D11InputLayout* EffectTransparant::CreateInputLayout(ID3D11Device* pDevice) const
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