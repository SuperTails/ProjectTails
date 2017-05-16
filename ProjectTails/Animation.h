#pragma once
#include <vector>
#include "SDL.h"
#include "SDL_image.h"
#include <math.h>
#include "prhsGameLib.h"
#include <iostream>
#include "Miscellaneous.h"
#include <cassert>

//SpritePath, delay, frames
struct AnimStruct {
	std::string SpritePath;
	int delay;
	int frames;
};

class Animation
{
public:
	enum effectType { NONE = 0, PALETTE_SWAP = 1, ROTATION = 2 };
	struct effectData {
		effectData() : swp() {};
		struct palSwp {
			std::vector < Uint32 > oldColors;
			std::vector < Uint32 > newColors;
			palSwp() : oldColors(), newColors() {};
		};
		struct rotate {
			rotate() : ctr(), center(&ctr), degrees(0) {};
			SDL_Point* center;
			int degrees;
		private:
			SDL_Point ctr;
		};
		palSwp swp;
		rotate rot;
	};


	Animation();

	//Creates a new animation from the given sprite sheet
	Animation(SDL_Surface* Sprites, int delay, int frames, bool tile = false);
	Animation(AnimStruct);
	Animation(SDL_Point tileSize);
	Animation(const Animation& arg);
	Animation(Animation&& other);

	void SetFrame(int f);

	void Update();

	void setLooped(bool b) { looped = b; };

	void Render(const SDL_Rect* dest, const int& rot, SDL_Point* center, const double& ratio, const SDL_RendererFlip& flip = SDL_FLIP_NONE, const effectType& effect = NONE, const effectData* efxData = nullptr);
	void SizeRender(SDL_Rect* dest, int rot, SDL_Point* center, double ratio);

	void SetDelay(int delay);

	SDL_Point GetSize() {return { SpriteSheet->w / numFrames, SpriteSheet->h }; };

	bool GetLooped() { return looped; };

	static void setTimeDifference(Uint32 dt) { deltaTime = dt; };

	void refreshTime() { timeError = 0; };

	int getNumFrames() { return numFrames; };

	SDL_Surface* GetSpriteSheet(SDL_RendererFlip flip = SDL_FLIP_NONE);

	int getFrame() { return frame; };

	SDL_Surface* getSurfaceFrame(int frame);

	inline SDL_Surface* getSurfaceFrame();

	bool updateTexture();

	void setTimeError(Uint32 error) { timeError = error; };

	SDL_Texture*& getTexture() { return tex; };

	Uint32 getPixel(const int& x, const int& y) const;

	void addStaticEffects(const std::vector < std::vector < effectType > >& types, const std::vector < std::vector < effectData > >& effects);

	static SDL_Surface* FlipSurface(SDL_Surface* surface, SDL_RendererFlip flip);
	static SDL_Surface* PaletteSwap(SDL_Surface* surface, const std::vector < Uint32 >& oldColors, const std::vector < Uint32 >& newColors);
	static SDL_Surface* Rotate(SDL_Surface* surface, const SDL_Point* cntr, int degrees, SDL_Point& offset);
	static Uint32 getPixel(const SDL_Surface* s, const int& x, const int& y);
	static void setPixel(SDL_Surface* surface, int x, int y, Uint32 pixel);
	static void setRenderer(SDL_Renderer* r);

	~Animation();
private:
	int Delay; //Time between frames
	SDL_Surface* SpriteSheet;
	std::vector <SDL_Rect> FrameWindows;
	int frame;
	int numFrames;
	bool looped;
	bool tile;
	Uint32 timeError;
	SDL_Texture* tex;
	static Uint32 deltaTime;
	static SDL_Renderer* renderer;
};

