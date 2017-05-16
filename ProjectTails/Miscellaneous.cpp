#include "stdafx.h"
#include "Miscellaneous.h"

InputComponent globalObjects::input;
SDL_Window* globalObjects::window;
double globalObjects::ratio;
SDL_Renderer* globalObjects::renderer;
Uint32 globalObjects::time;
Uint32 globalObjects::last_time;
std::vector < globalObjects::loadData > globalObjects::loadProgress(0);
std::vector < Animation > globalObjects::titleScreen(0);
int globalObjects::gameState(0);
int globalObjects::titleScreenOffset(0);
Uint32 globalObjects::titleScreenHoverBegin(0);
double globalObjects::titleScreenFlash(1.0);
int globalObjects::lastShuffle(0);
std::vector < int > globalObjects::lastPalette{ 0, 1, 2, 3 };

void globalObjects::renderBackground(std::vector < std::vector < Animation > >& background, const SDL_Window* window, const int& cameraCenterX, const double& ratio) {
	double parallax = 1.05;
	int width = background[0].size() * 256;
	for (int layer = 0; layer < 8; layer++) {
		int layerParallax = cameraCenterX * ((parallax - 1) * std::pow(layer, 1.5) / 4.0);
		for (int tile = 0; tile < background[layer].size(); tile++) {
			int newPos = 256 * tile - layerParallax;
			background[layer][tile].Update();
			while (newPos < -256) {
				newPos += 256 * background[layer].size();
			}
			if (newPos < 0) {
				SDL_Rect current{ newPos, WINDOW_VERTICAL_SIZE * ratio - 256, 256, 256 };
				background[layer][tile].Render(&current, 0, NULL, 1.0 / ratio);
			}
			while (newPos < 0) {
				newPos += 256 * background[layer].size();
			}
			if (newPos < WINDOW_HORIZONTAL_SIZE * ratio) {
				SDL_Rect current{ newPos, WINDOW_VERTICAL_SIZE * ratio - 256, 256, 256 };
				background[layer][tile].Render(&current, 0, NULL, 1.0 / ratio, SDL_FLIP_NONE);
			}
		}
	}
}

void globalObjects::renderTitleScreen(std::vector < std::vector < Animation > >& background, const int& centerX) {
	typedef ::Animation::effectData effectData;
	typedef ::Animation::effectType effectType;

	if (titleScreen.size() == 0) {
		AnimStruct current{ "..\\..\\asset\\Sky.png", -1, 1 };
		titleScreen.emplace_back(current); // Sky
		current = { "..\\..\\asset\\TitleScreen\\TitleScreen_Circle.png", -1, 1 };
		titleScreen.emplace_back(current); //Circle
		current = { "..\\..\\asset\\TitleScreen\\TitleScreen_Tails.png", 80, 11 };
		titleScreen.emplace_back(current); //Tails
		current = { "..\\..\\asset\\TitleScreen\\TitleScreen_Banner.png", -1, 1 };
		titleScreen.emplace_back(current); // Banner
		current = { "..\\..\\asset\\TitleScreen\\TitleScreen_Text.png", -1, 1 };
		titleScreen.emplace_back(current); //Text
		titleScreenHoverBegin = SDL_GetTicks();
	}

	titleScreenOffset = -5.0 * sin((SDL_GetTicks() - titleScreenHoverBegin) / 500.0);

	effectType efxType = effectType::NONE;
	effectData efxData;

	SDL_Rect current{ 0, 0, WINDOW_VERTICAL_SIZE * ratio, WINDOW_HORIZONTAL_SIZE * ratio };
	for (int i = 0; i < titleScreen.size(); i++) {
		efxType = effectType::NONE;
		if(i == 1)
			renderBackground(background, window, centerX, ratio);
		if (i == 2) {
			if (titleScreen[i].getFrame() != 10) {
				titleScreen[i].Update();
				titleScreenHoverBegin = SDL_GetTicks();
			}
			current = { int(80 + WINDOW_HORIZONTAL_SIZE * ratio / 2 - 128), int((WINDOW_VERTICAL_SIZE * ratio / 2) - 61) + titleScreenOffset, 103, 64 };
		}
		if (i == 4) {
			efxType = effectType::PALETTE_SWAP;
			efxData.swp.oldColors = std::vector<Uint32>{ 0x004a49ef, 0x006b6def, 0x009492f7, 0x00b5b6f7, 0x00ffffff, 0x006b2408, 0x00b56d10, 0x00ffb618 };
			double thisCurve = 0.1 * (1 + sin((SDL_GetTicks() - titleScreenHoverBegin) / 250.0));
			efxData.swp.newColors.resize(8, 0);
			for (int i = 0; i < efxData.swp.newColors.size(); i++) {
				efxData.swp.newColors[i] = lerp(efxData.swp.oldColors[i], 0x00FFFFFF, thisCurve);
			}
		}
		titleScreen[i].Render(&current, 0, NULL, 1.0 / ratio, SDL_FLIP_NONE, efxType, &efxData);
		current = { int(WINDOW_HORIZONTAL_SIZE * ratio / 2 - 128), int(WINDOW_VERTICAL_SIZE * ratio / 2) - 72 + titleScreenOffset, 256, 144 };
	}
}

void globalObjects::renderTitleFlash() {
	if (titleScreen[2].getFrame() != 10 || titleScreenFlash == 0.0)
		return;
	SDL_Surface* surface = SDL_CreateRGBSurface(0, WINDOW_HORIZONTAL_SIZE, WINDOW_VERTICAL_SIZE, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
	SDL_Rect current{ 0, 0, surface->w, surface->h };
	Uint32 t = SDL_GetTicks();
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
	Text t("..\\..\\asset\\FontGUI.png");
	SDL_Rect currentBar{ WINDOW_HORIZONTAL_SIZE / 4, currentHeight + 15, WINDOW_HORIZONTAL_SIZE / 2, 10 };
	for (int i = 0; i < loadProgress.size(); i++) {
		t.StringToText(loadProgress[i].label);
		currentBar.w = WINDOW_HORIZONTAL_SIZE / 2;
		SDL_RenderDrawRect(renderer, &currentBar);
		currentBar.w = loadProgress[i].progress * WINDOW_HORIZONTAL_SIZE / 2;
		SDL_Rect currentText{ WINDOW_HORIZONTAL_SIZE / 2 - t.getText()->w / 2, currentHeight, t.getText()->w, t.getText()->h };
		SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, t.getText());
		SDL_RenderCopy(renderer, tex, NULL, &currentText);
		SDL_DestroyTexture(tex);
		SDL_RenderFillRect(renderer, &currentBar);
		currentHeight += rowHeight;
		currentBar.y += rowHeight;
	}
	SDL_RenderPresent(renderer);
}

double globalObjects::lerp(const double& x, const double& y, const double& t) {
	return (1.0 - t) * x + t * y;
}

Uint32 globalObjects::lerp(const Uint32& x, const Uint32& y, const double& t) {
	Uint8 redX = x >> 16;
	Uint8 grnX = x >> 8;
	Uint8 bluX = x >> 0;

	Uint8 redY = y >> 16;
	Uint8 grnY = y >> 8;
	Uint8 bluY = y >> 0;

	Uint32 redZ = lerp(double(redX), double(redY), t);
	Uint32 grnZ = lerp(double(grnX), double(grnY), t);
	Uint32 bluZ = lerp(double(bluX), double(bluY), t);

	return (redZ << 16) | (grnZ << 8) | (bluZ << 0);
}