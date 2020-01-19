#pragma once
#include <SDL2/SDL.h>
#include <cstdint>
#include "Camera.h"

namespace drawing {
	struct Color {
		std::uint8_t r, g, b;
	};

	void drawPoint(SDL_Renderer* renderer, const Camera& camera, Point pos, Color color, double size);

	void drawRect(SDL_Renderer* renderer, const Camera& camera, Rect rect, Color color, bool filled);

	void drawLine(SDL_Renderer* renderer, const Camera& camera, Point begin, Point end, Color color);
};
