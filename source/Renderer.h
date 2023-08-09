#pragma once
#include "Camera.h"
#include "Mesh.h"
#include "HardwareRasterizer.h"
#include "SoftwareRasterizer.h"


struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Update(const Timer* pTimer);
		void Render() ;

		//SHARED
		void NextRasterizerMode();
		void ToggleRotateMesh();

		//HARDWARE
		void ToggleFireMesh();
		void NextSampleStateFilter();

		// SOFTWARE
		void NextShadingMode();
		void ToggleRenderMode();
		void ToggleNormalMap();
		void ToggleBoundingBox();


		void ToggleCullMode();
		void ToggleUniformBackground();

		void ToggleColorShadingMode();
		void AdjustGammaCorrection(bool lowerIt);

	private:

		enum class Rasterizers
		{
			Hardware,
			Software,

			COUNT
		};

		Rasterizers m_CurrentRenderer{ Rasterizers::Hardware };
		Mesh::CullMode m_CullMode;

		SDL_Window* m_pWindow{};
		Camera m_Camera;

		std::vector<Mesh*> m_pMeshes;
		Mesh* m_pFireMesh;

		int m_Width{};
		int m_Height{};

		bool m_IsInitialized{ false };

		HardwareRasterizer* m_pHardwareRasterizer{};
		SoftwareRasterizer* m_pSoftwareRasterizer{};

		bool m_ShouldRotateMesh{ true };
		bool m_IsBackgroundUniform{ false };

	};
}
