#pragma once


#include <cstdint>
#include <vector>

#include "Camera.h"
#include "DataTypes.h"

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	class Texture;
	class Mesh;
	struct Vertex;
	class Timer;
	class Scene;

	class SoftwareRasterizer final
	{
	public:
		SoftwareRasterizer(SDL_Window* pWindow);
		~SoftwareRasterizer();

		SoftwareRasterizer(const SoftwareRasterizer&) = delete;
		SoftwareRasterizer(SoftwareRasterizer&&) noexcept = delete;
		SoftwareRasterizer& operator=(const SoftwareRasterizer&) = delete;
		SoftwareRasterizer& operator=(SoftwareRasterizer&&) noexcept = delete;


		void Update(Timer* pTimer);

		void SoftwareRender(std::vector<Mesh*>& pMeshes, Camera& camera, bool isBackgroundUniform) const;

		bool SaveBufferToImage() const;



		void NextShadingMode();
		void ToggleRenderMode();
		void ToggleNormalMap();
		void ToggleBoundingBox();
		void NextColorShadingMode();

		void AdjustGammaCorrection(bool lowerIt);



	private:

		enum class ColorShadingMode
		{
			Gamma,
			MaxToOne,
			Filmic,

			COUNT
		};

		enum class RenderMode
		{
			Default,
			Depth,
			
			COUNT
		};

		enum class ShadingMode
		{
			ObservedArea,
			Diffuse,
			Specular,
			Combined,

			COUNT
		};

		RenderMode m_RenderMode{ RenderMode::Default };
		ShadingMode m_ShadingMode{ ShadingMode::Combined };
		ColorShadingMode m_ColorShadingMode{ ColorShadingMode::Gamma };

		bool m_UseNormalMaps{ true };
		bool m_RotateMesh{ true };

		SDL_Window* m_pWindow{};

		SDL_Surface* m_pFrontBuffer{ nullptr };
		SDL_Surface* m_pBackBuffer{ nullptr };

		uint32_t* m_pBackBufferPixels{};
		float* m_pDepthBufferPixels{};


		int m_Width{};
		int m_Height{};

		Vector3 m_LightDirection{ 0.577f, -0.577f, 0.577f };

		float m_GammaCorrection{1.6f};
		bool m_IsGammaCorrectionEnabled{ true } ;


		bool m_IsShowingBoundingBoxes{ false };

		bool CheckCullMode(const Mesh* pMesh, const float edge01, const float edge02, const float edge03) const;

		void VertexTransformationFunction(Mesh* pMesh, Camera& camera) const;

		void ResetDepthBufferAndClearBackground(bool isBackgroundUniform) const;

		bool IsVertexInFrustrum(const Vector4& vertex, float min = -1.f, float max = 1.f) const;

		void Render(Mesh* pMesh, Camera& camera) const;

		void RenderMeshTriangle(const Mesh* pMesh, const std::vector<Vector2>& verticesScreen, size_t currentVertexIdx, bool swapVertices = false) const;

		void PixelShading(const Vertex_Out& pixel, const Mesh* pMesh, int pixelIdx) const;

		void UpdateColorInBuffer(int px, int py, ColorRGB& finalColor, bool isTransparent = false) const;


		ColorRGB ApplyGammaCorrection(const ColorRGB& color) const;
		ColorRGB ApplyFilmicToneMapping(const ColorRGB& color) const;

	};
}
