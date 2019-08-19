#include "Animation.h"
#include "Functions.h"
#include "Constants.h"
#include <SDL.h>
#include <iostream>
#include <cassert>
#include <fstream>
#include <math.h>

animation_effects::AnimationEffectList NO_EFFECT;

void Render(const Animation& animation, const SDL_Point& position, int rotation, const SDL_Point* center, double ratio, SDL_RendererFlip flip, AnimationEffectList effectList) {
	animation.Render(position, rotation, center, ratio, flip, effectList);
}

Animation::Animation() :
	numFrames(0),
	hasLooped(false),
	currentFrame(0)
{
};

/*Animation::Animation(SDL_Surface* Sprites, int delay, int frames, bool til) :
	timeError(0),
	delay(delay),
	numFrames(frames),
	currentFrame(0),
	offset{ 0, 0 },
	hasLooped(false),
	sprite(Sprites)
{
	SDL_SetSurfaceRLE(SpriteSheet, SDL_TRUE);
	SDL_SetColorKey(SpriteSheet, SDL_RLEACCEL, SDL_MapRGBA(SDL_GetWindowSurface(globalObjects::window)->format, 1, 2, 3, SDL_ALPHA_OPAQUE));
	SDL_SetSurfaceBlendMode(SpriteSheet, SDL_BLENDMODE_NONE);

	if (SpriteSheet == nullptr) {
		std::cout << SDL_GetError() << "\n";
		throw "Spritesheet could not be created";
	}
	for (int i = 0; i < frames; i++) {
		FrameWindows[i].x = i*(Sprites->w) / frames;
		FrameWindows[i].y = 0;
		FrameWindows[i].w = (Sprites->w) / frames;
		FrameWindows[i].h = (Sprites->h);
	}
	tex = SDL_CreateTextureFromSurface(globalObjects::renderer, SpriteSheet);
	if (tex == nullptr) {
		std::cout << SDL_GetError() << "\n";
		throw "Texture could not be created";
	}
}*/

Animation::Animation(const Surface& s, DurationType d, int f) :
	sprite(s),
	timer(d),
	numFrames(f),
	currentFrame(0),
	hasLooped(false) {
	start();
}

Animation::Animation(const AnimStruct& a) :
	timer(a.delay),
	numFrames(a.frames),
	currentFrame(0),
	hasLooped(false),
	sprite(a.SpritePath)
{
	std::ifstream helperFile(a.SpritePath.substr(0, a.SpritePath.size() - 4));

	if (!helperFile.fail()) {
		SDL_Point temp;
		helperFile >> temp.x >> temp.y;
		offset = temp;
	}

	start();
}

Animation::Animation(SDL_Point tileSize) :
	numFrames(1),
	currentFrame(0),
	sprite(SDL_CreateRGBSurface(0, constants::GROUND_PIXEL_WIDTH, constants::GROUND_PIXEL_WIDTH, 32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF)) {

};

void Animation::SetFrame(int f) {
	currentFrame = f;
}

void Animation::Update() {
	using namespace std::chrono_literals;
	if (timer.isTiming()) {
		if (timer.update()) {
			hasLooped |= (currentFrame == numFrames - 1);
			++currentFrame;
			currentFrame %= numFrames;
		}
	}
}

Surface animation_effects::apply(const PaletteSwap& effect, Surface srf) {
	for (int i = 0; i < effect.oldColors.size(); ++i) {
		for (int x = 0; x < srf.size().x; ++x) {
			for (int y = 0; y < srf.size().y; ++y) {
				auto srcColor = getPixel(srf.get(), x, y);
				if (srcColor == effect.oldColors[i]) {
					srcColor = effect.newColors[i];
				}
				setPixel(srf.get(), x, y, srcColor);
			}
		}
	}

	return srf;

};

std::pair < Surface, SDL_Point > animation_effects::rotateSprite(const Rotation& effect, Surface srf) {
	if (effect.degrees == 0) {
		return { srf, { 0, 0 } }; 
	} 

	typedef std::pair < double, double > FloatPoint;
	auto rotate = [&effect](std::pair < double, double >& p, bool reverse = false) {
		const double angle = effect.degrees * (reverse ? -1 : 1);
		FloatPoint newCenter = { p.first - effect.center.x, p.second - effect.center.y };
		auto result = ::rotate(newCenter, angle);
		result.first += effect.center.x;
		result.second += effect.center.y;
		return result;
	};
	
	std::vector < std::pair < double, double > > corners{ { 0, 0 }, { srf.size().x - 1, 0 }, { 0, srf.size().y - 1 }, { srf.size().x - 1, srf.size().y - 1 } };
	std::transform(corners.begin(), corners.end(), corners.begin(), rotate);

	auto xLess = [](FloatPoint a, FloatPoint b) { return a.first < b.first; };
	auto yLess = [](FloatPoint a, FloatPoint b) { return a.second < b.second; };
	double xMin = std::min_element(corners.begin(), corners.end(), xLess)->first;
	double yMin = std::min_element(corners.begin(), corners.end(), yLess)->second;
	double xMax = std::max_element(corners.begin(), corners.end(), xLess)->first;
	double yMax = std::max_element(corners.begin(), corners.end(), yLess)->second;
	
	Surface retSurface(SDL_CreateRGBSurface(0, std::floor(xMax - xMin) + 1, std::floor(yMax - yMin) + 1, srf.get()->format->BitsPerPixel, srf.get()->format->Rmask, srf.get()->format->Gmask, srf.get()->format->Bmask, srf.get()->format->Amask));
	SDL_FillRect(retSurface.get(), NULL, SDL_MapRGBA(&imageFormat, 0, 0, 0, SDL_ALPHA_TRANSPARENT));

	for (int y = 0; y < retSurface.size().y; ++y) {
		for (int x = 0; x < retSurface.size().x; ++x) {
			FloatPoint temp{ x + xMin, y + yMin };
			temp = rotate(temp, true);
			if (temp.first >= 0 && temp.first < srf.size().x && temp.second >= 0 && temp.second < srf.size().y) {
				setPixel(retSurface.get(), x, y, getPixel(srf.get(), temp.first, temp.second));
			}
		}
	}

	SDL_SetColorKey(retSurface.get(), SDL_FALSE, SDL_MapRGBA(&imageFormat, 0, 0, 0, SDL_ALPHA_TRANSPARENT));

	return { retSurface, { int(xMin), int(yMin) } };
}

std::tuple < SDL_Point, SDL_Point, int > animation_effects::getPrincipalRect(SDL_Point offset, SDL_Point size, int angle) {
	angle %= 360;
	switch (angle / 90) {
	case 0:
		return { offset, size, angle % 90 };
	case 1:
		return { { size.x - offset.y, offset.x }, { size.y, size.x }, angle % 90 };
	case 2:
		return { { size.x - offset.x, size.y - offset.y }, size, angle % 90 };
	case 3:
		return { { offset.y, size.x - offset.x }, { size.y, size.x }, angle % 90 };
	}
}

SDL_Point animation_effects::getLeftmostPoint(SDL_Point offset, SDL_Point size, int angle) {
	angle = (angle % 360);
	switch (3 - angle / 90) {
	case 0:
		return rotate({ -offset.x, -offset.y }, angle);
	case 1:
		return rotate({ size.x - offset.x, -offset.y }, angle); 
	case 2:
		return rotate({ size.x - offset.x, size.y - offset.y }, angle);
	case 3:
		return rotate({ -offset.x, size.y - offset.y }, angle);
	}
}

SDL_Point animation_effects::getTopmostPoint(SDL_Point offset, SDL_Point size, int angle) {
	angle = (angle % 360);
	switch (3 - angle / 90) {
	case 0:
		return rotate({ size.x - offset.x, -offset.y }, angle);
	case 1:
		return rotate({ size.x - offset.x, size.y - offset.y }, angle);
	case 2:
		return rotate({ -offset.x, size.y - offset.y }, angle);
	case 3:
		return rotate({ -offset.x, -offset.y }, angle);
	}
}

SDL_Point animation_effects::getRotatedOffset(SDL_Point oldOffset, SDL_Point oldSize, int angle) {
	angle %= 360;
	
	if (angle == 0) {
		return oldOffset;
	}
	else {
		SDL_Point leftmost = getLeftmostPoint(oldOffset, oldSize, angle);
		SDL_Point topmost = getTopmostPoint(oldOffset, oldSize, angle);

		return { -leftmost.x, -topmost.y };
	}
}

Surface animation_effects::apply(const Rotation& effect, Surface srf) {
	return rotateSprite(effect, srf).first;
}

std::optional<SDL_Point> Animation::getOffset() {
	return offset;
}

Surface applyEffectList(Surface surface, const AnimationEffectList& effects) {
	for (const AnimationEffect& effect : effects) {
		auto apply_any = [&surface](const auto& arg) {
			using namespace animation_effects;
			return apply(arg, surface);
		};
		surface = std::visit(apply_any, effect);
	}

	return surface;
}

SDL_Rect getFrameWindow(SDL_Point size, int frame) {
	return SDL_Rect { size.x * frame, 0, size.x, size.y };
}

Surface getSurfaceRegion(const Surface& surface, const SDL_Rect& region) {
	Surface tempSrf(SDL_Point{ region.w, region.h }); 
	SDL_BlitSurface(surface.get(), &region, tempSrf.get(), nullptr);
	return tempSrf;
}

SDL_Point getFlippedOffset(SDL_Point size, SDL_Point offset, SDL_RendererFlip flip) {
	switch(static_cast<int>(flip)) {
	case SDL_FLIP_NONE:
		return offset;
	case SDL_FLIP_HORIZONTAL:
		return SDL_Point { size.x - offset.x, offset.y };
	case SDL_FLIP_VERTICAL:
		return SDL_Point { offset.x, size.y - offset.y };
	case (SDL_FLIP_HORIZONTAL | SDL_FLIP_VERTICAL):
		return SDL_Point { size.x - offset.x, size.y - offset.y };
	}
}

Surface expandSurface(const Surface& surface, SDL_Point moveTo) {
	SDL_Point newSize = surface.size() + SDL_Point{ std::abs(moveTo.x), std::abs(moveTo.y) };

	const auto& format = &imageFormat;

	Surface temp(SDL_CreateRGBSurface(0, newSize.x, newSize.y, format->BitsPerPixel, format->Rmask, format->Gmask, format->Bmask, format->Amask));
	SDL_FillRect(temp.get(), nullptr, SDL_MapRGBA(temp.get()->format, 0, 0, 0, SDL_ALPHA_TRANSPARENT));

	SDL_Rect destRect { -moveTo.x, -moveTo.y, surface.size().x, surface.size().y};
	SDL_BlitSurface(surface.get(), nullptr, temp.get(), &destRect);

	return temp;
}

void Animation::Render(SDL_Point dest, int rot, const SDL_Point* center, double ratio, SDL_RendererFlip flip, AnimationEffectList effects) const {
	if (sprite.empty()) {
		std::cout << "\nSpritesheet is null\n";
		throw "Animation Render Exception: Spritesheet Is Null!";
	}

	if (sprite.empty()) {
		std::cerr << "Sprite is null!\n";
	}
	
	SDL_Rect thisFrame = getFrameWindow({ sprite.size().x / numFrames, sprite.size().y }, currentFrame);

	if (effects.empty()) {
		dest -= offset.value_or(SDL_Point{ 0, 0 });

		SDL_Rect screenDest = { dest.x, dest.y, thisFrame.w, thisFrame.h };
		screenDest *= ratio;

		RenderExact(sprite, thisFrame, screenDest, flip, rot, center);
	}
	else {
		Sprite tempSprite(getSurfaceRegion(sprite.getSpriteSheet(), thisFrame));

		auto newOffset = offset;
		transform(tempSprite, effects, newOffset, flip);

		SDL_Rect src{ 0, 0, tempSprite.size().x, tempSprite.size().y };

		dest -= newOffset.value_or(SDL_Point{ 0, 0 });

		SDL_Rect screenDst = SDL_Rect{ dest.x, dest.y, tempSprite.size().x, tempSprite.size().y } * ratio;

		RenderExact(tempSprite, src, screenDst, flip, rot, center);
	}
}

void Animation::setDelay(DurationType delay) {
	timer.duration = delay;
}

void Animation::transform(Sprite& sprite, AnimationEffectList effects, std::optional < SDL_Point >& offset, SDL_RendererFlip& flip) {
	SDL_Point oldSize = sprite.size();

	for (AnimationEffect& effect : effects) {
		if (std::holds_alternative<animation_effects::Rotation>(effect)) {
			auto& rotation = std::get<animation_effects::Rotation>(effect);
			if (flip == SDL_FLIP_NONE) {
				rotation.center += offset.value_or(SDL_Point{ 0, 0 });
			}
			else {
				sprite.setSpriteSheet(Animation::FlipSurface(sprite.getSpriteSheet(), flip));
				
				if (offset) {
					*offset = getFlippedOffset(sprite.size(), *offset, flip);
					
					rotation.center += *offset;
				}
				
				flip = SDL_FLIP_NONE;
			}

			if (offset) {
				*offset = animation_effects::getRotatedOffset(*offset, sprite.size(), rotation.degrees);
			}
		}
	}

	sprite.setSpriteSheet(applyEffectList(sprite.getSpriteSheet(), effects));
}

void Animation::RenderExact(const Sprite& sprite, const SDL_Rect& src, const SDL_Rect& dst, SDL_RendererFlip flip, int degrees, const SDL_Point* center) {
	SDL_RenderCopyEx(globalObjects::renderer, sprite.getTexture().get(), &src, &dst, degrees, center, flip);
}

Uint32 getPixel(const SDL_Surface* surface, const int& x, const int& y) {
	return (static_cast < Uint32 * >(surface->pixels))[x + y * surface->w];
}

void setPixel(SDL_Surface* surface, int x, int y, Uint32 pixel) {
	Uint32* pixels = (Uint32 *)surface->pixels;

	pixels[x + y * surface->w] = pixel;
}

void Animation::addStaticEffects(const std::vector < AnimationEffectList >& effects) {
	int width = sprite.size().x;
	int height = sprite.size().y;
	if (effects.size() > numFrames) {
		Surface newSheet(SDL_Point{ static_cast<int>(effects.size() * width), height });
		SDL_Rect dst{ 0, 0, sprite.size().x, sprite.size().y };
		for (int frame = 0; frame < effects.size(); ++frame) {
			SDL_Rect destination = getFrameWindow({ width, height }, frame);
			SDL_BlitSurface(applyEffectList(sprite.getSpriteSheet(), effects[frame]).get(), nullptr, newSheet.get(), &destination);
		}
		numFrames *= effects.size();
		sprite.setSpriteSheet(newSheet);
		/*SDL_BlitSurface(sprite.getSpriteSheet().get(), nullptr, newSheet.get(), nullptr);
		while (effects.size() > numFrames) {
			++numFrames;
			SDL_Rect newArea = getFrameWindow({ width, height }, numFrames - 1);
			Surface temp = getSurfaceFrame(0);
			std::cout << newArea.x << " " << newArea.y << " " << newArea.w << " " << newArea.h << "\n";
			SDL_BlitSurface(temp.get(), nullptr, newSheet.get(), &newArea);
		}
		sprite.setSpriteSheet(newSheet);*/
	}
	

	for (int frame = 0; frame < effects.size(); ++frame) {
		Surface current = getSurfaceFrame(frame);

		current = applyEffectList(current, effects[frame]);

		SDL_Rect tempFrame = getFrameWindow({ width, height }, frame);
		SDL_BlitSurface(current.get(), nullptr, sprite.getSpriteSheet().get(), &tempFrame);
	}

	sprite.updateTexture();
}

Surface Animation::FlipSurface(const Surface& srf, SDL_RendererFlip flip) {
	SDL_Surface* surface = srf.get();
	if (flip == SDL_FLIP_NONE) {
		return srf;
	}
	Surface outputSrf(SDL_Point { surface->w, surface->h }); 
	SDL_Surface* outputSurface = outputSrf.get();
	if (SDL_MUSTLOCK(surface)) {
		SDL_LockSurface(surface);
	}

	for (int x = 0, rx = outputSurface->w - 1; x < outputSurface->w; x++, rx--) {
		for (int y = 0, ry = outputSurface->h - 1; y < outputSurface->h; y++, ry--) {
			Uint32 pixel = ::getPixel(surface, x, y);
			if ((flip & SDL_FLIP_HORIZONTAL) && (flip & SDL_FLIP_VERTICAL)) {
				//Horizontal and vertical
				setPixel(outputSurface, rx, ry, pixel);
			}
			else if (flip & SDL_FLIP_HORIZONTAL) {
				//Only horizontal
				setPixel(outputSurface, rx, y, pixel);
			}
			else {
				//Only vertical
				setPixel(outputSurface, x, ry, pixel);
			}
		}
	}

	if (SDL_MUSTLOCK(surface)) {
		SDL_UnlockSurface(surface);
	}

	return outputSrf;
}

void Animation::updateTexture() {
	sprite.updateTexture();
}

Uint32 Animation::getPixel(const int& x, const int& y) const {
	return ::getPixel(sprite.getSpriteSheet().get(), x, y);
}

SDL_Surface* Animation::getSpriteSheet() {
	return sprite.getSpriteSheet().get();
}

Surface Animation::getSurfaceFrame(int frame) {
	SDL_Rect frameWindow = getFrameWindow({ sprite.size().x / numFrames, sprite.size().y }, frame);
	return getSurfaceRegion(sprite.getSpriteSheet(), frameWindow);
}

Surface Animation::getSurfaceFrame() {
	return getSurfaceFrame(currentFrame);
}

void Animation::synchronize(const Animation& other) noexcept {
	assert(numFrames == other.numFrames);
	timer = other.timer;
	currentFrame = other.currentFrame;
}

void swap(Animation& lhs, Animation& rhs) noexcept {
	using std::swap;

	swap(lhs.sprite, rhs.sprite);
	swap(lhs.offset, rhs.offset);
	swap(lhs.timer, rhs.timer);
	swap(lhs.currentFrame, rhs.currentFrame);
	swap(lhs.numFrames, rhs.numFrames);
	swap(lhs.hasLooped, rhs.hasLooped);
}
