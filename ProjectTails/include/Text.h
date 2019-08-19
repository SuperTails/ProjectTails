#pragma once
#include <string>
#include "SDL.h"
#include <vector>
#include "SDL_image.h"
#include "Miscellaneous.h"

class Text
{
public:
	Text(const std::string& font_path);
	~Text();

	void Render(const SDL_Rect& position);
	void Render(const SDL_Point& corner);

	void StringToText(const std::string& str);
	const SDL_Surface* getText();

private:
	std::vector < int > StringToIndex(const std::string& str);
	std::vector < SDL_Rect > IndicesToWindows(std::vector < int > ind);
	void WindowsToText(std::vector < SDL_Rect > win);

	std::string lastStr;
	SDL_Surface* font;

	SDL_Surface* text;
	SDL_Texture* textTexture;

	static const std::vector < int > widths;
	int totalWidth;
};

