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
	auto& materials = pScene->GetMaterials();
	auto& lights = pScene->GetLights();

	const float aspectRatio{ m_Width / float(m_Height) };
	const float FOV{ tanf(TO_RADIANS * camera.fovAngle / 2) };
	const float epsilon{ 0.001f };

	const Matrix cameraToWorld = camera.CalculateCameraToWorld();
	// Test sphere
	// Sphere testSphere{ {0.f,0.f,100}, 50, 0};

	for (float px{ 0.5f }; px < m_Width; ++px)
	{
		const float x{ (2 * px / m_Width - 1) * aspectRatio * FOV };

		for (float py{ 0.5f }; py < m_Height; ++py)
		{
			const float y{ (1 - 2 * py / m_Height) * FOV };

			Vector3 rayDirection{ x, y, 1};
			rayDirection = cameraToWorld.TransformVector(rayDirection);
			rayDirection.Normalize();

			const Ray viewRay{ camera.origin, rayDirection };

			ColorRGB finalColor{};

			// hitinfo
			bool isShaded{ false };
			HitRecord closestHit{};
			pScene->GetClosestHit(viewRay, closestHit);

			for (const Light& light : lights)
			{
				// get light to closesthit
				const Vector3 lightDirection{ closestHit.origin - light.origin };
				const float length{ lightDirection.Magnitude() - epsilon };
				Ray lightRay{light.origin, lightDirection.Normalized(), 0.0001f, length};

				// if it hits, the object is being blocked => darken
				isShaded = pScene->DoesHit(lightRay);
				if (isShaded)
					break;
			}

			//Plane testPlane{ {0.f, -50.f, 0.f}, {0.f, 1.f, 0.f}, 0 };
			//GeometryUtils::HitTest_Plane(testPlane, viewRay, closestHit);

			if (closestHit.didHit)
			{
				finalColor = materials[closestHit.materialIndex]->Shade();
				if (isShaded)
					finalColor *= 0.5f;

				//const float scaledT{ closestHit.t / 500.f };
				//finalColor = { scaledT,scaledT,scaledT };
			}

			// Update Color in Buffer
			finalColor.MaxToOne();

			m_pBufferPixels[int(px) + (int(py) * m_Width)] = SDL_MapRGB(m_pBuffer->format,
				static_cast<uint8_t>(finalColor.r * 255),
				static_cast<uint8_t>(finalColor.g * 255),
				static_cast<uint8_t>(finalColor.b * 255));
		}
	}

	//@END
	//Update SDL Surface
	SDL_UpdateWindowSurface(m_pWindow);
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBuffer, "RayTracing_Buffer.bmp");
}
