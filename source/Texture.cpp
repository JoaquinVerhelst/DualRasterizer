#include "pch.h"
#include "Texture.h"

namespace dae
{
	Texture::Texture(ID3D11Device* pDevice, SDL_Surface* pSurface)
		: m_pSurface{ pSurface }
		, m_pSurfacePixels{ static_cast<uint32_t*>(pSurface->pixels) }
	{
		DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
		D3D11_TEXTURE2D_DESC desc{};
		desc.Width = pSurface->w;
		desc.Height = pSurface->h;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = format;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA initData;
		initData.pSysMem = pSurface->pixels;
		initData.SysMemPitch = static_cast<UINT>(pSurface->pitch);
		initData.SysMemSlicePitch = static_cast<UINT>(pSurface->h * pSurface->pitch);

		HRESULT hr = pDevice->CreateTexture2D(&desc, &initData, &m_pResource);

		if (FAILED(hr))
		{
			std::wcout << L"Texture 2D Resource creation failed!\n";
			return;
		}

		//Create Shader Resource View
		D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc{};
		SRVDesc.Format = format;
		SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		SRVDesc.Texture2D.MipLevels = 1;

		hr = pDevice->CreateShaderResourceView(m_pResource, &SRVDesc, &m_pSRV);
		if (FAILED(hr))
		{
			std::wcout << L"Shader Resource View creation failed!\n";
			return;
		}

		////Release pSurface, as it's not necessary anymore
		//SDL_FreeSurface(pSurface);
	}

	Texture::~Texture()
	{
		if (m_pSRV) m_pSRV->Release();
		if (m_pResource) m_pResource->Release();

		if (m_pSurface)
		{
			SDL_FreeSurface(m_pSurface);
			//m_pSurface = nullptr;
		}
	}

	ID3D11ShaderResourceView* Texture::GetSRV() const
	{
		return m_pSRV;
	}

	ColorRGB Texture::Sample(const Vector2& uv) const
	{
		
		//const size_t x{ static_cast<size_t>(uv.x * m_pSurface->w) };
		//const size_t y{ static_cast<size_t>(uv.y * m_pSurface->h) };

		const int x{ static_cast<int>(std::clamp(uv.x, 0.0f, 1.0f) * m_pSurface->w) };
		const int y{ static_cast<int>(std::clamp(uv.y, 0.0f, 1.0f) * m_pSurface->h) };

		uint8_t r{}, g{}, b{}, a{};

		const Uint32 pixel{ m_pSurfacePixels[x + y * m_pSurface->w] };

		SDL_GetRGBA(pixel, m_pSurface->format, &r, &g, &b, &a);

		const float clampColorValue{ 1.f / 255.f };

		return ColorRGB{ r * clampColorValue, g * clampColorValue, b * clampColorValue, a * clampColorValue };
	}

	Texture* Texture::LoadFromFile(ID3D11Device* pDevice, const std::string& path)
	{
		SDL_Surface* pSurface{ IMG_Load(path.c_str()) };
		if (!pSurface)
		{
			std::wcout << L"Texture surface creation failed!\n";
			return nullptr;
		}

		return new Texture(pDevice, pSurface);
	}
}
