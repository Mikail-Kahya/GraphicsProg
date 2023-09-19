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
	// Test sphere
	Sphere testSphere{ {0.f,0.f,100}, 50, 0};

	for (int px{ }; px < m_Width; ++px)
	{
		for (int py{ }; py < m_Height; ++py)
		{
			const float x{ (2 * (px + 0.5f) / m_Width - 1) * aspectRatio };
			const float y{ 1 - 2 * (py + 0.5f) / m_Height };

			Vector3 rayDirection{ x, y, 1};
			//rayDirection.Normalize();

			Ray viewRay{ {0,0,0}, rayDirection };

			ColorRGB finalColor{};

			// hitinfo
			HitRecord closestHit{};

			GeometryUtils::HitTest_Sphere(testSphere, viewRay, closestHit);

			if (closestHit.didHit)
			{
				const float scaledT{ (closestHit.t - 50.f) / 40.f };
				//finalColor = ColorRGB{ scaledT, scaledT, scaledT };
				finalColor = materials[closestHit.materialIndex]->Shade();
			}

			// Update Color in Buffer
			finalColor.MaxToOne();

			m_pBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBuffer->format,
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
