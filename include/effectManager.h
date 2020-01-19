#pragma once
#include <SDL.h>
#include "Constants.h"
#include "Miscellaneous.h"
#include "Animation.h"
#include <memory>
class effectManager
{
public:
	effectManager() = delete;
	~effectManager() = delete;

	static void loadEHZRain();


	static void fadeTo(bool fadeToWhite, double duration);

	static void fadeFrom(bool fadeFromWhite, double duration);

	static void updateFade();

	//static void renderEHZRain();

	static bool currentlyFading();

	static bool fadeComplete();

	inline static void unloadEHZRain();

private:
	static std::unique_ptr<Animation> rain;
	static double fadeFrames;
	static double startFrames;
	static bool fadeWhite;
	static bool inFade;
};

