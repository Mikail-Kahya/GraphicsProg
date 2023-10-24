//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Renderer.h"
#include "Math.h"
#include "Matrix.h"
#include "Material.h"
#include "Scene.h"
#include "Utils.h"

#define PARALLEL_EXECUTION
#ifdef PARALLEL_EXECUTION
#include <execution>
#endif

using namespace dae;

Renderer::Renderer(SDL_Window * pWindow) :
	m_pWindow(pWindow),
	m_pBuffer(SDL_GetWindowSurface(pWindow))
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);
	m_pBufferPixels = static_cast<uint32_t*>(m_pBuffer->pixels);
}

void Renderer::Render(Scene* pScene) const
{
	Camera& camera = pScene->GetCamera();
	const Matrix cameraToWorld{ camera.CalculateCameraToWorld() };

	const float aspectRatio{ m_Width / float(m_Height) };
	const float fov{ tanf(TO_RADIANS * camera.fovAngle / 2) };

	const auto& lightVec{ pScene->GetLights() };
	const auto& materialVec{ pScene->GetMaterials() };

	const uint32_t nrPixels{ uint32_t(m_Width * m_Height) };

#ifdef PARALLEL_EXECUTION

#else
	for (uint32_t index{}; index < nrPixels; ++index)
		RenderOnePixel(pScene, index, fov, aspectRatio, cameraToWorld, camera.origin, materialVec, lightVec);
#endif

	//@END
	//Update SDL Surface
	SDL_UpdateWindowSurface(m_pWindow);
}

void Renderer::RenderOnePixel(	Scene* pScene, uint32_t pixelIndex, float fov, float aspectRatio,
								const Matrix& cameraToWorld, const Vector3& cameraOrigin,
								const std::vector<Material*>& materialVec, const std::vector<Light>& lightVec) const
{
	const uint32_t px{ pixelIndex % m_Width};
	const uint32_t py{ pixelIndex / m_Width };

	const float x{ (2 * (px + 0.5f) / m_Width - 1) * aspectRatio * fov };
	const float y{ (1 - 2 * (py + 0.5f) / m_Height) * fov };

	Vector3 rayDirection{ x, y, 1 };
	rayDirection = cameraToWorld.TransformVector(rayDirection);
	rayDirection.Normalize();

	Ray viewRay{ cameraOrigin, rayDirection };

	ColorRGB finalColor{};

	for (int bounce{}; bounce < m_Bounces; ++bounce)
	{
		// hitinfo
		HitRecord closestHit{};
		pScene->GetClosestHit(viewRay, closestHit);

		if (closestHit.didHit)
		{
			Material* material{ materialVec[closestHit.materialIndex] };

			for (const Light& light : lightVec)
			{
				// get light to closesthit
				const Vector3 invertedLightDirection{ LightUtils::GetDirectionToLight(light, closestHit.origin) };
				const float length{ invertedLightDirection.Magnitude() - FLT_EPSILON };
				Ray invertedLightRay{ closestHit.origin + closestHit.normal * FLT_EPSILON, invertedLightDirection.Normalized(), FLT_EPSILON, length };

				// if it hits, the object is being blocked => darken
				if (pScene->DoesHit(invertedLightRay) && m_EnableShadows)
					continue;

				const float observedArea{ Vector3::Dot(invertedLightRay.direction, closestHit.normal) };
				const ColorRGB radiance{ LightUtils::GetRadiance(light, closestHit.origin) };
				const ColorRGB materialShading{ material->Shade(closestHit, invertedLightRay.direction, -viewRay.direction) };
				ColorRGB lighting{};

				if (observedArea < 0)
					continue;

				switch (m_LightingMode)
				{
				case LightingMode::ObservedArea:
					finalColor += colors::White * observedArea;
					continue;
				case LightingMode::Radiance:
					lighting = radiance;
					break;
				case LightingMode::BRDF:
					lighting = materialShading;
					break;
				case LightingMode::Combined:
					lighting = radiance * materialShading * observedArea;
					break;
				}

				finalColor += lighting;
			}
		}

		viewRay.direction = Vector3::Reflect(viewRay.direction, closestHit.normal);
		viewRay.origin = closestHit.origin;
	}

	// Update Color in Buffer
	finalColor.MaxToOne();

	m_pBufferPixels[int(px) + (int(py) * m_Width)] = SDL_MapRGB(m_pBuffer->format,
		static_cast<uint8_t>(finalColor.r * 255),
		static_cast<uint8_t>(finalColor.g * 255),
		static_cast<uint8_t>(finalColor.b * 255));
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBuffer, "RayTracing_Buffer.bmp");
}

void Renderer::CycleLightMode()
{
	int modeIndex{ int(m_LightingMode) };
	++modeIndex;
	modeIndex %= int(LightingMode::Combined) + 1;

	m_LightingMode = LightingMode(modeIndex);

	switch (m_LightingMode)
	{
	case LightingMode::ObservedArea:

		std::cout << "ObservedArea\n";
		break;
	case LightingMode::Radiance:
		std::cout << "Radiance\n";
		break;
	case LightingMode::BRDF:
		std::cout << "BRDF\n";
		break;
	case LightingMode::Combined:
		std::cout << "Combined\n";
		break;
	}
}
