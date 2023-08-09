#include "pch.h"
#include "Effect.h"
#include "Texture.h"

dae::Effect::Effect(ID3D11Device* pDevice, const std::wstring& assetFile)
	:m_pEffect{ LoadEffect(pDevice, assetFile) }
{

	m_pTechnique = m_pEffect->GetTechniqueByName("DefaultTechnique");
	if (!m_pTechnique->IsValid())
		std::wcout << L"Technique not valid\n";

	m_pMatWorldViewProjVar = m_pEffect->GetVariableByName("gWorldViewProj")->AsMatrix();
	if (!m_pMatWorldViewProjVar->IsValid())
	{
		std::wcout << L"m_pMatViewProjVar is not valid!\n";
	}

	m_pMatWorldVar = m_pEffect->GetVariableByName("gWorldMatrix")->AsMatrix();
	if (!m_pMatWorldVar->IsValid())
	{
		std::wcout << L"m_pMatWorldVar is not valid!\n";
	}

	m_pMatViewInverseVar = m_pEffect->GetVariableByName("gInverseViewMatrix")->AsMatrix();
	if (!m_pMatViewInverseVar->IsValid())
	{
		std::wcout << L"m_pMatViewInverseVar is not valid!\n";
	}


}

dae::Effect::~Effect()
{

	if (m_pMatWorldViewProjVar)
	{
		m_pMatWorldViewProjVar->Release();
		m_pMatWorldViewProjVar = nullptr;
	}

	if (m_pMatWorldVar)
	{
		m_pMatWorldVar->Release();
		m_pMatWorldVar = nullptr;
	}
	if (m_pMatViewInverseVar)
	{
		m_pMatViewInverseVar->Release();
		m_pMatViewInverseVar = nullptr;
	}


	if (m_pTechnique)
	{
		m_pTechnique->Release();
		m_pTechnique = nullptr;
	}
	if (m_pEffect)
	{
		m_pEffect->Release();
		m_pEffect = nullptr;

	}


}

ID3DX11Effect* dae::Effect::LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile)
{
	HRESULT result;
	ID3D10Blob* pErrorBlob{ nullptr };
	ID3DX11Effect* pEffect;

	DWORD shaderFlags{ 0 };
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
		if (pErrorBlob)
		{
			const char* pErrors{ static_cast<char*>(pErrorBlob->GetBufferPointer()) };

			std::wstringstream ss;
			for (unsigned int i{ 0 }; i < pErrorBlob->GetBufferSize(); ++i)
			{
				ss << pErrors[i];
			}
			OutputDebugStringW(ss.str().c_str());
			pErrorBlob->Release();
			pErrorBlob = nullptr;

			std::wcout << ss.str() << std::endl;
		}
		else
		{
			std::wstringstream ss;
			ss << "EffectLoader: Failed to CreateEffectFromFile!\nPath: " << assetFile;
			std::wcout << ss.str() << std::endl;
		}
		return nullptr;
	}

	return pEffect;
}

ID3DX11Effect* dae::Effect::GetEffect() const
{
	return m_pEffect;
}

ID3DX11EffectTechnique* dae::Effect::GetTechnique() const
{
	return m_pTechnique;
}

ID3D11InputLayout* dae::Effect::CreateInputLayout(ID3D11Device* pDevice) const
{
	//Create Vertex Layout
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

void dae::Effect::SetViewProjectionMatrix(const Matrix& matrix)
{
	m_pMatWorldViewProjVar->SetMatrix(reinterpret_cast<const float*>(&matrix));
}

void dae::Effect::SetWorldMatrix(const Matrix& matrix)
{
	m_pMatWorldVar->SetMatrix(reinterpret_cast<const float*>(&matrix));
}

void dae::Effect::SetViewInverseMatrix(const Matrix& matrix)
{
	m_pMatViewInverseVar->SetMatrix(reinterpret_cast<const float*>(&matrix));
}
