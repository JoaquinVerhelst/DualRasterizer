#include "pch.h"
#include "Renderer.h"
#include "Mesh.h"
#include "Effect.h"
#include "Texture.h"
#include "Utils.h"
#include "EffectShader.h"
#include "EffectTransparant.h"

namespace dae {

	Renderer::Renderer(SDL_Window* pWindow) :
		m_pWindow(pWindow),
		m_CullMode( Mesh::CullMode::Back )
	{
		//Initialize
		SDL_GetWindowSize(pWindow, &m_Width, &m_Height);


		// Camera
		m_Camera = Camera{ 60.f, { 0.0f, 0.0f, -50.f }, static_cast<float>(m_Width) / m_Height };


		//Renderers
		m_pHardwareRasterizer = new HardwareRasterizer{ pWindow };
		m_pSoftwareRasterizer = new SoftwareRasterizer{ pWindow };


		std::cout << "[SHARED KEY BINDINGS]" << '\n';
		std::cout << "[F1]  Toggle Rasterizer Mode (HARDWARE / SOFTWARE)" << '\n';
		std::cout << "[F2]  Toggle Vehicle Rotation (ON / OFF)" << '\n';
		std::cout << "[F3]  Toggle FireFX (ON / OFF)" << '\n';
		std::cout << "[F9]  Cycle CullModes (BACK / FRONT / NONE)" << '\n';
		std::cout << "[F10] Toggle Uniform ClearColor (ON / OFF)" << "\n";
		std::cout << "[F11] Toggle Print FPS (ON / OFF)" << "\n";

		std::cout << "\n";
		std::cout << "\n";

		std::cout << "[Key Bindings - HARDWARE]" << "\n";
		std::cout << "[F4]  Cycle Sampler State (POINT / LINEAR / ANISOTROPIC)" << "\n";

		std::cout << "\n";
		std::cout << "\n";

		std::cout << "[Key Bindings - SOFTWARE]" << "\n";
		std::cout << "[F5]  Cycle Shading Modes (COMBINED / OBSERVED_AREA / DIFFUSE / SPECULAR)" << "\n";
		std::cout << "[F6]  Toggle NormalMap (ON / OFF)" << "\n";
		std::cout << "[F7]  Toggle DepthBuffer Visualization (ON / OFF)" << "\n";
		std::cout << "[F8]  Toggle BoundingBox Visualization (ON / OFF)" << "\n";
		std::cout << "[G]  Cycle Color Shading Modes (GAMMA / MAX_TO_POINT / FILMIC )" << "\n";

		std::cout << "[Up arrow]  Increases Gamma Correction" << "\n";
		std::cout << "[Down arrow]  Decreases Gamma Correction" << "\n";


		std::cout << "\n";
		std::cout << "\n";

		std::cout << "Extra's: Transparency, Gamma Correction and MultiThreading are added to the Software Rasterizer" << "\n";
		std::cout << "Also an attempt to Filmic ToneMapping has been added to the Software Rasterizer" << "\n";

		std::cout << "\n";
		std::cout << "\n";




		//Vehicle 

		ID3D11Device* pDevice{ m_pHardwareRasterizer->GetDevice() };

		EffectShader* pVehicleEffect = new EffectShader(pDevice, L"Resources/Shader3D.fx");

		Mesh* tempMesh = new Mesh{ pDevice, "Resources/vehicle.obj", pVehicleEffect };

		tempMesh->SetDiffuseMap(Texture::LoadFromFile(pDevice, "Resources/vehicle_diffuse.png"));
		tempMesh->SetNormalMap(Texture::LoadFromFile(pDevice, "Resources/vehicle_normal.png"));
		tempMesh->SetSpecularMap(Texture::LoadFromFile(pDevice, "Resources/vehicle_specular.png"));
		tempMesh->SetGlossinessMap(Texture::LoadFromFile(pDevice, "Resources/vehicle_gloss.png"));


		m_pMeshes.emplace_back(tempMesh);


		//////////Fire Combustion

		EffectTransparant* pCombustionEffect = new EffectTransparant(pDevice, L"Resources/Transparent3D.fx");

		tempMesh = new Mesh{ pDevice, "Resources/fireFx.obj", pCombustionEffect };

		tempMesh->SetDiffuseMap(Texture::LoadFromFile(pDevice, "Resources/fireFX_diffuse.png"));
		tempMesh->SetTransparent(true);
		tempMesh->SetCullMode(Mesh::CullMode::None);

		m_pMeshes.emplace_back(tempMesh);

		m_pFireMesh = tempMesh;


	}

	Renderer::~Renderer()
	{
		delete m_pHardwareRasterizer;
		delete m_pSoftwareRasterizer;

		//delete m_pCamera;

		for (Mesh* pMesh : m_pMeshes)
		{
			delete pMesh;
		}

	}

	void Renderer::Update(const Timer* pTimer)
	{
		m_Camera.Update(pTimer);


		constexpr float rotationSpeed{ 45.0f * TO_RADIANS };

		for (Mesh* pMesh : m_pMeshes)
		{
			if (m_ShouldRotateMesh)
			{
				float rotationSpeedRadian = 1.f;
				pMesh->SetWorldMatrix(Matrix::CreateRotationY(rotationSpeedRadian * pTimer->GetElapsed()) * pMesh->GetWorldMatrix());
			}

			pMesh->SetMatrices(m_Camera.GetViewMatrix() * m_Camera.GetProjectionMatrix(), m_Camera.GetInverseViewMatrix());
		}
	}


	void Renderer::Render() 	{
		switch (m_CurrentRenderer)
		{
			case dae::Renderer::Rasterizers::Software:
			{
				m_pSoftwareRasterizer->SoftwareRender(m_pMeshes, m_Camera, m_IsBackgroundUniform);
				break;
			}
			case dae::Renderer::Rasterizers::Hardware:
			{
				m_pHardwareRasterizer->HardwareRender(m_pMeshes, m_IsBackgroundUniform);
				break;
			}
		}
	}
	void Renderer::NextRasterizerMode()
	{
		m_CurrentRenderer = static_cast<Rasterizers>((static_cast<int>(m_CurrentRenderer) + 1) % (static_cast<int>(Rasterizers::COUNT)));

		switch (m_CurrentRenderer)
		{
		case Rasterizers::Software:
			std::cout << "RASTERIZER MODE: Software" << "\n";
			break;
		case Rasterizers::Hardware:
			std::cout << "RASTERIZER MODE: Hardware" << "\n";
			break;
		}
		
	}
	void Renderer::ToggleRotateMesh()
	{

		m_ShouldRotateMesh = !m_ShouldRotateMesh;


		if(m_ShouldRotateMesh) 
			std::cout << "ROTATE MESH : True" << "\n";
		else
			std::cout << "ROTATE MESH : False" << "\n";
	}

	void Renderer::ToggleFireMesh()
	{

		m_pFireMesh->SetActive(!m_pFireMesh->IsActive());

		if (m_pFireMesh->IsActive())
			std::cout << "FIRE MESH : Enabled" << "\n";
		else
			std::cout << "FIRE MESH : Disabled" << "\n";
	}

	void Renderer::NextSampleStateFilter()
	{
		m_pHardwareRasterizer->NextSampleStateFilter(m_pMeshes);
	}

	void Renderer::NextShadingMode()
	{
		m_pSoftwareRasterizer->NextShadingMode();
	}

	void Renderer::ToggleRenderMode()
	{
		m_pSoftwareRasterizer->ToggleRenderMode();
	}

	void Renderer::ToggleNormalMap()
	{
		m_pSoftwareRasterizer->ToggleNormalMap();
	}

	void Renderer::ToggleBoundingBox()
	{
		m_pSoftwareRasterizer->ToggleBoundingBox();
	}

	void Renderer::ToggleCullMode()
	{
		m_CullMode = static_cast<Mesh::CullMode>((static_cast<int>(m_CullMode) + 1) % (static_cast<int>(Mesh::CullMode::COUNT)));

		switch (m_CullMode)
		{
		case dae::Mesh::CullMode::Back:
			std::cout << "CULLING MODE : Back" << "\n";
			break;
		case dae::Mesh::CullMode::Front:
			std::cout << "CULLING MODE : Front" << "\n";
			break;
		case dae::Mesh::CullMode::None:
			std::cout << "CULLING MODE : None" << "\n";
			break;
		}

		for (size_t i = 0; i < m_pMeshes.size(); i++)
		{
			m_pMeshes[i]->SetCullMode(m_CullMode);
			m_pHardwareRasterizer->NextCullingMode(m_pMeshes, m_CullMode);
		}
	}

	void Renderer::ToggleUniformBackground()
	{
		m_IsBackgroundUniform = !m_IsBackgroundUniform;

		if (m_IsBackgroundUniform)
			std::cout << "BACKGROUND UNIFORM : True" << "\n";
		else
			std::cout << "BACKGROUND UNIFORM : False" << "\n";
	}

	void Renderer::ToggleColorShadingMode()
	{
		m_pSoftwareRasterizer->NextColorShadingMode();
	}

	void Renderer::AdjustGammaCorrection(bool lowerIt)
	{
		m_pSoftwareRasterizer->AdjustGammaCorrection(lowerIt);
	}

}
