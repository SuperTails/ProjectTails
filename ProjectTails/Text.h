#pragma once
#include <string>
#include "SDL.h"
#include <vector>
#include "SDL_image.h"
#include "Miscellaneous.h"

class Text
{
public:
	Text(std::string font_path);
	~Text();

	std::vector < int > StringToIndex(std::string str);
	std::vector < SDL_Rect > IndicesToWindows(std::vector < int > ind);
	void WindowsToText(std::vector < SDL_Rect > win);
	void WindowsToText(std::vector < SDL_Rect > win, SDL_Surface* surface);
	void StringToText(std::string str);
	void StringToText(std::string str, SDL_Surface* surface);
	SDL_Surface*& getText();

private:
	std::string lastStr;
	SDL_Surface* font;
	SDL_Surface* text;
	static const std::vector < int > widths;
	int totalWidth;
};

