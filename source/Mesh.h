#pragma once
#include "DataTypes.h"


namespace dae
{



	class Effect;
	class Texture;


	class Mesh final
	{
	public:

		enum class PrimitiveTopology
		{
			TriangleList,
			TriangleStrip
		};

		enum class CullMode
		{
			Back,
			Front,
			None,
			
			COUNT
		};


		Mesh(ID3D11Device* pDevice, const std::string& objFilePath, Effect* pEffect);
		~Mesh();

		Mesh(const Mesh&) = delete;
		Mesh(Mesh&&) noexcept = delete;
		Mesh& operator=(const Mesh&) = delete;
		Mesh& operator=(Mesh&&) noexcept = delete;

		void UpdateSampleState(ID3D11SamplerState* pSampleState);
		void UpdateCullMode(ID3D11RasterizerState* pRasterizerState);


		void SetMatrices(const Matrix& viewProjMatrix, const Matrix& inverseViewMatrix);



		void SetDiffuseMap(Texture* pDiffuseTexture);
		void SetNormalMap(Texture* pNormalTexture);
		void SetSpecularMap(Texture* pSpecularTexture);
		void SetGlossinessMap(Texture* pGlossinessTexture);

		Texture* GetDiffuseMap() const;
		Texture* GetNormalMap() const;
		Texture* GetSpecularMap() const;
		Texture* GetGlossinessMap() const;

		// Getters and setters
		ID3D11InputLayout* GetInputLayout();
		ID3D11Buffer* GetVertexBuffer();
		ID3D11Buffer* GetIndexBuffer();
		Effect* GetEffect();
		uint32_t GetNumIndices();


		const std::vector<Vertex>& GetVertices() const;
		const std::vector<uint32_t>& GetIndices() const;
		const std::vector<Vertex_Out>& GetVerticesOut() const;
		void SetVerticesOut(const std::vector<Vertex_Out>& newVerticesOut);

		const Matrix& GetWorldMatrix() const;
		const PrimitiveTopology& GetPrimitiveTopology() const;
		const CullMode& GetCullMode() const;
		void SetCullMode(const CullMode& cullMode);

		void SetWorldMatrix(const Matrix& worldMatrix);
		bool IsActive() const;
		void SetActive(bool enabled);

		bool IsTransparent() const;
		void SetTransparent(bool isTransparent);


	private:

		bool m_Enabled{true};
		bool m_IsTransparent{false};


		Effect* m_pEffect{ nullptr };
		ID3D11Buffer* m_pVertexBuffer{ nullptr };
		ID3D11Buffer* m_pIndexBuffer{ nullptr };
		ID3D11InputLayout* m_pInputLayout{ nullptr };

		uint32_t m_NumIndices{};
		Matrix m_RotationMatrix{};



		std::vector<Vertex> m_Vertices{};
		std::vector<uint32_t> m_Indices{};

		std::vector<Vertex_Out> m_VerticesOut{};

		PrimitiveTopology m_PrimitiveTopology{ PrimitiveTopology::TriangleList };
		CullMode m_CullMode{ CullMode::None };

		Matrix m_WorldMatrix{};


		Texture* m_pDiffuseMap{ nullptr };
		Texture* m_pNormalMap{ nullptr };
		Texture* m_pGlossinessMap{ nullptr };
		Texture* m_pSpecularMap{ nullptr };
	};
}
