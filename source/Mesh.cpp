#include "pch.h"
#include "Mesh.h"
#include "Effect.h"
#include "Utils.h"
#include "Texture.h"

dae::Mesh::Mesh(ID3D11Device* pDevice, const std::string& objFilePath, Effect* pEffect)
	:m_pEffect{ pEffect }
{


	if (!Utils::ParseOBJ(objFilePath, m_Vertices, m_Indices))
	{
		std::cout << "Invalid filepath!\n";
	}


	m_pInputLayout = m_pEffect->CreateInputLayout(pDevice);

	//Create Vertex Buffer
	D3D11_BUFFER_DESC bd{};
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(Vertex) * static_cast<uint32_t>(m_Vertices.size());
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData{};
	initData.pSysMem = m_Vertices.data();

	HRESULT result = pDevice->CreateBuffer(&bd, &initData, &m_pVertexBuffer);
	if (FAILED(result))
	{
		std::wcout << L"Vertex Buffer creation failed!\n";
		return;
	}

	//Create Index Buffer
	m_NumIndices = static_cast<uint32_t>(m_Indices.size());
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(uint32_t) * m_NumIndices;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	initData.pSysMem = m_Indices.data();

	result = pDevice->CreateBuffer(&bd, &initData, &m_pIndexBuffer);
	if (FAILED(result))
	{
		std::wcout << L"Index Buffer creation failed!\n";
		return;
	}

}

dae::Mesh::~Mesh()
{

	if (m_pIndexBuffer) m_pIndexBuffer->Release();
	if (m_pVertexBuffer) m_pVertexBuffer->Release();
	if (m_pInputLayout) m_pInputLayout->Release();

	if (m_pEffect) delete m_pEffect;


	if (m_pDiffuseMap) delete m_pDiffuseMap;
	if (m_pNormalMap) delete m_pNormalMap;
	if (m_pGlossinessMap) delete m_pGlossinessMap;
	if (m_pSpecularMap) delete m_pSpecularMap;

}

void dae::Mesh::UpdateSampleState(ID3D11SamplerState* pSampleState)
{
	ID3DX11EffectSamplerVariable* pSamplerEffect{ m_pEffect->GetEffect()->GetVariableByName("gSampleState")->AsSampler() };
	HRESULT result{ pSamplerEffect->SetSampler(0, pSampleState) };
	if (FAILED(result))
	{
		std::wcout << L"Failed to update mesh sampler state\n";
		return;
	}
}

void dae::Mesh::UpdateCullMode(ID3D11RasterizerState* pRasterizerState)
{
	ID3DX11EffectRasterizerVariable* pCullingEffect{ m_pEffect->GetEffect()->GetVariableByName("gRasterizerState")->AsRasterizer() };


	HRESULT result{ pCullingEffect->SetRasterizerState(0, pRasterizerState) };
	if (FAILED(result))
	{
		std::wcout << L"Failed to update mesh Culling Mode\n";
		return;
	}
}


void dae::Mesh::SetMatrices(const Matrix& viewProjMatrix, const Matrix& inverseViewMatrix)
{
	m_pEffect->SetViewProjectionMatrix(m_WorldMatrix * viewProjMatrix);
	m_pEffect->SetViewInverseMatrix(inverseViewMatrix);
	m_pEffect->SetWorldMatrix(m_WorldMatrix);
}

void dae::Mesh::SetDiffuseMap(Texture* pDiffuseTexture)
{
	m_pDiffuseMap = pDiffuseTexture;
	m_pEffect->SetDiffuseMap(m_pDiffuseMap);
}

void dae::Mesh::SetNormalMap(Texture* pNormalTexture)
{
	m_pNormalMap = pNormalTexture;
	m_pEffect->SetNormalMap(m_pNormalMap);
}

void dae::Mesh::SetSpecularMap(Texture* pSpecularTexture)
{
	m_pSpecularMap = pSpecularTexture;
	m_pEffect->SetSpecularMap(m_pSpecularMap);
}

void dae::Mesh::SetGlossinessMap(Texture* pGlossinessTexture)
{
	m_pGlossinessMap = pGlossinessTexture;
	m_pEffect->SetGlossinessMap(m_pGlossinessMap);
}

dae::Texture* dae::Mesh::GetDiffuseMap() const
{
	return m_pDiffuseMap;
}

dae::Texture* dae::Mesh::GetNormalMap() const
{
	return m_pNormalMap;
}

dae::Texture* dae::Mesh::GetSpecularMap() const
{
	return m_pSpecularMap;
}

dae::Texture* dae::Mesh::GetGlossinessMap() const
{
	return m_pGlossinessMap;
}

ID3D11InputLayout* dae::Mesh::GetInputLayout()
{
	return m_pInputLayout;
}

ID3D11Buffer* dae::Mesh::GetVertexBuffer()
{
	return m_pVertexBuffer;
}

ID3D11Buffer* dae::Mesh::GetIndexBuffer()
{
	return m_pIndexBuffer;
}

dae::Effect* dae::Mesh::GetEffect()
{
	return m_pEffect;
}

uint32_t dae::Mesh::GetNumIndices()
{
	return m_NumIndices;
}

const std::vector<dae::Vertex>& dae::Mesh::GetVertices() const
{
	return m_Vertices;
}

const std::vector<uint32_t>& dae::Mesh::GetIndices() const
{
	return m_Indices;
}

const std::vector<dae::Vertex_Out>& dae::Mesh::GetVerticesOut() const
{
	return m_VerticesOut;
}

void dae::Mesh::SetVerticesOut(const std::vector<Vertex_Out>& newVerticesOut)
{
	m_VerticesOut.clear();
	m_VerticesOut = newVerticesOut;
}

const dae::Matrix& dae::Mesh::GetWorldMatrix() const
{
	return m_WorldMatrix;
}

const dae::Mesh::PrimitiveTopology& dae::Mesh::GetPrimitiveTopology() const
{
	return m_PrimitiveTopology;
}

const dae::Mesh::CullMode& dae::Mesh::GetCullMode() const
{
	return m_CullMode;
}

void dae::Mesh::SetCullMode(const CullMode& cullMode)
{
	m_CullMode = cullMode;
}

void dae::Mesh::SetWorldMatrix(const Matrix& worldMatrix)
{
	m_WorldMatrix = worldMatrix;
}

bool dae::Mesh::IsActive() const
{
	return m_Enabled;
}

void dae::Mesh::SetActive(bool enabled)
{
	m_Enabled = enabled;
}

bool dae::Mesh::IsTransparent() const
{
	return m_IsTransparent;
}

void dae::Mesh::SetTransparent(bool isTransparent)
{
	m_IsTransparent = isTransparent;
}
