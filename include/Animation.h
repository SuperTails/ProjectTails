#pragma once
#include "Sprite.h"
#include "Timer.h"
#include <vector>
#include <functional>
#include <chrono>
#include <tuple>
#include <variant>
#include <optional>

struct SDL_Window;
struct SDL_Renderer;

namespace globalObjects {
	extern SDL_Window* window;
	extern SDL_Renderer* renderer;
}

namespace animation_effects {
	struct PaletteSwap {
		std::vector < Uint32 > oldColors;
		std::vector < Uint32 > newColors;
	};
	struct Rotation {
		SDL_Point center;
		int degrees;
	};
	struct Ripple {
		std::function< int(int) > transform;
		bool vertical;
	};

	typedef std::variant < animation_effects::PaletteSwap, animation_effects::Rotation, animation_effects::Ripple > AnimationEffect;
	typedef std::vector < AnimationEffect > AnimationEffectList;

	std::pair< Surface, SDL_Point > apply(const PaletteSwap& effect, Surface srf);
	std::pair< Surface, SDL_Point > apply(const Rotation& effect, Surface& srf);
	std::pair< Surface, SDL_Point > apply(const Ripple& effect, Surface& srf);

	std::pair< Surface, SDL_Point > rotateSprite(const Rotation& effect, Surface& srf);
	std::pair< Surface, SDL_Point > rippleSprite(const Ripple& effect, Surface& srf);

	std::tuple < SDL_Point, SDL_Point, int > getPrincipalRect(SDL_Point oldOffset, SDL_Point oldSize, int angle);
	SDL_Point getLeftmostPoint(SDL_Point offset, SDL_Point size, int angle);
	SDL_Point getTopmostPoint(SDL_Point offset, SDL_Point size, int angle);
	SDL_Point getRotatedOffset(SDL_Point oldOffset, SDL_Point size, int angle);
	
	const AnimationEffectList NO_EFFECT;
}

//SpritePath, delay, frames
struct AnimStruct {
	std::string SpritePath;
	std::chrono::milliseconds delay;
	int frames;
};

using animation_effects::AnimationEffect;
using animation_effects::AnimationEffectList;


std::pair< Surface, SDL_Point > applyEffectList(Surface surface, const AnimationEffectList& effects);

SDL_Rect getFrameWindow(SDL_Point size, int frame);

Surface getSurfaceRegion(const Surface& surface, const SDL_Rect& region);

SDL_Point getFlippedOffset(SDL_Point size, SDL_Point offset, SDL_RendererFlip flip);

Surface expandSurface(const Surface& surface, SDL_Point moveTo);

void Render(const Animation& animation, const SDL_Point& position, int rotation, const SDL_Point* center, double scale, SDL_RendererFlip flip = SDL_FLIP_NONE, AnimationEffectList effectList = animation_effects::NO_EFFECT);

class Animation
{
public:
	typedef std::chrono::milliseconds DurationType;

	Animation();

	//Creates a new animation from the given sprite sheet
	//Animation(SDL_Surface* Sprites, int delay, int frames, bool tile = false);
	Animation(const Surface& s, DurationType d, int f);
	Animation(const AnimStruct& a);
	Animation(SDL_Point tileSize);
	Animation(const Animation& arg) = default;
	Animation(Animation&& other) = default;

	void SetFrame(int f);

	void Update();

	void setLooped(bool b) { hasLooped = b; };

	void Render(SDL_Point dest, int rot, const SDL_Point* center, double ratio, SDL_RendererFlip flip = SDL_FLIP_NONE, AnimationEffectList effects = animation_effects::NO_EFFECT) const;

	void setDelay(DurationType delay);

	SDL_Point GetSize() { return { getSize(sprite).x / numFrames, getSize(sprite).y }; };

	bool GetLooped() { return hasLooped; };

	void refreshTime() { timer.reset(); };

	void setOffset(std::optional< SDL_Point > newOffset);
	std::optional< SDL_Point > getOffset() const;

	int getNumFrames() const { return numFrames; };

	void stop() { timer.stop(); };

	void start() { timer.start(); };

	Surface& getSpriteSheet();

	int getFrame() const { return currentFrame; };

	Surface getSurfaceFrame(int frame);

	Surface getSurfaceFrame();

	void updateTexture();

	Texture& getTexture() { return sprite.getTexture(); };

	Uint32 getPixel(const int& x, const int& y) const;

	Animation& operator= (const Animation& rhs) = default;

	Animation& operator= (Animation&& rhs) noexcept = default;

	void addStaticEffects(const std::vector < AnimationEffectList >& effects);

	static Surface FlipSurface(const Surface& surface, SDL_RendererFlip flip);

	static void RenderExact(const Sprite& sprite, const SDL_Rect& src, const SDL_Rect& dst, SDL_RendererFlip flip, int degrees, const SDL_Point* cntr = nullptr);

	static void transform(Sprite& sprite, AnimationEffectList effects, std::optional< SDL_Point >& offset, SDL_RendererFlip& flip);

	void synchronize(const Animation& other) noexcept;

	friend void swap(Animation& lhs, Animation& rhs) noexcept;

private:
	Sprite sprite;
	
	std::optional<SDL_Point> offset;

	Timer timer;

	int currentFrame;
	int numFrames;

	bool hasLooped;
};
