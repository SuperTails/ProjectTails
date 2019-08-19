#include "stdafx.h"
#include "Text.h"
#include <iostream>

const std::vector < int > Text::widths{ /*0*/7, 4, 7, 7, 7, 7, 7, 7, 7, 7,/*10*/3, 7, 7, 7, 7, 7, 7, 7, 7, 3,/*20*/7, 8, 6, 10, 9, 7, 7, 7, 7, 7,/*30*/7, 7, 7, 10, 9, 7, 8, 3 };

void Text::Render(const SDL_Rect& position) const {
	if (!text.empty()) {
		text.render(position);
	}
}

void Text::Render(const SDL_Point& point) const {
	if (!text.empty()) {
		Render(SDL_Rect{ point.x, point.y, text.size().x, text.size().y });
	}
}

std::vector< int > Text::StringToIndex(const std::string& str) {
	std::vector< int > indexes;
	for (char c : str) {
		int v = tolower(c);
		if (v == '-') {
			indexes.push_back(24); // N, for negative
		} 
		else if (v == '.') {
			indexes.push_back(14); // D, for decimal
		}
		else if (v == ' ') {
			indexes.push_back(37); // Space
		} 
		else if (v == ':') {
			indexes.push_back(10); // Colon
		}
		else if (v == '\n') {
			indexes.push_back(widths.size());
		}
		else if (std::isdigit(v)) {
			indexes.push_back(v - '0');
		}
		else if (std::isalpha(v)) {
			indexes.push_back(v - 'a' + 11);
		}
	}
	return indexes;
};

Text::Text(const std::string& font_path) :
	font(font_path) {
	if (font == nullptr) {
		std::cerr << "Could not load font. Error:\n" << SDL_GetError() << "\n";
	}
}

void Text::setText(const std::string& text) {
	StringToText(text);
}
 
std::vector< SDL_Rect > Text::IndicesToWindows(const std::vector < int >& indices) {
	totalWidth = 0;
	totalHeight = 12;
	int y = 0;
	const int letterHeight = 11;

	std::vector < SDL_Rect > r;
	for (const auto& ind : indices) {
		if (ind == widths.size()) {
			totalHeight += letterHeight + 1;
			r.push_back({ 0, 0, 0, 0 });
			continue;
		}
		else {
			const int w = widths[ind];
			r.push_back({ ind * 10 + 9 - w, 0, w + 1, letterHeight });
			totalWidth += w + 1;
		}
	}
	return r;
}

void Text::WindowsToText(const std::vector < SDL_Rect >& win) {
	Surface temp(SDL_Point{ totalWidth, totalHeight });

	int x = 0;
	int y = 0;
	for (const auto& window : win) {
		if (window.x == 0 && window.y == 0 && window.w == 0 && window.h == 0) {
			y += 12;
			x = 0;
		}
		else {
			SDL_Rect dest = { x, y, window.w, 11 };
			SDL_BlitSurface(font.get(), &window, temp.get(), &dest);
			x += window.w;
		}
	}
	text.setSpriteSheet(temp);
}

const Sprite Text::getText() {
	return text;
}

void Text::StringToText(const std::string& str) {
	if (str == lastStr)
		return;
	else
		lastStr = str;
	WindowsToText(IndicesToWindows(StringToIndex(str)));
}
