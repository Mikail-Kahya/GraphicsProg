#pragma once

#include <cstdint>

#include "DataTypes.h"

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	class Scene;

	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow);
		~Renderer() = default;

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Render(Scene* pScene) const;
		bool SaveBufferToImage() const;

		void CycleLightMode();
		void ToggleShadows() { m_EnableShadows = !m_EnableShadows; }

	private:
		enum class LightingMode
		{
			ObservedArea,	// Lambert Cosine Law
			Radiance,		// Incident Radiance
			BRDF,			// Scattering of the light
			Combined		// ObservedArea * Radiance * BRDF
		};

		SDL_Window* m_pWindow{};

		SDL_Surface* m_pBuffer{};
		uint32_t* m_pBufferPixels{};

		int m_Width{};
		int m_Height{};
		bool m_EnableShadows{ true };
		LightingMode m_LightingMode{ LightingMode::Combined };
	};
}
