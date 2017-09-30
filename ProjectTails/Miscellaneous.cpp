#include "stdafx.h"
#include "Functions.h"
#include "Animation.h"
#include "Miscellaneous.h"
#include "Text.h"
#include "Constants.h"
#include "InputComponent.h"
#include <time.h>
#include <SDL.h>
#include <algorithm>

InputComponent globalObjects::input;
SDL_Window* globalObjects::window;
double globalObjects::ratio;
SDL_Renderer* globalObjects::renderer;
std::vector < globalObjects::loadData > globalObjects::loadProgress(0);
std::vector < Animation > globalObjects::titleScreen(0);
int globalObjects::gameState(0);
int globalObjects::titleScreenOffset(0);
std::uint32_t globalObjects::titleScreenHoverBegin(0);
double globalObjects::titleScreenFlash(1.0);

void globalObjects::renderBackground(std::vector < std::vector < Animation > >& background, const int& cameraCenterX, const double& ratio) {
	double parallax = 1.05;

	const int tileWidth = 256;
	const int layerWidth = background[0].size() * tileWidth;

	for (int layer = 0; layer < 8; layer++) {
		int layerParallax = cameraCenterX * ((parallax - 1) * std::pow(layer, 1.5) / 4.0);

		layerParallax %= 2 * layerWidth;

		for (int tile = 0; tile < background[layer].size(); tile++) {
			int newPos = tileWidth * tile - layerParallax;

			auto& currentTile = background[layer][tile];
			currentTile.Update();
			
			while (newPos < -tileWidth) {
				newPos += layerWidth;
			}

			if (newPos < 0) {
				SDL_Rect current{ newPos, static_cast<int>(WINDOW_VERTICAL_SIZE * ratio - tileWidth), tileWidth, tileWidth };
				currentTile.Render(getXY(current), 0, NULL, 1.0 / ratio);

				newPos += layerWidth;
			}

			if (newPos < WINDOW_HORIZONTAL_SIZE * ratio) {
				SDL_Rect current{ newPos, static_cast<int>(WINDOW_VERTICAL_SIZE * ratio - tileWidth), tileWidth, tileWidth };
				currentTile.Render(getXY(current), 0, NULL, 1.0 / ratio, SDL_FLIP_NONE);
			}
		}
	}
}

void globalObjects::renderTitleScreen(std::vector < std::vector < Animation > >& background, const int& centerX) {
	if (titleScreen.size() == 0) {
		using namespace std::chrono_literals;
		AnimStruct current{ ASSET"Sky.png", -1ms, 1 };
		titleScreen.emplace_back(current); // Sky
		current = { ASSET"TitleScreen/Circle.png", -1ms, 1 };
		titleScreen.emplace_back(current); //Circle
		current = { ASSET"TitleScreen/Tails.png", 80ms, 11 };
		titleScreen.emplace_back(current); //Tails
		current = { ASSET"TitleScreen/Banner.png", -1ms, 1 };
		titleScreen.emplace_back(current); // Banner
		current = { ASSET"TitleScreen/Text.png", -1ms, 1 };
		titleScreen.emplace_back(current); //Text
		titleScreenHoverBegin = SDL_GetTicks();
	}

	titleScreenOffset = -5.0 * sin((SDL_GetTicks() - titleScreenHoverBegin) / 500.0);

	SDL_Rect current{ 0, 0, static_cast<int>(WINDOW_VERTICAL_SIZE * ratio), static_cast<int>(WINDOW_HORIZONTAL_SIZE * ratio) };
	for (int i = 0; i < titleScreen.size(); i++) {
		AnimationEffectList effects = animation_effects::NO_EFFECT;
		if(i == 1)
			renderBackground(background, centerX, ratio);
		if (i == 2) {
			if (titleScreen[i].getFrame() != 10) {
				titleScreen[i].Update();
				titleScreenHoverBegin = SDL_GetTicks();
			}
			current = { int(80 + WINDOW_HORIZONTAL_SIZE * ratio / 2 - 128), int((WINDOW_VERTICAL_SIZE * ratio / 2) - 61) + titleScreenOffset, 103, 64 };
		}
		if (i == 4) {
			auto effect = animation_effects::PaletteSwap {};
			auto color = [](auto r, auto g, auto b) {
				return SDL_MapRGBA(&imageFormat, r, g, b, SDL_ALPHA_OPAQUE);
			};
			effect.oldColors = { color(0x4a,0x49,0xef), color(0x6b,0x6d,0xef), color(0x94,0x92,0xf7), color(0xb5,0xb6,0xf7), color(0xff,0xff,0xff), color(0x6b,0x24,0x08), color(0xb5,0x6d,0x10), color(0xff,0xb6,0x18) };
			double thisCurve = 0.1 * (1 + sin((SDL_GetTicks() - titleScreenHoverBegin) / 250.0));
			effect.newColors.resize(8);
			std::transform(effect.oldColors.begin(), effect.oldColors.end(), effect.newColors.begin(), [&](auto& a) {
				return lerp(a, 0xFFFFFFFF, thisCurve);
			});
			effects.push_back(std::move(effect));
		}
		titleScreen[i].Render(getXY(current), 0, NULL, 1.0 / ratio, SDL_FLIP_NONE, effects);
		current = { int(WINDOW_HORIZONTAL_SIZE * ratio / 2 - 128), int(WINDOW_VERTICAL_SIZE * ratio / 2) - 72 + titleScreenOffset, 256, 144 };
	}
}

void globalObjects::renderTitleFlash() {
	if (titleScreen[2].getFrame() != 10 || titleScreenFlash == 0.0)
		return;
	SDL_Surface* surface = SDL_CreateRGBSurface(0, WINDOW_HORIZONTAL_SIZE, WINDOW_VERTICAL_SIZE, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
	SDL_Rect current{ 0, 0, surface->w, surface->h };
	std::uint32_t t = SDL_GetTicks();
	titleScreenFlash = lerp(1.0, 0.0, std::min((t - titleScreenHoverBegin) / 1000.0, 1.0));
	SDL_FillRect(surface, &current, SDL_MapRGBA(surface->format, 0xFF, 0xFF, 0xFF, SDL_ALPHA_OPAQUE * titleScreenFlash));
	SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_BLEND);
	SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_RenderCopy(renderer, tex, NULL, NULL);
	SDL_DestroyTexture(tex);
	SDL_FreeSurface(surface);
}

void globalObjects::unloadTitleScreen() {
	titleScreen.clear();
}

void globalObjects::updateLoading(const double& incr) {
	int currentHeight = WINDOW_VERTICAL_SIZE / 2;
	int rowHeight = 40;
	loadProgress.back().progress += incr;
	std::string assetFolder(ASSET);
	std::string fontPath(ASSET"FontGUI.png");
	Text t(fontPath);
	SDL_Rect currentBar{ WINDOW_HORIZONTAL_SIZE / 4, currentHeight + 15, WINDOW_HORIZONTAL_SIZE / 2, 10 };
	for (int i = 0; i < loadProgress.size(); i++) {
		t.StringToText(loadProgress[i].label);
		currentBar.w = WINDOW_HORIZONTAL_SIZE / 2;
		SDL_RenderDrawRect(renderer, &currentBar);
		currentBar.w = loadProgress[i].progress * WINDOW_HORIZONTAL_SIZE / 2;
		SDL_Point textPosition { WINDOW_HORIZONTAL_SIZE / 2 - t.getText()->w / 2 };
		t.Render(textPosition);
		SDL_RenderFillRect(renderer, &currentBar);
		currentHeight += rowHeight;
		currentBar.y += rowHeight;
	}
	SDL_RenderPresent(renderer);
}

double globalObjects::lerp(const double& x, const double& y, const double& t) {
	return (1.0 - t) * x + t * y;
}

std::uint32_t globalObjects::lerp(const std::uint32_t& x, const std::uint32_t& y, const double& t) {
	Uint8 redX = x >> imageFormat.Rshift;
	Uint8 grnX = x >> imageFormat.Gshift;
	Uint8 bluX = x >> imageFormat.Bshift;
	Uint8 alpX = x >> imageFormat.Ashift;

	Uint8 redY = y >> imageFormat.Rshift;
	Uint8 grnY = y >> imageFormat.Gshift;
	Uint8 bluY = y >> imageFormat.Bshift;
	Uint8 alpY = y >> imageFormat.Ashift;

	std::uint32_t redZ = lerp(double(redX), double(redY), t);
	std::uint32_t grnZ = lerp(double(grnX), double(grnY), t);
	std::uint32_t bluZ = lerp(double(bluX), double(bluY), t);
	std::uint32_t alpZ = lerp(double(alpX), double(alpY), t);

	return (redZ << imageFormat.Rshift) | (grnZ << imageFormat.Gshift) | (bluZ << imageFormat.Bshift) | (alpZ << imageFormat.Ashift);
}
