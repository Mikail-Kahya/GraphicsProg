#pragma once

#include <cstdint>

#include "DataTypes.h"

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	class Scene;

	enum class ViewMethod
	{

	};

	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow);
		~Renderer() = default;

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Update();
		void Render(Scene* pScene) const;
		bool SaveBufferToImage() const;

	private:
		SDL_Window* m_pWindow{};

		SDL_Surface* m_pBuffer{};
		uint32_t* m_pBufferPixels{};

		int m_Width{};
		int m_Height{};
		bool m_ShowShadows{ true };
		bool m_ButtonPressed{ false };
	};
}
