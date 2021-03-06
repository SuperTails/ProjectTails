
/*
	This version of the prhsGameLib.cpp file was last updated on January 4th, 2017.

	Version 1.1.0

	DO NOT EDIT THIS FILE
*/

#include "stdafx.h"
#include "prhsGameLib.h"

SDLInit sdlStatus;

SDLInit::SDL::SDL() {
	if (const int flags = SDL_INIT_EVERYTHING; SDL_Init(flags) < 0) { 
		std::cout << "Could not initialize SDL." << std::endl << SDL_GetError() << std::endl; 
		success = false;
	}
}
SDLInit::SDL::~SDL() {
	if (success) {
		SDL_Quit();
	}
}
SDLInit::Mixer::Mixer() {
	//TODO: Fix
	if (const int flags = /*MIX_INIT_MP3*/0; (Mix_Init(flags) & flags) != flags) {
		std::cout << "Could not initialize SDL_mixer." << std::endl << Mix_GetError() << std::endl;
		success = false;
		return;
	}

	if (Mix_OpenAudio(PRHS_SoundManager::sampleRate, MIX_DEFAULT_FORMAT, 2, PRHS_SoundManager::sampleSize) == -1) {
		std::cout << "Could not open audio." << std::endl << Mix_GetError() << std::endl;
		success = false;
		return;
	}
}
SDLInit::Mixer::~Mixer() {
	if (success) {
		Mix_Quit();
	}
}
SDLInit::Image::Image() {
	if (const int flags = IMG_INIT_JPG | IMG_INIT_PNG; (IMG_Init(flags) & flags) != flags) {
		std::cout << "Could not initialize SDL_image." << std::endl << IMG_GetError() << std::endl;
		success = false;
		return;
	}
}
SDLInit::Image::~Image() {
	if (success) {
		IMG_Quit();
	}
}


bool initStatus = []() {
	if (const int flags = SDL_INIT_EVERYTHING; SDL_Init(flags) < 0) { 
		std::cout << "Could not initialize SDL." << std::endl << SDL_GetError() << std::endl; 
		return false; 
	}

	if (const int flags = IMG_INIT_JPG | IMG_INIT_PNG; (IMG_Init(flags) & flags) != flags) {
		std::cout << "Could not initialize SDL_image." << std::endl << IMG_GetError() << std::endl;
		return false;
	}

	if (const int flags = MIX_INIT_MP3; (Mix_Init(flags) & flags) != flags) {
		std::cout << "Could not initialize SDL_mixer." << std::endl << Mix_GetError() << std::endl;
		return false;
	}

	if (Mix_OpenAudio(PRHS_SoundManager::sampleRate, MIX_DEFAULT_FORMAT, 1, PRHS_SoundManager::sampleSize) == -1) {
		std::cout << "Could not open audio." << std::endl << Mix_GetError() << std::endl;
		return false;
	}

	return true;
}();

SDL_Window* PRHS_Window::window = NULL;
SDL_Renderer* PRHS_Window::renderer = NULL;
PRHS_Window::PRHS_Window(const char *title, int width, int height, bool fullscreen) {
	//set window parameters
	this->windowWidth = width;
	this->windowHeight = height;
	this->fullscreen = fullscreen;

	//get display mode
	SDL_DisplayMode currentDisplayMode;
	SDL_GetCurrentDisplayMode(0, &currentDisplayMode);

	if (fullscreen) { //enter fullscreen mode
		scaleFactorX = currentDisplayMode.w / (float)windowWidth; //calculate x scaling
		scaleFactorY = currentDisplayMode.h / (float)windowHeight; //calculate y scaling
		window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, currentDisplayMode.w, currentDisplayMode.h, SDL_WINDOW_FULLSCREEN);
	}
	else { //enter windowed mode
		scaleFactorX = 1;
		scaleFactorY = 1;
		window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowWidth, windowHeight, SDL_WINDOW_SHOWN);
	}

	if (window == NULL) { //Check for errors creating window
		std::cout << "Could not create window" << std::endl << SDL_GetError() << std::endl; //print error message
		return; //exit
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED); //create renderer
	if (renderer == NULL) { //Check for errors creating renderer
		std::cout << "Could not create renderer" << std::endl << SDL_GetError() << std::endl; //print error message
		return; //exit
	}

	//initialize background texture and scaling factors
	backgroundSet = false;
	background = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC, 1, 1);
	Uint32* pixel = new Uint32[windowWidth * windowHeight];
	memset(pixel, 0, windowWidth * windowHeight * sizeof(Uint32));
	SDL_UpdateTexture(background, NULL, pixel, windowWidth * sizeof(Uint32));
	delete [] pixel;
	backgroundScaleFactorX = 1;
	backgroundScaleFactorY = 1;

	//clear window and render
	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, background, NULL, NULL);
	SDL_RenderPresent(renderer);
}

PRHS_Window::~PRHS_Window() {
	if (background) { //if background is not a NULL pointer
		SDL_DestroyTexture(background); //free memory
	}
	if (renderer) { //if renderer is not a NULL pointer
		SDL_DestroyRenderer(renderer); //free memory
	}
	if (window) { //if window is not a NULL pointer
		SDL_DestroyWindow(window); //free memory
	}
}

SDL_Window* PRHS_Window::getWindow() {
	return window;
}

void PRHS_Window::render(SDL_Texture* texture, PRHS_Rect destRect) {
	SDL_Rect conversionRect = destRect;

	//handle scaling the image correctly
	conversionRect.x *= scaleFactorX;
	conversionRect.y *= scaleFactorY;
	conversionRect.w *= scaleFactorX;
	conversionRect.h *= scaleFactorY;

	//render the image to the screen
	SDL_RenderCopyEx(renderer, texture, NULL, &conversionRect, destRect.r, NULL, SDL_FLIP_NONE);
}

/*void PRHS_Window::render(PRHS_Entity* entity) {
	render(entity->getSkin(), entity->getPosition());
}*/

void PRHS_Window::updateDisplay() {
	//update screen with all previous renders
	SDL_RenderPresent(renderer);
}

/*void PRHS_Window::refreshBackground() {
	//redraw entire background image
	SDL_RenderCopy(renderer, background, NULL, NULL);
}

void PRHS_Window::refreshBackground(PRHS_Rect destRect) {

	if (destRect.r % 180 != 0) {
		const float phi = atan(destRect.w / (float)destRect.h);
		const float theta = (destRect.r * halfPI / 90);
		const float radius = sqrt(pow(destRect.w / 2.0, 2) + pow(destRect.h / 2.0, 2));
		const float x1 = radius * cos(halfPI - theta - phi);
		const float y1 = radius * sin(halfPI - theta - phi);
		const float x2 = radius * cos(halfPI - theta + phi);
		const float y2 = radius * sin(halfPI - theta + phi);

		int deltaX;
		int deltaY;
		if (abs(x1) > abs(x2)) {
			deltaX = (abs(x1) - (destRect.w / 2.0)) + 2;
			deltaY = (abs(y2) - (destRect.h / 2.0)) + 2;
		}
		else {
			deltaX = (abs(x2) - (destRect.w / 2.0)) + 2;
			deltaY = (abs(y1) - (destRect.h / 2.0)) + 2;
		}

		destRect.x = (destRect.x - deltaX) * scaleFactorX;
		destRect.y = (destRect.y - deltaY) * scaleFactorY;
		destRect.w = (destRect.w + (deltaX * 2)) * scaleFactorX;
		destRect.h = (destRect.h + (deltaY * 2)) * scaleFactorY;
	}

	//handle scaling
	SDL_Rect srcRect;
	if (backgroundSet) {
		srcRect.x = ceil(destRect.x / (float)backgroundScaleFactorX);
		srcRect.y = ceil(destRect.y / (float)backgroundScaleFactorY);
		srcRect.w = ceil(destRect.w / (float)backgroundScaleFactorX);
		srcRect.h = ceil(destRect.h / (float)backgroundScaleFactorY);
	}
	else {
		srcRect = {0,0,1,1};
	}

	//redraw part of the background image
	SDL_Rect destSdlRect = convertRect(destRect);
	SDL_RenderCopy(renderer, background, &srcRect, &destSdlRect);
}

void PRHS_Window::refreshBackground(PRHS_Entity* entity) {
	//refresh the background around an entity
	refreshBackground(entity->getPosition());
}*/

void PRHS_Window::setBackground(std::string pathToImage) {
	backgroundSet = true;
	background = SDL_CreateTextureFromSurface(renderer, IMG_Load(pathToImage.c_str())); //update background field

	int width;
	int height;
	SDL_QueryTexture(background, NULL, NULL, &width, &height); //get image width and height
							
	SDL_DisplayMode currentDisplayMode; //get current display mode
	SDL_GetCurrentDisplayMode(0, &currentDisplayMode);

	if (fullscreen) { //if in fullscreen mode
		backgroundScaleFactorX = currentDisplayMode.w / (float)width;
		backgroundScaleFactorY = currentDisplayMode.h / (float)height;
	}
	else { //if in windowed mode
		backgroundScaleFactorX = windowWidth / (float) width;
		backgroundScaleFactorY = windowHeight / (float) height;
	}

	//draw background image
	SDL_RenderCopy(renderer, background, NULL, NULL);
	SDL_RenderPresent(renderer);
}

int PRHS_Window::getHeight() {
	return windowHeight;
}

int PRHS_Window::getWidth() {
	return windowWidth;
}

SDL_Renderer* PRHS_Window::getRenderer() {
	return renderer;
}

PRHS_Entity::PRHS_Entity() noexcept : position{ 0, 0 }, previousPosition{ 0, 0 } {}

PRHS_Entity::PRHS_Entity(SDL_Point pos) noexcept : position{ pos }, previousPosition{ pos } {}

void swap(PRHS_Entity& lhs, PRHS_Entity& rhs) noexcept {
	using std::swap;

	swap(lhs.position, rhs.position);
	swap(lhs.previousPosition, rhs.previousPosition);
}

std::vector<PRHS_Effect> PRHS_SoundManager::globalEffects;
PRHS_SoundManager::PRHS_SoundManager() {

	effects.reserve(10);
}

PRHS_SoundManager::PRHS_SoundManager(std::string mainTrack) {
	PRHS_SoundManager();
	this->mainTrack = Mix_LoadMUS(mainTrack.c_str());
}

PRHS_SoundManager::~PRHS_SoundManager() {
	for (unsigned int i = 0; i < effects.size(); i++) {
		Mix_FreeChunk(effects.at(i).soundEffect);
	}

	if (mainTrack != NULL) {
		Mix_FreeMusic(mainTrack);
	}
}

void PRHS_SoundManager::cleanUp() {
	Mix_CloseAudio();
	Mix_Quit();
}

int PRHS_SoundManager::addEffect(std::string input) {
	PRHS_Effect temp;
	temp.soundEffect = Mix_LoadWAV(input.c_str());
	temp.channel = 0;
	temp.volume = 100;

	if (temp.soundEffect == NULL) {
		std::cout << "Error loading sound file: " << input << std::endl << Mix_GetError() << std::endl;
	}
	else {
		effects.push_back(temp);
	}

	return effects.size() - 1;
}

void PRHS_SoundManager::setEffectVolume(int index, int volume) {
	if (index >= effects.size()) {
		std::cout << "Index out of range." << std::endl;
		return;
	}
	else if (volume > 100) {
		effects.at(index).volume = MIX_MAX_VOLUME;
	}
	else if (volume < 0) {
		effects.at(index).volume = 0;
	}
	else {
		effects.at(index).volume = (int)(volume * (MIX_MAX_VOLUME / 100));
	}

	Mix_Volume(effects.at(index).channel, effects.at(index).volume);
}

void PRHS_SoundManager::playEffect(int index) {
	if ((unsigned int)index >= effects.size()) {
		std::cout << "Index out of range." << std::endl;
	}
	else {
		effects.at(index).channel = Mix_PlayChannel(-1, effects.at(index).soundEffect, 0);
		Mix_Volume(effects.at(index).channel, effects.at(index).volume);
	}
}

int PRHS_SoundManager::addGlobalEffect(std::string input) {
	PRHS_Effect temp;
	temp.soundEffect = Mix_LoadWAV(input.c_str());
	temp.channel = 0;
	temp.volume = 100;

	if (temp.soundEffect == NULL) {
		std::cout << "Error loading sound file: " << input << std::endl << Mix_GetError() << std::endl;
	}
	else {
		globalEffects.push_back(temp);
	}

	return globalEffects.size() - 1;
}

void PRHS_SoundManager::setGlobalEffectVolume(int index, int volume) {
	if ((unsigned int)index >= globalEffects.size()) {
		std::cout << "Index out of range." << std::endl;
		return;
	}
	else if (volume > 100) {
		globalEffects.at(index).volume = MIX_MAX_VOLUME;
	}
	else if (volume < 0) {
		globalEffects.at(index).volume = 0;
	}
	else {
		globalEffects.at(index).volume = (int)(volume * (MIX_MAX_VOLUME / 100));
	}

	Mix_Volume(globalEffects.at(index).channel, globalEffects.at(index).volume);
}

void PRHS_SoundManager::playGlobalEffect(int index) {
	if (index >= globalEffects.size()) {
		std::cout << "Index out of range." << std::endl;
	}
	else {
		globalEffects.at(index).channel = Mix_PlayChannel(-1, globalEffects.at(index).soundEffect, 1);
		Mix_Volume(globalEffects.at(index).channel, globalEffects.at(index).volume);
	}
}

void PRHS_SoundManager::setMainTrack(std::string path) {
	mainTrack = Mix_LoadMUS(path.c_str());
}

void PRHS_SoundManager::setMainTrackVolume(int volume) {
	if (volume > 100) {
		Mix_VolumeMusic(MIX_MAX_VOLUME);
	}
	else if (volume < 0) {
		Mix_VolumeMusic(0);
	}
	else {
		Mix_VolumeMusic(volume * (MIX_MAX_VOLUME / 100));
	}
}

void PRHS_SoundManager::playMainTrack(int times) {
	Mix_PlayMusic(mainTrack, times);
}

void PRHS_SoundManager::pauseMainTrack() {
	Mix_PauseMusic();
}

void PRHS_SoundManager::resumeMainTrack() {
	Mix_ResumeMusic();
}

void PRHS_SoundManager::stopMainTrack() {
	Mix_HaltMusic();
}


PRHS_Controls PRHS_GetControlState() {
	SDL_Event event;
	while (SDL_PollEvent(&event) != 0) {
	}
	const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);

	PRHS_Controls output;
	output.UP = currentKeyStates[SDL_GetScancodeFromKey(SDLK_UP)];
	output.DOWN = currentKeyStates[SDL_GetScancodeFromKey(SDLK_DOWN)];
	output.LEFT = currentKeyStates[SDL_GetScancodeFromKey(SDLK_LEFT)];
	output.RIGHT = currentKeyStates[SDL_GetScancodeFromKey(SDLK_RIGHT)];
	output.BUTTON_1 = currentKeyStates[SDL_GetScancodeFromKey(SDLK_SPACE)];
	output.BUTTON_2 = currentKeyStates[SDL_GetScancodeFromKey(SDLK_RETURN)];
	output.BUTTON_ESC = currentKeyStates[SDL_GetScancodeFromKey(SDLK_ESCAPE)];

	return output;
}

PRHS_Rect convertRect(SDL_Rect inputRect) {
	return{ inputRect.x, inputRect.y, inputRect.w, inputRect.h, 0 };
}

SDL_Rect convertRect(PRHS_Rect inputRect) {
	return {inputRect.x, inputRect.y, inputRect.w, inputRect.h};
}

SDL_Texture* PRHS_CreateTexture(std::string pathToImage, SDL_Renderer* renderer) {
	SDL_Texture* output = SDL_CreateTextureFromSurface(renderer, IMG_Load(pathToImage.c_str()));

	if (output == NULL) {
		std::cout << "Could not load image at " << pathToImage << std::endl << IMG_GetError() << std::endl;
		return NULL;
	}
	else {
		return output;
	}
}

void PRHS_Quit() {
	IMG_Quit();
	SDL_Quit();
}
