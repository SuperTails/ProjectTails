#include "stdafx.h"
#include "SoundHandler.h"

Mix_Music* SoundHandler::actEnd(nullptr);
Mix_Music* SoundHandler::music(nullptr);
Mix_Music* SoundHandler::intro(nullptr);
const int SoundHandler::volume(0);
int SoundHandler::musicState(0);

void SoundHandler::init() {
	music = Mix_LoadMUS("..\\..\\asset\\TitleScreen.wav");
	actEnd = Mix_LoadMUS("..\\..\\asset\\StageClear.wav");
	Mix_PlayMusic(music, 0);
	Mix_VolumeMusic(volume);
}

void SoundHandler::setMusic(const std::string& path, const bool hasIntro) {
	Mix_PauseMusic();
	musicState = 1;
	
	if (hasIntro)
		intro = Mix_LoadMUS(std::string(path + "Intro.wav").c_str());

	music = Mix_LoadMUS(std::string(path + "Loop.wav").c_str());
	
	if (hasIntro)
		Mix_PlayMusic(intro, 0);
	else {
		Mix_PlayMusic(music, -1);
	}
}

void SoundHandler::updateMusic() {
	if (!Mix_PlayingMusic() && musicState == 1) {
		Mix_PlayMusic(music, -1);
	}
	else if (musicState == 2 && !Mix_PlayingMusic()) {
		musicState = 3;
		Mix_PlayMusic(actEnd, 0);
	}
}

void SoundHandler::actFinish() {
	musicState = 2;
	Mix_FadeOutMusic(1000);
}