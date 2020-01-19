#include "stdafx.h"
#include "Functions.h"
#include "Animation.h"
#include "Miscellaneous.h"
#include "Text.h"
#include "Constants.h"
#include "InputComponent.h"
#include <time.h>
#include "Camera.h"
#include <SDL.h>
#include <algorithm>

InputComponent globalObjects::input;
SDL_Window* globalObjects::window;
SDL_Renderer* globalObjects::renderer;
std::vector< globalObjects::loadData > globalObjects::loadProgress(0);
std::vector< Animation > globalObjects::titleScreen(0);
int globalObjects::gameState(0);
std::uint32_t globalObjects::titleScreenHoverBegin(0);
double globalObjects::titleScreenFlash(1.0);
bool globalObjects::debug(false);

void globalObjects::renderBackground(std::vector < std::vector < Animation > >& background, const Camera& camera) {
	const double parallax = 0.05;

	const int tileWidth = 256;
	const int layerWidth = background[0].size() * tileWidth;

	const int cameraCenterX = camera.getPosition().x - camera.getOffset().x;

	for (int layer = 0; layer < 8; ++layer) {
		auto time = Timer::getTime();
		AnimationEffectList effects = (layer != 0) ? 
			AnimationEffectList{} :
			AnimationEffectList{ animation_effects::Ripple{ [&](int x) -> int { 
				if (x > 80) {
					double amp = std::max((x - 80) / 40.0, 2.0);
					double offset = time.count() / 300.0;
					return amp * sin(M_PI / 4 * (x + offset));
				}
				else {
					return 0;
				}
			}, true } };
		const int layerParallax = int(cameraCenterX * (parallax * std::pow(layer, 1.5) / 4.0)) % (2 * layerWidth);

		for (int tile = 0; tile < background[layer].size(); ++tile) {
			int newPos = tileWidth * tile - layerParallax;

			auto& currentTile = background[layer][tile];
			currentTile.Update();
			
			while (newPos < -tileWidth) {
				newPos += layerWidth;
			}

			if (newPos < 0) {
				SDL_Point current{ newPos, static_cast<int>(WINDOW_VERTICAL_SIZE / camera.scale - tileWidth) };
				currentTile.Render(current, 0, NULL, camera.scale, SDL_FLIP_NONE, effects);

				newPos += layerWidth;
			}

			if (newPos < WINDOW_HORIZONTAL_SIZE / camera.scale) {
				SDL_Point current{ newPos, static_cast<int>(WINDOW_VERTICAL_SIZE / camera.scale - tileWidth) };
				currentTile.Render(current, 0, NULL, camera.scale, SDL_FLIP_NONE, effects);
			}
		}
	}
}

void globalObjects::renderTitleScreen(std::vector < std::vector < Animation > >& background, const Camera& camera) {
	if (titleScreen.size() == 0) {
		using namespace std::chrono_literals;
		titleScreen.emplace_back(AnimStruct{ ASSET"Sky.png", -1ms, 1 }); // Sky
		titleScreen.emplace_back(AnimStruct{ ASSET"TitleScreen/Circle.png", -1ms, 1 }); //Circle
		titleScreen.emplace_back(AnimStruct{ ASSET"TitleScreen/Tails.png", 80ms, 11 }); //Tails
		titleScreen.emplace_back(AnimStruct{ ASSET"TitleScreen/Banner.png", -1ms, 1 }); // Banner
		titleScreen.emplace_back(AnimStruct{ ASSET"TitleScreen/Text.png", -1ms, 1 }); //Text
		titleScreenHoverBegin = SDL_GetTicks();
	}

	static int titleScreenOffset;

	titleScreenOffset = -5.0 * sin((SDL_GetTicks() - titleScreenHoverBegin) / 500.0);

	SDL_Point current{ 0, 0 };

	for (int i = 0; i < titleScreen.size(); ++i) {
		AnimationEffectList effects = animation_effects::NO_EFFECT;
		if(i == 1) {
			renderBackground(background, camera);
		}
		if (i == 2) {
			if (titleScreen[i].getFrame() != 10) {
				titleScreen[i].Update();
				titleScreenHoverBegin = SDL_GetTicks();
			}
			current = { int(WINDOW_HORIZONTAL_SIZE / (camera.scale * 2) - 48), int((WINDOW_VERTICAL_SIZE / (camera.scale * 2) - 61)) + titleScreenOffset };
		}
		if (i == 4) {
			auto effect = animation_effects::PaletteSwap{};
			auto color = [](auto r, auto g, auto b) {
				return SDL_MapRGBA(&imageFormat, r, g, b, SDL_ALPHA_OPAQUE);
			};
			effect.oldColors = { color(0x4a,0x49,0xef), color(0x6b,0x6d,0xef), color(0x94,0x92,0xf7), color(0xb5,0xb6,0xf7), color(0xff,0xff,0xff), color(0x6b,0x24,0x08), color(0xb5,0x6d,0x10), color(0xff,0xb6,0x18) };
			effect.newColors.reserve(effect.oldColors.size());
			std::transform(effect.oldColors.begin(), effect.oldColors.end(), std::back_inserter(effect.newColors), [](auto& a) {
				const double thisCurve = 0.1 * (1.0 + sin((SDL_GetTicks() - titleScreenHoverBegin) / 250.0));
				return lerp(a, 0xFFFFFFFF, thisCurve);
			});
			effects.push_back(std::move(effect));
		}
		titleScreen[i].Render(current, 0, NULL, camera.scale, SDL_FLIP_NONE, effects);
		current = { int(WINDOW_HORIZONTAL_SIZE / (camera.scale * 2) - 128), int(WINDOW_VERTICAL_SIZE / (camera.scale * 2)) - 72 + titleScreenOffset };
	}
}

void globalObjects::renderTitleFlash() {
	if (titleScreen[2].getFrame() != 10 || titleScreenFlash == 0.0)
		return;
	Surface surface(SDL_Point{ WINDOW_HORIZONTAL_SIZE, WINDOW_VERTICAL_SIZE });
	const SDL_Rect current{ 0, 0, surface.size().x, surface.size().y };
	const std::uint32_t t = SDL_GetTicks();
	titleScreenFlash = lerp(1.0, 0.0, std::min((t - titleScreenHoverBegin) / 1000.0, 1.0));
	SDL_FillRect(surface.get(), &current, SDL_MapRGBA(&imageFormat, 0xFF, 0xFF, 0xFF, SDL_ALPHA_OPAQUE * titleScreenFlash));
	SDL_SetSurfaceBlendMode(surface.get(), SDL_BLENDMODE_BLEND);
	Sprite sprite{ std::move(surface) };
	sprite.render({ 0, 0, WINDOW_HORIZONTAL_SIZE, WINDOW_VERTICAL_SIZE });
}

void globalObjects::unloadTitleScreen() {
	titleScreen.clear();
}

void globalObjects::updateLoading(double incr) {
	int currentHeight = WINDOW_VERTICAL_SIZE / 2;
	int rowHeight = 40;
	loadProgress.back().progress += incr;
	//Text t(ASSET"FontGUI.png");
	SDL_Rect currentBar{ WINDOW_HORIZONTAL_SIZE / 4, currentHeight + 15, WINDOW_HORIZONTAL_SIZE / 2, 10 };
	for (int i = 0; i < loadProgress.size(); i++) {
		//t.StringToText(loadProgress[i].label);
		currentBar.w = WINDOW_HORIZONTAL_SIZE / 2;
		SDL_RenderDrawRect(renderer, &currentBar);
		currentBar.w = loadProgress[i].progress * WINDOW_HORIZONTAL_SIZE / 2;
		//SDL_Point textPosition { WINDOW_HORIZONTAL_SIZE / 2 - t.getText().size().x / 2, WINDOW_VERTICAL_SIZE / 2 };
		//t.Render(textPosition);
		SDL_RenderFillRect(renderer, &currentBar);
		currentHeight += rowHeight;
		currentBar.y += rowHeight;
	}

	SDL_RenderPresent(renderer);
}

double globalObjects::lerp(double x, double y, double t) {
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
