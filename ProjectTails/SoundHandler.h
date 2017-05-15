#pragma once
#include <SDL.h>
#include <SDL_mixer.h>
#include <string>

namespace SoundHandler {
	extern Mix_Music* actEnd;
	extern Mix_Music* music;
	extern Mix_Music* intro;
	extern const int volume;
	//0 = Title Screen, 1 = In act, 2 = Act music fading out, 3 = Act complete music
	extern int musicState;
	
	void init();
	void setMusic(const std::string& str, const bool hasIntro = false);
	void updateMusic();
	void actFinish();
}
