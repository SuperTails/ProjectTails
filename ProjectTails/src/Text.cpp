#include "stdafx.h"
#include "Text.h"
#include <iostream>

const std::vector < int > Text::widths{ /*0*/7, 4, 7, 7, 7, 7, 7, 7, 7, 7,/*10*/3, 7, 7, 7, 7, 7, 7, 7, 7, 3,/*20*/7, 8, 6, 10, 9, 7, 7, 7, 7, 7,/*30*/7, 7, 7, 10, 9, 7, 8, 3 };

void Text::Render(const SDL_Rect& position) {
	SDL_RenderCopy(globalObjects::renderer, textTexture, nullptr, &position);
}

void Text::Render(const SDL_Point& point) {
	if(text != nullptr) {
		Render(SDL_Rect { point.x, point.y, text->w, text->h });
	}
}

std::vector < int > Text::StringToIndex(const std::string& str) {
	std::vector < char > txt(str.begin(), str.end());
	std::vector < int > indexes;
	for (int i = 0; i < txt.size(); i++) {
		int v = tolower(txt[i]);
		if (v == '-') {
			indexes.push_back(24);
		} 
		else if (v == '.') {
			indexes.push_back(14);
		}
		else if (v == ' ') {
			indexes.push_back(v + 5);
		} 
		else if (v < ';') {
			indexes.push_back(v - 48);
		}
		else {
			indexes.push_back(v - 86);
		}
	}
	return indexes;
};

Text::Text(const std::string& font_path) :
	lastStr(""),
	text(nullptr),
	textTexture(nullptr),
	font(nullptr)
{
	font = IMG_Load(font_path.c_str());

	if (font == nullptr) {
		std::cerr << "Could not load font. Error:\n" << SDL_GetError() << "\n";
	}

	totalWidth = 0;
}
 
Text::~Text()
{
	SDL_DestroyTexture(textTexture);
	SDL_FreeSurface(text);
	SDL_FreeSurface(font);
}

std::vector < SDL_Rect > Text::IndicesToWindows(std::vector < int > ind) {
	//10 by 11
	totalWidth = 0;
	int w = 0;
	SDL_Rect s;
	std::vector < SDL_Rect > r;
	r.clear();
	for (int i = 0; i < ind.size(); i++) {
		w = widths[ind[i]];
		s.x = ind[i] * 10 + 9 - w;
		s.y = 0;
		s.w = w + 1;
		s.h = 11;

		r.push_back(s);

		totalWidth += w + 1;
	}
	return r;
}

void Text::WindowsToText(std::vector < SDL_Rect > win) {
	int x = 0;
	SDL_Rect dest;
	SDL_Surface* temp = SDL_CreateRGBSurface(0, totalWidth, 11, 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
	SDL_FreeSurface(text);
	text = SDL_ConvertSurface(temp, SDL_GetWindowSurface(globalObjects::window)->format, 0);
	SDL_FreeSurface(temp);
	SDL_SetColorKey(text, SDL_TRUE, SDL_MapRGBA(text->format, 1, 2, 3, SDL_ALPHA_OPAQUE));
	SDL_FillRect(text, NULL, SDL_MapRGBA(text->format, 1, 2, 3, SDL_ALPHA_OPAQUE));
	for (int i = 0; i < win.size(); i++) {
		dest = { x, 0, win[i].w, 11 };
		SDL_BlitSurface(font, &(win[i]), text, &dest);
		x += win[i].w;
	}
	SDL_DestroyTexture(textTexture);
	textTexture = SDL_CreateTextureFromSurface(globalObjects::renderer, text);
}

const SDL_Surface* Text::getText() {
	return text;
}

/*void Text::WindowsToText(std::vector < SDL_Rect > win, SDL_Surface* text) {
	int x = 0;
	SDL_Rect dest;
	SDL_SetColorKey(text, SDL_TRUE, SDL_MapRGBA(text->format, 1, 2, 3, SDL_ALPHA_OPAQUE));
	SDL_FillRect(text, NULL, SDL_MapRGBA(text->format, 1, 2, 3, SDL_ALPHA_OPAQUE));
	for (int i = 0; i < win.size(); i++) {
		dest = { x, 0, win[i].w, 11 };
		SDL_BlitSurface(font, &(win[i]), text, &dest);
		x += win[i].w;
	}
}*/

void Text::StringToText(const std::string& str) {
	if (str == lastStr)
		return;
	else
		lastStr = str;
	WindowsToText(IndicesToWindows(StringToIndex(str)));
}

/*void Text::StringToText(const std::string& str, SDL_Surface* surface) {
	if (str == lastStr)
		return;
	else
		lastStr = str;
	WindowsToText(IndicesToWindows(StringToIndex(str)), surface);
}*/
