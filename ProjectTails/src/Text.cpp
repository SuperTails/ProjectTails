#include "Text.h"
#include "Camera.h"
#include "SDL2/SDL_image.h"
#include <iostream>
#include <unordered_map>

struct Font {
	struct Character{ 
		int width;
		Sprite sprite;
	};
	int maxWidth;
	int height;

	std::unordered_map< char, Character > characters;
};

Font::Character parseCharacter(Surface& file, SDL_Rect position) {
	int edgeX = position.x;
	{
		Surface::PixelLock lock{ file };
		for (edgeX = position.x; edgeX < position.x + position.w; ++edgeX) {
			bool columnPresent = false;
			for (int y = position.y; y < position.y + position.h; ++y) {
				if ((getPixel(file.get(), edgeX, y) & getFormat().Amask) == 0xFF000000) {
					columnPresent = true;
					break;
				}
			}

			if (columnPresent) {
				break;
			}
		}
	}

	SDL_Rect realPos{ edgeX, position.y, position.x + position.w - edgeX, position.h };

	Surface tempSurface(SDL_Point{ realPos.w, realPos.h });

	SDL_BlitSurface(file.get(), &realPos, tempSurface.get(), nullptr);

	std::cout << realPos.w << ", ";

	return Font::Character{ realPos.w, Sprite{ std::move(tempSurface) } };
}

Font parseFont(Surface& file, const std::string& mapping, int maxWidth) {
	const int height = file.size().y;

	if (file.size().x % maxWidth != 0) {
		std::cerr << "Image width is not a multiple of character max width\n";
		throw std::invalid_argument("Invalid image width or character max width");
	}

	const int numChars = file.size().x / maxWidth;
	if (numChars != mapping.size()) {
		std::cerr << "Number of characters in image does not match mapping\n";
		throw std::invalid_argument("Image character count does not match string\n");
	}

	Font result{ maxWidth, height };

	for (int i = 0; i < numChars; ++i) {
		result.characters.emplace(mapping[i], parseCharacter(file, SDL_Rect{ i * maxWidth, 0, maxWidth, height }));
	}
	std::cout << "\n";

	return result;
}

static std::unordered_map< std::string, Font > fonts{};

void text::addFont(const std::string& name, Surface surface, const std::string& mapping, int maxWidth) {
	if (fonts.count(name)) {
		std::cerr << "Font '" << name << "' already exists.\n";
		throw std::invalid_argument("Font '" + name + "' already exists");
	}

	{
	Surface::PixelLock lock(surface);
	std::cout << "nopixel: 0x" << std::hex << getPixel(surface.get(), 0, 0) << std::dec << "\n";
	std::cout << "opaque: " << SDL_ALPHA_OPAQUE << "\n";
	}

	fonts.emplace(name, parseFont(surface, mapping, maxWidth));
}

void text::renderRelative(Point corner, const Camera& camera, const std::string& font, const std::string& text) {
	corner -= camera.getPosition();
	corner.x *= camera.scale;
	corner.y *= camera.scale;
	
	renderAbsolute(static_cast< SDL_Point >(corner), font, text);
}

void text::renderAbsolute(SDL_Point corner, const std::string& font, const std::string& text) {
	int offsetX = 0;
	int offsetY = 0;
	for (char c : text) {
		switch (c) {
		case '\n':
			offsetX = 0;
			offsetY += 3 + fonts.at(font).height;
			break;
		case ' ':
			offsetX += fonts.at(font).maxWidth;
			break;
		default:
			c = std::toupper(c);

			const Sprite& sprite = fonts.at(font).characters.at(c).sprite;
			sprite.render(SDL_Rect{ corner.x + offsetX, corner.y + offsetY, sprite.size().x, sprite.size().y });
			offsetX += 1 + sprite.size().x;
		}
	}
}
