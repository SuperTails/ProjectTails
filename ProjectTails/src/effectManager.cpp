#include "stdafx.h"
#include "effectManager.h"
#include "Timer.h"

std::unique_ptr < Animation > effectManager::rain(nullptr);
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
			fadeFrames -= (Timer::getFrameTime().count()) / (1000.0 / 60.0);
			fadeFrames = std::max(fadeFrames, 0.0);
		}
		else {
			fadeFrames += (Timer::getFrameTime().count()) / (1000.0 / 60.0);
			if (fadeFrames >= 0.0) {
				inFade = false;
			}
		}
	}
}

void effectManager::loadEHZRain() {
	int pixelWidth = WINDOW_HORIZONTAL_SIZE * globalObjects::ratio;
	int pixelHeight = WINDOW_VERTICAL_SIZE * globalObjects::ratio;

	Surface rainFile("..\\..\\asset\\Rain.png");

	Surface rainSrf(SDL_Point{ pixelWidth * 2, pixelHeight });

	SDL_SetColorKey(rainSrf.get(), SDL_RLEACCEL, SDL_MapRGBA(rainSrf.get()->format, 1, 2, 3, SDL_ALPHA_OPAQUE));

	for (int x = 0; x + 64 <= pixelWidth; x += 64) {
		for (int y = 0; y + 64 <= pixelHeight; y += 64) {
			SDL_Rect src{ 0, 0, 64, 64 };
			SDL_Rect dst{ x, y, 64, 64 };
			SDL_BlitSurface(rainFile.get(), &src, rainSrf.get(), &dst);
			dst.x += pixelWidth;
			src.x += 64;
			SDL_BlitSurface(rainFile.get(), &src, rainSrf.get(), &dst);
		}
	}
	
	using namespace std::chrono_literals;
	rain = std::make_unique < Animation >(rainSrf, 64ms, 2);

	SDL_SetTextureAlphaMod(rain->getTexture().get(), 0x80);
}

/*void effectManager::renderEHZRain() {
	SDL_Rect dst = { 0, 0 };
	rain->Update();
	rain->Render(dst, 0, NULL, 1.0 / globalObjects::ratio);
}*/

void effectManager::unloadEHZRain() {
	delete rain.release();
}
