#pragma once
#include "Animation.h"
#include <vector>
#include "InputComponent.h"
#include <SDL.h>
#include "Constants.h"
#include "Text.h"
#include <algorithm>
#include <time.h>

class Animation;

namespace globalObjects {
	struct loadData {
		std::string label;
		double progress;
		loadData() : label(), progress(0.0) {};
		loadData(std::string l, double p) : label(l), progress(p) {};
	};
	extern InputComponent input;
	extern SDL_Window* window;
	extern double ratio;
	extern SDL_Renderer* renderer;
	extern Uint32 time, last_time;
	extern std::vector < loadData > loadProgress;
	extern std::vector < Animation > titleScreen;
	extern int titleScreenOffset;
	extern Uint32 titleScreenHoverBegin;
	extern int gameState;
	extern double titleScreenFlash;
	extern int lastShuffle;
	extern std::vector < int > lastPalette;
	
	void renderBackground(std::vector < std::vector < Animation > >& background, const SDL_Window* window, const int& cameraCenterX, const double& ratio);
	void renderTitleScreen(std::vector < std::vector < Animation > >& background, const int& centerX);
	void renderTitleFlash();
	void unloadTitleScreen();
	void updateLoading(const double& incr);
	double lerp(const double& x, const double& y, const double& t);
	Uint32 lerp(const Uint32& x, const Uint32& y, const double& t);
};