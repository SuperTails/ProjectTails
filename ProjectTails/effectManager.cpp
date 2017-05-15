#include "stdafx.h"
#include "effectManager.h"

Animation* effectManager::rain(nullptr);
bool effectManager::fadeWhite(false);
double effectManager::fadeFrames(0.0);
double effectManager::startFrames(0.0);
bool effectManager::inFade(false);

bool effectManager::currentlyFading() {
	return inFade;
}

bool effectManager::fadeComplete() {
	return !fadeFrames;
}

void effectManager::fadeTo(bool fadeToWhite, double duration) {
	startFrames = fadeFrames = duration;
	fadeWhite = fadeToWhite;
	inFade = true;
}

void effectManager::fadeFrom(bool fadeFromWhite, double duration) {
	startFrames = fadeFrames = -duration;
	fadeWhite = fadeFromWhite;
}

void effectManager::updateFade() {
	if (inFade) {
		SDL_Surface* srf = SDL_CreateRGBSurface(0, WINDOW_HORIZONTAL_SIZE, WINDOW_VERTICAL_SIZE, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
		int transparency = SDL_ALPHA_OPAQUE*(1 - (fadeFrames / startFrames));
		if (startFrames < 0) {
			transparency = SDL_ALPHA_OPAQUE - transparency;
		}
		SDL_FillRect(srf, NULL, SDL_MapRGBA(srf->format, 255*fadeWhite,255*fadeWhite,255*fadeWhite,transparency));
		SDL_SetSurfaceBlendMode(srf, SDL_BLENDMODE_BLEND);
		SDL_Texture* tex = SDL_CreateTextureFromSurface(globalObjects::renderer, srf);
		SDL_RenderCopy(globalObjects::renderer, tex, NULL, NULL);
		SDL_DestroyTexture(tex);
		SDL_FreeSurface(srf);
		if (fadeFrames >= 0.0) {
			fadeFrames -= (globalObjects::time - globalObjects::last_time) / (1000.0 / 60.0);
			fadeFrames = std::max(fadeFrames, 0.0);
		}
		else {
			fadeFrames += (globalObjects::time - globalObjects::last_time) / (1000.0 / 60.0);
			if (fadeFrames >= 0.0) {
				inFade = false;
			}
		}
	}
}

void effectManager::loadEHZRain() {
	int pixelWidth = WINDOW_HORIZONTAL_SIZE * globalObjects::ratio;
	int pixelHeight = WINDOW_VERTICAL_SIZE * globalObjects::ratio;

	SDL_Surface* window = SDL_GetWindowSurface(globalObjects::window);

	SDL_Surface* rainFile = IMG_Load("..\\..\\asset\\Rain.png");

	SDL_SetSurfaceRLE(rainFile, SDL_TRUE);

	SDL_Surface* rainSrf = SDL_CreateRGBSurface(0, pixelWidth * 2, pixelHeight, 32, window->format->Rmask, window->format->Gmask, window->format->Bmask, window->format->Amask);

	SDL_SetColorKey(rainSrf, SDL_RLEACCEL, SDL_MapRGBA(rainSrf->format, 1, 2, 3, SDL_ALPHA_OPAQUE));

	for (int x = 0; x + 64 <= pixelWidth; x += 64) {
		for (int y = 0; y + 64 <= pixelHeight; y += 64) {
			SDL_Rect src{ 0, 0, 64, 64 };
			SDL_Rect dst{ x, y, 64, 64 };
			SDL_BlitSurface(rainFile, &src, rainSrf, &dst);
			dst.x += pixelWidth;
			src.x += 64;
			SDL_BlitSurface(rainFile, &src, rainSrf, &dst);
		}
	}

	rain = new Animation(rainSrf, 64, 2, globalObjects::window);

	SDL_SetTextureAlphaMod(rain->getTexture(), 0x80);

	SDL_FreeSurface(rainFile);
}

void effectManager::renderEHZRain() {
	SDL_Rect dst = { 0, 0, int(WINDOW_HORIZONTAL_SIZE), int(WINDOW_VERTICAL_SIZE) };
	rain->Update();
	rain->Render(&dst, 0, globalObjects::window, NULL, 1.0 / globalObjects::ratio);
}

void effectManager::unloadEHZRain() {
	delete rain;
}