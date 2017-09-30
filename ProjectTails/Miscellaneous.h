#pragma once
#include <cstddef>
#include <string>
#include <vector>
#include <chrono>

class Animation;
class InputComponent;
class SDL_Window;
class SDL_Renderer;

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
	extern std::vector < loadData > loadProgress;
	extern std::vector < Animation > titleScreen;
	extern int titleScreenOffset;
	extern std::uint32_t titleScreenHoverBegin;
	extern int gameState;
	extern double titleScreenFlash;

	void renderBackground(std::vector < std::vector < Animation > >& background, const int& cameraCenterX, const double& ratio);
	void renderTitleScreen(std::vector < std::vector < Animation > >& background, const int& centerX);
	void renderTitleFlash();
	void unloadTitleScreen();
	void updateLoading(const double& incr);
	double lerp(const double& x, const double& y, const double& t);
	std::uint32_t lerp(const std::uint32_t& x, const std::uint32_t& y, const double& t);
};
