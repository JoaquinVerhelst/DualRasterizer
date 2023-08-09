#include "pch.h"
#include "SoftwareRasterizer.h"
#include "Mesh.h"
#include "Texture.h"
#include "Utils.h"

#include <ppl.h>

#define PARALLEL


dae::SoftwareRasterizer::SoftwareRasterizer(SDL_Window* pWindow)
	: m_pWindow(pWindow)
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);


	//Create Buffers
	m_pFrontBuffer = SDL_GetWindowSurface(pWindow);
	m_pBackBuffer = SDL_CreateRGBSurface(0, m_Width, m_Height, 32, 0, 0, 0, 0);
	m_pBackBufferPixels = (uint32_t*)m_pBackBuffer->pixels;


	const int nrPixels{ m_Width * m_Height };
	m_pDepthBufferPixels = new float[nrPixels];
	std::fill_n(m_pDepthBufferPixels, nrPixels, FLT_MAX);

}

dae::SoftwareRasterizer::~SoftwareRasterizer()
{
	delete[] m_pDepthBufferPixels;
}

void dae::SoftwareRasterizer::Update(Timer* pTimer)
{
}



void dae::SoftwareRasterizer::SoftwareRender(std::vector<Mesh*>& pMeshes, Camera& camera, bool isBackgroundUniform) const
{
	ResetDepthBufferAndClearBackground(isBackgroundUniform);
	SDL_LockSurface(m_pBackBuffer);

	for (auto& pMesh : pMeshes)
	{
		if (pMesh->IsActive())
			Render(pMesh, camera);
	}

	SDL_UnlockSurface(m_pBackBuffer);
	SDL_BlitSurface(m_pBackBuffer, 0, m_pFrontBuffer, 0);
	SDL_UpdateWindowSurface(m_pWindow);
}


bool dae::SoftwareRasterizer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBackBuffer, "Rasterizer_ColorBuffer.bmp");
}


bool dae::SoftwareRasterizer::CheckCullMode(const Mesh* pMesh, const float edge01, const float edge02, const float edge03) const
{

	switch (pMesh->GetCullMode())
	{
	case Mesh::CullMode::Front:
		
		return (edge01 > 0 && edge02 > 0 && edge03 > 0);
		break;

	case Mesh::CullMode::Back:

		return (edge01 < 0 && edge02 < 0 && edge03 < 0);
		break;

	case Mesh::CullMode::None:

		return (edge01 > 0 && edge02 > 0 && edge03 > 0) || (edge01 < 0 && edge02 < 0 && edge03 < 0);
		break;
	}

	return false;
}

void dae::SoftwareRasterizer::VertexTransformationFunction(Mesh* pMesh, Camera& camera) const
{

	std::vector<Vertex_Out> verticesOut;
	std::vector<Vertex> verticesIn = pMesh->GetVertices();

	verticesOut.reserve(verticesIn.size());

	Matrix worldMatrix = pMesh->GetWorldMatrix();

	const Matrix worldViewProjectionMatrix{ worldMatrix * camera.GetViewMatrix() * camera.GetProjectionMatrix()};

	for (const auto& vIn : verticesIn)
	{
		
		Vertex_Out vOut{ Vector4{}, vIn.uv , vIn.normal, vIn.tangent, {} };
		
		vOut.position = worldViewProjectionMatrix.TransformPoint({ vIn.position, 1.0f });

		const float invVInPosW{ 1.f / vOut.position.w };

		vOut.position.x *= invVInPosW;
		vOut.position.y *= invVInPosW;
		vOut.position.z *= invVInPosW;

		vOut.normal = worldMatrix.TransformVector(vIn.normal);
		vOut.tangent = worldMatrix.TransformVector(vIn.tangent);
		vOut.viewDirection = (worldMatrix.TransformPoint(vIn.position) - camera.GetOrigin());

		verticesOut.emplace_back(vOut);
	}

	pMesh->SetVerticesOut(verticesOut);
}


void dae::SoftwareRasterizer::ResetDepthBufferAndClearBackground(bool isBackgroundUniform) const
{
	std::fill_n(m_pDepthBufferPixels, (m_Width * m_Height), FLT_MAX);


	Uint8 color{};

	if (isBackgroundUniform)
		color = static_cast<Uint8>(0.1f * 255);
	else
		color = static_cast<Uint8>(0.39f * 255);

	SDL_FillRect(m_pBackBuffer, nullptr, SDL_MapRGB(m_pBackBuffer->format, color, color, color));


}


bool dae::SoftwareRasterizer::IsVertexInFrustrum(const Vector4& vertex, float min, float max) const
{
	return vertex.x >= min && vertex.x <= max && vertex.y >= min && vertex.y <= max && vertex.z >= 0.f && vertex.z <= max;
}


void dae::SoftwareRasterizer::Render(Mesh* pMesh, Camera& camera) const
{

	//World Space -> NDC
	VertexTransformationFunction(pMesh, camera);


	//NDC -> Screen Space
	std::vector<Vector2> verticesScreen;
	verticesScreen.reserve(pMesh->GetVerticesOut().size());

	for (const Vertex_Out& vertexNdc : pMesh->GetVerticesOut())
	{
		verticesScreen.emplace_back(Vector2{ (vertexNdc.position.x + 1.f) / 2.f * m_Width, (1.f - vertexNdc.position.y) / 2.f * m_Height });
	};


	switch (pMesh->GetPrimitiveTopology())
	{
	case Mesh::PrimitiveTopology::TriangleStrip:

		for (size_t vertIdx{}; vertIdx < pMesh->GetIndices().size() - 2; ++vertIdx)
		{
			RenderMeshTriangle(pMesh, verticesScreen, vertIdx, vertIdx & 1);
		}

		break;

	case Mesh::PrimitiveTopology::TriangleList:

		concurrency::parallel_for(0u, pMesh->GetNumIndices() / 3,
			[&](size_t i)
			{
				RenderMeshTriangle(pMesh, verticesScreen, i * 3, false);
			});
	
		break;
	}

}


void dae::SoftwareRasterizer::RenderMeshTriangle(const Mesh* pMesh, const std::vector<Vector2>& verticesScreen, size_t currentVertexIdx, bool swapVertices) const
{
	const Vector2 screenVector{ static_cast<float>(m_Width), static_cast<float>(m_Height) };


	const size_t vertIdx0{ pMesh->GetIndices()[currentVertexIdx + (2 * swapVertices)] };
	const size_t vertIdx1{ pMesh->GetIndices()[currentVertexIdx + 1] };
	const size_t vertIdx2{ pMesh->GetIndices()[currentVertexIdx + (!swapVertices * 2)] };


	if (vertIdx0 == vertIdx1 || vertIdx1 == vertIdx2 || vertIdx2 == vertIdx0)
		return;


	if (!IsVertexInFrustrum(pMesh->GetVerticesOut()[vertIdx0].position)
		|| !IsVertexInFrustrum(pMesh->GetVerticesOut()[vertIdx1].position)
		|| !IsVertexInFrustrum(pMesh->GetVerticesOut()[vertIdx2].position))
		return;



	const Vector2& v0{ verticesScreen[vertIdx0] };
	const Vector2& v1{ verticesScreen[vertIdx1] };
	const Vector2& v2{ verticesScreen[vertIdx2] };






	const Vector2& edgeV0V1{ v1 - v0 };
	const Vector2& edgeV1V2{ v2 - v1 };
	const Vector2& edgeV2V0{ v0 - v2 };

	const float invTriangleArea{ 1.f / Vector2::Cross( edgeV0V1, edgeV2V0) };


	//Bounding Box - Optimization
	Vector2 minBoundingBox{ Vector2::Min(v0, Vector2::Min(v1, v2)) };
	Vector2 maxBoundingBox{ Vector2::Max(v0, Vector2::Max(v1, v2)) };

	minBoundingBox = Vector2::Max(Vector2::Zero, Vector2::Min(minBoundingBox, screenVector));
	maxBoundingBox = Vector2::Max(Vector2::Zero, Vector2::Min(maxBoundingBox, screenVector));


	for (int px{ static_cast<int>(minBoundingBox.x) }; px < maxBoundingBox.x; ++px)
	{
		for (int py{ static_cast<int>(minBoundingBox.y) }; py < maxBoundingBox.y; ++py)
		{

			const int pixelIdx{ px + py * m_Width };

			const Vector2 currentPixel{ static_cast<float>(px),static_cast<float>(py) };

			Vector2 pixelPos = { static_cast<float>(pixelIdx % static_cast<int>(m_Width)), static_cast<float>(pixelIdx / static_cast<int>(m_Height)) };


			if (m_IsShowingBoundingBoxes)
			{
				m_pBackBufferPixels[pixelIdx] = SDL_MapRGB(m_pBackBuffer->format,
					static_cast<uint8_t>(255),
					static_cast<uint8_t>(255),
					static_cast<uint8_t>(255));
				continue;
			}


			const float edge0{ Vector2::Cross(currentPixel - v0, edgeV0V1) };
			const float edge1{ Vector2::Cross(currentPixel - v1, edgeV1V2) };
			const float edge2{ Vector2::Cross(currentPixel - v2, edgeV2V0) };



			if (!CheckCullMode(pMesh, edge0, edge1, edge2)) continue;


			//Weight 
			float weight0, weight1, weight2;
			weight0 = edge1 * invTriangleArea;
			weight1 = edge2 * invTriangleArea;
			weight2 = edge0 * invTriangleArea;



			const float depth0{ pMesh->GetVerticesOut()[vertIdx0].position.z };
			const float depth1{ pMesh->GetVerticesOut()[vertIdx1].position.z };
			const float depth2{ pMesh->GetVerticesOut()[vertIdx2].position.z };

			const float interpolatedDepth{ 1.f / (weight0 * (1.f / depth0) + weight1 * (1.f / depth1) + weight2 * (1.f / depth2)) };



			if (m_pDepthBufferPixels[pixelIdx] <= interpolatedDepth || interpolatedDepth < 0.f || interpolatedDepth > 1.f) continue;


			if (!pMesh->IsTransparent())
				m_pDepthBufferPixels[pixelIdx] = interpolatedDepth;


			ColorRGB finalColor{};


			switch (m_RenderMode)
			{
				case RenderMode::Default:
				{

					Vertex_Out pixel{};

					const Vertex_Out& v0Out{ pMesh->GetVerticesOut()[vertIdx0] };
					const Vertex_Out& v1Out{ pMesh->GetVerticesOut()[vertIdx1] };
					const Vertex_Out& v2Out{ pMesh->GetVerticesOut()[vertIdx2] };


					const float interpolatedWDepth { 
						1.f / (weight0 * (1.f / v0Out.position.w) +
						weight1 * (1.f / v1Out.position.w) +
						weight2 * (1.f / v2Out.position.w )) };


					pixel.position = { static_cast<float>(px), static_cast<float>(py), 0.f, 0.f };

					pixel.uv = interpolatedWDepth *
						((weight0 * v0Out.uv) / v0Out.position.w +
							(weight1 * v1Out.uv) / v1Out.position.w +
							(weight2 * v2Out.uv) / v2Out.position.w );


					pixel.normal = Vector3{ interpolatedWDepth *
						(weight0 * v0Out.normal / v0Out.position.w +
						weight1 * v1Out.normal / v1Out.position.w +
						weight2 * v2Out.normal / v2Out.position.w) }.Normalized();


					pixel.tangent = Vector3{ interpolatedWDepth *
						(weight0 * v0Out.tangent / v0Out.position.w +
						weight1 * v1Out.tangent / v1Out.position.w +
						weight2 * v2Out.tangent / v2Out.position.w) }.Normalized();

					pixel.viewDirection = Vector3{ interpolatedWDepth *
						(weight0 * v0Out.viewDirection / v0Out.position.w +
						weight1 * v1Out.viewDirection / v1Out.position.w +
						weight2 * v2Out.viewDirection / v2Out.position.w) }.Normalized();

					PixelShading(pixel, pMesh, pixelIdx);
					continue;
				}
				case RenderMode::Depth:
				{

					if (pMesh->IsTransparent()) continue;


					const float depthCol{ Remap(interpolatedDepth , 0.985f,1.f) };
					finalColor = { depthCol,depthCol,depthCol };

					UpdateColorInBuffer(px, py, finalColor);
					continue;

				}


			}


		}
	}
}


void dae::SoftwareRasterizer::PixelShading(const Vertex_Out& pixel, const Mesh* pMesh, int pixelIdx) const
{
	//Color
	ColorRGB finalColor{};


	const float lightIntensity{ 7.f };
	const float kd{ 1.f };
	const float shininess{ 25.f };
	Vector3 sampledNormal{ pixel.normal };
	const ColorRGB ambient{ 0.025f, 0.025f, 0.025f };


	if (pMesh->IsTransparent())
	{

		const ColorRGB diffuseColor{ pMesh->GetDiffuseMap()->Sample(pixel.uv) };


		if (diffuseColor.a < FLT_EPSILON)
		{
			return;
		}
		else
		{
			Uint8 r{}, g{}, b{};
			SDL_GetRGB(m_pBackBufferPixels[pixelIdx], m_pBackBuffer->format, &r, &g, &b);

			constexpr float maxColorValue{ 255.0f };
			const ColorRGB prevColor{ r / maxColorValue, g / maxColorValue, b / maxColorValue };


			finalColor += prevColor * (1.0f - diffuseColor.a) + diffuseColor * diffuseColor.a;
		}


		UpdateColorInBuffer(static_cast<int>(pixel.position.x), static_cast<int>(pixel.position.y), finalColor, true);

		return;
	}




	if (m_UseNormalMaps)
	{
		const Vector3 binormal{ Vector3::Cross(pixel.normal, pixel.tangent) };
		const Matrix tangentSpaceAxis{ pixel.tangent, binormal, pixel.normal, Vector3::Zero };

		ColorRGB sampledNormalRGB{ pMesh->GetNormalMap()->Sample(pixel.uv) };
		sampledNormalRGB = 2.f * sampledNormalRGB - ColorRGB{1.f, 1.f, 1.f};


		sampledNormal = Vector3{ sampledNormalRGB.r, sampledNormalRGB.g, sampledNormalRGB.b };
		sampledNormal = tangentSpaceAxis.TransformVector(sampledNormal);
		sampledNormal.Normalize();
	}


	const float observedArea{ std::max(Vector3::Dot(sampledNormal, -m_LightDirection), 0.0f)};
	const float exp{ shininess * pMesh->GetGlossinessMap()->Sample(pixel.uv).r };
	const ColorRGB specular{ pMesh->GetSpecularMap()->Sample(pixel.uv) * BRDF::Phong(1.0f, exp, -m_LightDirection, pixel.viewDirection, sampledNormal) };
	const ColorRGB diffuse{ BRDF::Lambert(kd, pMesh->GetDiffuseMap()->Sample(pixel.uv)) * lightIntensity };


	switch (m_ShadingMode)
	{
		case ShadingMode::ObservedArea:
		{
			finalColor = ColorRGB{ observedArea, observedArea, observedArea };
			break;
		}
		case ShadingMode::Diffuse:
		{
			finalColor = diffuse * observedArea;
			break;
		}
		case ShadingMode::Specular:
		{
			finalColor = specular * observedArea;
			break;
		}
		case ShadingMode::Combined:
		{
			finalColor = (diffuse + specular) * observedArea;
			break;
		}
	}

	

	finalColor += ambient;



	UpdateColorInBuffer(static_cast<int>(pixel.position.x), static_cast<int>(pixel.position.y), finalColor);

}

void dae::SoftwareRasterizer::UpdateColorInBuffer(int px, int py, ColorRGB& color, bool isTransparent) const
{
	ColorRGB finalColor{ color };

	if (isTransparent )
		finalColor.MaxToOne();
	else
	{
		switch (m_ColorShadingMode)
		{
		case ColorShadingMode::Gamma:
			finalColor = ApplyGammaCorrection(color);
			break;
		case ColorShadingMode::MaxToOne:
			finalColor.MaxToOne();
			break;
		case ColorShadingMode::Filmic:
			finalColor = ApplyFilmicToneMapping(color);
			break;

		}
	}



	finalColor.r = std::max(0.0f, std::min(finalColor.r, 1.0f));
	finalColor.g = std::max(0.0f, std::min(finalColor.g, 1.0f));
	finalColor.b = std::max(0.0f, std::min(finalColor.b, 1.0f));


	m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
		static_cast<uint8_t>(finalColor.r * 255),
		static_cast<uint8_t>(finalColor.g * 255),
		static_cast<uint8_t>(finalColor.b * 255));
}

dae::ColorRGB dae::SoftwareRasterizer::ApplyGammaCorrection(const ColorRGB& color) const
{
	ColorRGB correctedColor;
	correctedColor.r = std::pow(color.r, m_GammaCorrection);
	correctedColor.g = std::pow(color.g, m_GammaCorrection);
	correctedColor.b = std::pow(color.b, m_GammaCorrection);
	return correctedColor;
}


// Not really finished
dae::ColorRGB dae::SoftwareRasterizer::ApplyFilmicToneMapping(const dae::ColorRGB& color) const
{


	const float A = 0.15f;
	const float B = 0.50f;
	const float C = 0.10f;
	const float D = 0.20f;
	const float E = 0.02f;
	const float F = 0.30f;


	ColorRGB mappedColor;
	mappedColor.r = ((color.r * (A * color.r + C * B) + D * E) / (color.r * (A * color.r + B) + D * F)) - E / F;
	mappedColor.g = ((color.g * (A * color.g + C * B) + D * E) / (color.g * (A * color.g + B) + D * F)) - E / F;
	mappedColor.b = ((color.b * (A * color.b + C * B) + D * E) / (color.b * (A * color.b + B) + D * F)) - E / F;

	mappedColor *= 3;

	return mappedColor;
}


void dae::SoftwareRasterizer::NextShadingMode()
{
	// Shuffle through all the lighting modes
	m_ShadingMode = static_cast<ShadingMode>((static_cast<int>(m_ShadingMode) + 1) % (static_cast<int>(ShadingMode::COUNT)));


	switch (m_ShadingMode)
	{
	case ShadingMode::Combined:
		std::cout << "SHADING MODE: Combined" << "\n";
		break;
	case ShadingMode::ObservedArea:
		std::cout << "SHADING MODE: ObservedArea" << "\n";
		break;
	case ShadingMode::Diffuse:
		std::cout << "SHADING MODE: Diffuse" << "\n";
		break;
	case ShadingMode::Specular:
		std::cout << "SHADING MODE: Specular" << "\n";
		break;
	}
}

void dae::SoftwareRasterizer::ToggleRenderMode()
{
	m_RenderMode = static_cast<RenderMode>((static_cast<int>(m_RenderMode) + 1) % (static_cast<int>(RenderMode::COUNT)));

	if (m_RenderMode == RenderMode::Default)
		std::cout << "RENDER MODE: Default" << '\n';
	else
		std::cout << "RENDER MODE: Depth" << '\n';
}

void dae::SoftwareRasterizer::ToggleNormalMap()
{
	m_UseNormalMaps = !m_UseNormalMaps;


	if (m_UseNormalMaps)
		std::cout << "NORMAL MAPS: Enabled" << '\n';
	else
		std::cout << "NORMAL MAPS: Disabled" << '\n';
}

void dae::SoftwareRasterizer::ToggleBoundingBox()
{
	m_IsShowingBoundingBoxes = !m_IsShowingBoundingBoxes;


	if (m_IsShowingBoundingBoxes)
		std::cout << "BOUNDING BOX: Enabled" << '\n';
	else
		std::cout << "BOUNDING BOX: Disabled" << '\n';
}

void dae::SoftwareRasterizer::NextColorShadingMode()
{

	m_ColorShadingMode = static_cast<ColorShadingMode>((static_cast<int>(m_ColorShadingMode) + 1) % (static_cast<int>(ColorShadingMode::COUNT)));


	switch (m_ColorShadingMode)
	{
	case ColorShadingMode::Gamma:
		std::cout << "COLOR SHADING MODE: Gamma Correction" << "\n";
		break;
	case ColorShadingMode::MaxToOne:
		std::cout << "COLOR SHADING MODE: Max To One" << "\n";
		break;
	case ColorShadingMode::Filmic:
		std::cout << "COLOR SHADING MODE: Filmic ToneMapping" << "\n";
		break;
	}


	//m_IsGammaCorrectionEnabled = !m_IsGammaCorrectionEnabled;


	//if (m_IsGammaCorrectionEnabled)
	//	std::cout << "GAMMA CORRECTION: Enabled" << '\n';
	//else
	//	std::cout << "GAMMA CORRECTION: Disabled" << '\n';

}

void dae::SoftwareRasterizer::AdjustGammaCorrection(bool lowerIt)
{
	if (lowerIt && m_GammaCorrection > 0.f)
	{
		m_GammaCorrection -= 0.2f;
		std::cout << "GAMMA CORRECTION LOWERED: " + std::to_string(m_GammaCorrection) << '\n';
	}
	else
	{
		m_GammaCorrection += 0.2f;
		std::cout << "GAMMA CORRECTION RAISED: " + std::to_string(m_GammaCorrection) << '\n';
	}
}
