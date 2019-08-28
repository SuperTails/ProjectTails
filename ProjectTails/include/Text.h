#pragma once
#include "Constants.h"
#include "Miscellaneous.h"
#include "Sprite.h"
#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#include <string>
#include <vector>


class Text
{
public:
	Text() = default;
	Text(const Text&) = default;
	Text(Text&&) = default;
	Text(const std::string& path);

	void Render(const SDL_Rect& position) const;
	void Render(const SDL_Point& corner) const;

	void setText(const std::string& str);

	void StringToText(const std::string& str);
	const Sprite getText();

private:
	std::vector < int > StringToIndex(const std::string& str);
	std::vector < SDL_Rect > IndicesToWindows(const std::vector < int >& indices);
	void WindowsToText(const std::vector < SDL_Rect >& win);

	std::string lastStr;
	Surface font{ ASSET"FontGUI.png" };

	Sprite text;

	static const std::vector < int > widths;
	int totalWidth{};
	int totalHeight{};
};

