#pragma once
#include "Constants.h"
#include "Miscellaneous.h"
#include "Sprite.h"
#include "Shapes.h"
#include "SDL2/SDL.h"
#include <string>

class Camera;

namespace text {
	void renderAbsolute(SDL_Point corner, const std::string& font, const std::string& text);
	void renderRelative(Point corner, const Camera& camera, const std::string& font, const std::string& text);

	void addFont(const std::string& name, Surface surface, const std::string& mapping, int maxWidth);
}
