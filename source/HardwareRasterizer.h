#pragma once
#include "Camera.h"
#include "Mesh.h"
struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	class Texture;

	class HardwareRasterizer final
	{
	public:
		HardwareRasterizer(SDL_Window* pWindow);
		~HardwareRasterizer();

		HardwareRasterizer(const HardwareRasterizer&) = delete;
		HardwareRasterizer(HardwareRasterizer&&) noexcept = delete;
		HardwareRasterizer& operator=(const HardwareRasterizer&) = delete;
		HardwareRasterizer& operator=(HardwareRasterizer&&) noexcept = delete;


		void NextSampleStateFilter(std::vector<Mesh*> pMeshes);
		void NextCullingMode(std::vector<Mesh*> pMeshes, Mesh::CullMode cullMode);


		void HardwareRender(std::vector<Mesh*> pMeshes, bool isBackgroundUniform) const;

		ID3D11Device* GetDevice();



	private:
		SDL_Window* m_pWindow{};

		int m_Width{};
		int m_Height{};

		bool m_IsInitialized{ false };

		//DIRECTX
		ID3D11Device* m_pDevice;
		ID3D11DeviceContext* m_pDeviceContext;
		IDXGISwapChain* m_pSwapChain;

		ID3D11Texture2D* m_pDepthStencilBuffer;
		ID3D11DepthStencilView* m_pDepthStencilView;

		ID3D11Resource* m_pRenderTargetBuffer;
		ID3D11RenderTargetView* m_pRenderTargetView;

		ID3D11SamplerState* m_pSamplerState{ nullptr };

		ID3D11RasterizerState* m_pCullingMode{ nullptr };

		enum class SampleStateFilters
		{
			POINT,
			LINEAR,
			ANISOTROPIC
		};
		SampleStateFilters m_SampleFilters{ 0 };

		HRESULT InitializeDirectX();
		void LoadSampleState(const D3D11_FILTER& filter, ID3D11Device* device, std::vector<Mesh*> pMeshes);
		void LoadCullMode(D3D11_CULL_MODE cullMode, const std::vector<Mesh*>& pMeshes);
		void Render(Mesh* pMesh) const;
		//...


	};
}
