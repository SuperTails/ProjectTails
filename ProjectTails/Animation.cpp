#include "stdafx.h"
#include "Animation.h"

Uint32 Animation::deltaTime;
SDL_Renderer* Animation::renderer;

Animation::Animation() {
	SpriteSheet = nullptr;
	tex = nullptr;
}

Animation::Animation(const Animation& arg) {
	Delay = arg.Delay;
	SpriteSheet = SDL_ConvertSurface(arg.SpriteSheet, arg.SpriteSheet->format, SDL_RLEACCEL);
	if (SpriteSheet == NULL) {
		std::cout << SDL_GetError() << "\n";
		throw "Spritesheet could not be created";
	}
	numFrames = arg.numFrames;
	FrameWindows = arg.FrameWindows;
	looped = arg.looped;
	frame = arg.frame;
	tile = arg.tile;
	timeError = arg.timeError;
	tex = SDL_CreateTextureFromSurface(globalObjects::renderer, SpriteSheet);
	if (tex == NULL) {
		std::cout << SDL_GetError() << "\n";
		throw "Texture could not be created";
	}
}

Animation::Animation(Animation&& other) :
	Delay(std::move(other.Delay)),
	numFrames(std::move(other.numFrames)),
	FrameWindows(std::move(other.FrameWindows)),
	looped(std::move(other.looped)),
	frame(std::move(other.frame)),
	tile(std::move(other.tile)),
	timeError(std::move(other.timeError)),
	tex(nullptr),
	SpriteSheet(nullptr)
{
	using std::swap;

	swap(SpriteSheet, other.SpriteSheet);

	if (other.tex) {
		swap(tex, other.tex);
	}
	else {
		tex = SDL_CreateTextureFromSurface(globalObjects::renderer, SpriteSheet);
		if (tex == NULL) {
			std::cout << SDL_GetError() << "\n";
			throw "Texture could not be created";
		}
	}
	std::cout << "Animation move constructor was called!\n";
}

Animation::Animation(SDL_Surface* Sprites, int delay, int frames, bool til) :
	timeError(0),
	tile(til),
	Delay(delay),
	numFrames(frames),
	frame(0),
	FrameWindows(frames),
	looped(false),
	SpriteSheet(NULL)
{
	SDL_Surface* s = Sprites;
	SDL_FreeSurface(SpriteSheet);
	SpriteSheet = SDL_ConvertSurface(s, SDL_GetWindowSurface(globalObjects::window)->format, NULL);
	if (SpriteSheet == NULL) {
		std::cout << SDL_GetError() << "\n";
		throw "Spritesheet could not be created";
	}
	SDL_FreeSurface(s);
	SDL_SetSurfaceRLE(SpriteSheet, SDL_TRUE);
	SDL_SetColorKey(SpriteSheet, SDL_RLEACCEL, SDL_MapRGBA(SDL_GetWindowSurface(globalObjects::window)->format, 1, 2, 3, SDL_ALPHA_OPAQUE));
	SDL_SetSurfaceBlendMode(SpriteSheet, SDL_BLENDMODE_NONE);
	for (int i = 0; i < frames; i++) {
		FrameWindows[i].x = i*(Sprites->w) / frames;
		FrameWindows[i].y = 0;
		FrameWindows[i].w = (Sprites->w) / frames;
		FrameWindows[i].h = (Sprites->h);
	}
	tex = SDL_CreateTextureFromSurface(globalObjects::renderer, SpriteSheet);
	if (tex == NULL) {
		std::cout << SDL_GetError() << "\n";
		throw "Texture could not be created";
	}
}

Animation::Animation(AnimStruct a) :
	timeError(0),
	tile(false),
	Delay(a.delay),
	numFrames(a.frames),
	frame(0),
	FrameWindows(a.frames),
	looped(false),
	tex(NULL)
{
	SDL_Surface* s = IMG_Load(a.SpritePath.c_str());
	SpriteSheet = SDL_ConvertSurface(s, SDL_GetWindowSurface(globalObjects::window)->format, NULL);
	if (SpriteSheet == NULL) {
		std::cout << SDL_GetError() << "\n";
		throw "Spritesheet could not be created";
	}
	SDL_FreeSurface(s);
	SDL_SetSurfaceRLE(SpriteSheet, SDL_TRUE);
	SDL_SetColorKey(SpriteSheet, SDL_TRUE, SDL_MapRGBA(SDL_GetWindowSurface(globalObjects::window)->format, 1, 2, 3, SDL_ALPHA_OPAQUE));
	SDL_SetSurfaceBlendMode(SpriteSheet, SDL_BLENDMODE_NONE);
	for (int i = 0; i < a.frames; i++) {
		FrameWindows[i].x = i*(SpriteSheet->w) / a.frames;
		FrameWindows[i].y = 0;
		FrameWindows[i].w = (SpriteSheet->w) / a.frames;
		FrameWindows[i].h = (SpriteSheet->h);
	}
	tex = SDL_CreateTextureFromSurface(globalObjects::renderer, SpriteSheet);
	if (tex == NULL) {
		std::cout << SDL_GetError() << "\n";
		throw "Texture could not be created";
	}
}

Animation::Animation(SDL_Point tileSize) :
	timeError(0),
	tile(true),
	FrameWindows(1, { 0, 0, constants::GROUND_PIXEL_WIDTH, constants::GROUND_PIXEL_WIDTH }),
	numFrames(1),
	tex(NULL)
{
	SDL_Surface* s = SDL_CreateRGBSurface(0, constants::GROUND_PIXEL_WIDTH, constants::GROUND_PIXEL_WIDTH, 32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
	SpriteSheet = SDL_ConvertSurface(s, SDL_GetWindowSurface(globalObjects::window)->format, NULL);
	if (SpriteSheet == NULL) {
		std::cout << SDL_GetError() << "\n";
		throw "Spritesheet could not be created";
	}
	SDL_FreeSurface(s);
	SDL_SetSurfaceRLE(SpriteSheet, SDL_TRUE);
	SDL_SetColorKey(SpriteSheet, SDL_RLEACCEL, SDL_MapRGBA(SDL_GetWindowSurface(globalObjects::window)->format, 1, 2, 3, SDL_ALPHA_OPAQUE));
	SDL_FillRect(SpriteSheet, NULL, SDL_MapRGBA(SDL_GetWindowSurface(globalObjects::window)->format, 1, 2, 3, SDL_ALPHA_OPAQUE));
};

void Animation::SetFrame(int f) {
	frame = f;
}

void Animation::Update() {
	if (tex == NULL) {
		tex = SDL_CreateTextureFromSurface(globalObjects::renderer, SpriteSheet);
	}
	if (!tile && Delay != -1) {
		timeError += deltaTime;
		if (timeError >= Delay) {
			looped |= (frame == numFrames - 1);
			++frame;
			frame %= numFrames;
			timeError -= Delay;
		}
	}
}

Animation::~Animation()
{
	if (tex) {
		SDL_DestroyTexture(tex);
	}
	if (SpriteSheet) {
		SDL_FreeSurface(SpriteSheet);
	}
	std::vector< SDL_Rect >().swap(FrameWindows);
}

void Animation::Render(const SDL_Rect* dest, const int& rot, SDL_Point* center, const double& ratio, const SDL_RendererFlip& flip, const effectType& effect, const effectData* efxData){
	if (SpriteSheet == NULL) {
		std::cout << "\nSpritesheet is null\n";
		throw "Animation Render Exception: Spritesheet Is Null!";
	}
	SDL_Texture* tempTex = nullptr;
	SDL_Surface* tempSrf = nullptr;
	SDL_Rect thisDest = *dest;

	SDL_Rect thisFrame = FrameWindows[frame];
	SDL_RendererFlip thisFlip = flip;
	if (effect) {
		std::swap(tempTex, tex);
		tempSrf = getSurfaceFrame();
		SDL_SetColorKey(tempSrf, SDL_RLEACCEL, SDL_MapRGBA(tempSrf->format, 1, 2, 3, SDL_ALPHA_OPAQUE));
		thisFrame.x = 0;
		thisFrame.y = 0;
		int oldWidth = tempSrf->w;
		int oldHeight = tempSrf->h;

		if (effect & PALETTE_SWAP) {
			SDL_Surface* temp = Animation::PaletteSwap(tempSrf, efxData->swp.oldColors, efxData->swp.newColors);
			SDL_FreeSurface(tempSrf);
			tempSrf = temp;
		}
		if (effect & ROTATION) {
			thisFlip = SDL_FLIP_NONE;
			SDL_Surface* temp = Animation::FlipSurface(tempSrf, flip);
			SDL_Point offset{ 0, 0 };
			SDL_FreeSurface(tempSrf);
			tempSrf = temp;
			temp = Animation::Rotate(tempSrf, efxData->rot.center, efxData->rot.degrees, offset);
			SDL_FreeSurface(tempSrf);
			tempSrf = temp;
			thisDest.x += offset.x;
			thisDest.y += offset.y;
		}

		thisFrame.w = tempSrf->w;
		thisFrame.h = tempSrf->h;

		tex = SDL_CreateTextureFromSurface(globalObjects::renderer, tempSrf);
		SDL_FreeSurface(tempSrf);
	}

	if (tex == NULL) {
		std::cout << "\nTexture is null\n";
		throw "Animation Render Exception: Texture is Null!";
	}
	if (center != NULL) {
		center->x *= ratio;
		center->y *= ratio;
	}
	SDL_Rect screenDest = { int(thisDest.x * ratio), int(thisDest.y * ratio), int(thisFrame.w * ratio), int(thisFrame.h * ratio) };
	SDL_RenderCopyEx(globalObjects::renderer, tex, &thisFrame, &screenDest, (effect & ROTATION) ? 0.0 : double(rot), center, thisFlip);
	if (tempTex != nullptr) {
		std::swap(tempTex, tex);
		SDL_DestroyTexture(tempTex);
	}
}

void Animation::SizeRender(SDL_Rect* dest, int rot, SDL_Point* center, double ratio) {
	if (SpriteSheet == NULL) {
		throw "Animation Render Exception: Spritesheet Is Null!";
		std::cerr << "Anim render Exception!";
	}
	SDL_Rect screenDest = { (int)((dest->x) * ratio), (int)((dest->y) * ratio),(int)(dest->w * ratio),(int)(dest->h * ratio)/*,dest->w, dest->h*/ };
	if (center != NULL) {
		center->x *= ratio;
		center->y *= ratio;
	}
	SDL_RenderCopyEx(renderer, tex, &FrameWindows[frame], &screenDest, double(rot), center, SDL_FLIP_NONE);
	SDL_DestroyTexture(tex);
}

void Animation::SetDelay(int delay) {
	Delay = delay;
}

Uint32 Animation::getPixel(const SDL_Surface* surface, const int& x, const int& y) {
	return (static_cast < Uint32 * >(surface->pixels))[x + y * surface->w];
}

void Animation::setPixel(SDL_Surface* surface, int x, int y, Uint32 pixel) {
	Uint32* pixels = (Uint32 *)surface->pixels;

	pixels[x + y * surface->w] = pixel;
}

SDL_Surface* Animation::GetSpriteSheet(SDL_RendererFlip flip) {
	//1st bit = horizontal flip
	//2nd bit = vertical flip
	if (flip == SDL_FLIP_NONE) {
		return SpriteSheet;
	}
	SDL_Surface* outputSurface = SDL_CreateRGBSurface(0, SpriteSheet->w, SpriteSheet->h, SpriteSheet->format->BitsPerPixel, SpriteSheet->format->Rmask, SpriteSheet->format->Gmask, SpriteSheet->format->Bmask, 0);
	if (outputSurface == NULL) {
		std::cout << SDL_GetError() << "\n";
		throw "Output surface could not be created";
	}

	if (SDL_MUSTLOCK(SpriteSheet)) {
		SDL_LockSurface(SpriteSheet);
	}
	
	for (int x = 0, rx = outputSurface->w - 1; x < outputSurface->w; x++, rx--) {
		for (int y = 0, ry = outputSurface->h - 1; y < outputSurface->h; y++, ry--) {
			Uint32 pixel = getPixel(SpriteSheet, x, y);
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

	if (SDL_MUSTLOCK(SpriteSheet)) {
		SDL_UnlockSurface(SpriteSheet);
	}
	SDL_SetSurfaceRLE(outputSurface, SDL_TRUE);
	SDL_SetColorKey(outputSurface, SDL_TRUE, SDL_MapRGBA(SpriteSheet->format, 1, 2, 3, SDL_ALPHA_OPAQUE));

	return outputSurface;
}

void Animation::addStaticEffects(const std::vector < std::vector < effectType > >& types, const std::vector < std::vector < effectData > >& effects) {
	SDL_DestroyTexture(tex);

	if (types.size() > FrameWindows.size()) {
		int width = FrameWindows[0].w;
		SDL_Surface* newSheet = SDL_CreateRGBSurface(0, types.size() * width, FrameWindows[0].h, SpriteSheet->format->BitsPerPixel, SpriteSheet->format->Rmask, SpriteSheet->format->Gmask, SpriteSheet->format->Bmask, SpriteSheet->format->Amask);
		SDL_FillRect(newSheet, NULL, SDL_MapRGB(newSheet->format, 1, 2, 3));
		SDL_SetColorKey(newSheet, SDL_RLEACCEL, SDL_MapRGBA(newSheet->format, 1, 2, 3, SDL_ALPHA_OPAQUE));
		SDL_Rect dst{ 0, 0, SpriteSheet->w, SpriteSheet->h };
		SDL_Surface* temp = SDL_ConvertSurface(SpriteSheet, SpriteSheet->format, 0);
		SDL_BlitSurface(temp, NULL, newSheet, &dst);
		SDL_FreeSurface(temp);
		while (types.size() > FrameWindows.size()) {
			SDL_Rect back = FrameWindows.back();
			FrameWindows.push_back(SDL_Rect{ back.x + back.w, back.y, back.w, back.h });
			SDL_BlitSurface(getSurfaceFrame(0), NULL, newSheet, &FrameWindows.back());
		}
		SDL_FreeSurface(SpriteSheet);
		SpriteSheet = newSheet;
		numFrames = FrameWindows.size();
	}
	

	for (int frame = 0; frame < types.size(); frame++) {
		SDL_Surface* current = getSurfaceFrame(frame);

		for (int effectIndex = 0; effectIndex < types[frame].size(); effectIndex++) {
			const effectType& type = types[frame][effectIndex];
			const effectData& effect = effects[frame][effectIndex];
			if (type & PALETTE_SWAP) {
				SDL_Surface* temp = Animation::PaletteSwap(current, effect.swp.oldColors, effect.swp.newColors);
				SDL_FreeSurface(current);
				current = temp;
			}
		}

		SDL_BlitSurface(current, NULL, SpriteSheet, &FrameWindows[frame]);
	}

	tex = SDL_CreateTextureFromSurface(globalObjects::renderer, SpriteSheet);
}

SDL_Surface* Animation::FlipSurface(SDL_Surface* surface, SDL_RendererFlip flip) {
	if (flip == SDL_FLIP_NONE) {
		return SDL_ConvertSurface(surface, surface->format, SDL_SWSURFACE);
	}
	SDL_Surface* outputSurface = SDL_CreateRGBSurface(0, surface->w, surface->h, surface->format->BitsPerPixel, surface->format->Rmask, surface->format->Gmask, surface->format->Bmask, 0);

	if (SDL_MUSTLOCK(surface)) {
		SDL_LockSurface(surface);
	}

	for (int x = 0, rx = outputSurface->w - 1; x < outputSurface->w; x++, rx--) {
		for (int y = 0, ry = outputSurface->h - 1; y < outputSurface->h; y++, ry--) {
			Uint32 pixel = getPixel(surface, x, y);
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
	SDL_SetSurfaceRLE(outputSurface, SDL_TRUE);
	SDL_SetColorKey(outputSurface, SDL_TRUE, SDL_MapRGBA(surface->format, 1, 2, 3, SDL_ALPHA_OPAQUE));

	return outputSurface;
}

SDL_Surface* Animation::PaletteSwap(SDL_Surface* surface, const std::vector < Uint32 >& oldColors, const std::vector < Uint32 >& newColors) {
	SDL_Surface* src = SDL_ConvertSurface(surface, surface->format, 0);

	SDL_SetSurfaceRLE(src, SDL_TRUE);
	
	SDL_Surface* dst = SDL_CreateRGBSurface(0, surface->w, surface->h, surface->format->BitsPerPixel, surface->format->Rmask, surface->format->Gmask, surface->format->Bmask, surface->format->Amask);
	
	SDL_FillRect(dst, NULL, SDL_MapRGB(surface->format, 1, 2, 3));

	for (int i = 0; i < oldColors.size(); i++) {
		SDL_SetColorKey(src, SDL_TRUE, oldColors[i]);
		SDL_FillRect(dst, NULL, newColors[i]);
		SDL_BlitSurface(src, NULL, dst, NULL);
		SDL_FreeSurface(src);
		src = SDL_ConvertSurface(dst, dst->format, SDL_RLEACCEL);
	}

	SDL_SetColorKey(dst, SDL_TRUE, SDL_MapRGB(surface->format, 1, 2, 3));
	SDL_SetSurfaceRLE(dst, SDL_TRUE);

	SDL_FreeSurface(src);
	return dst;
}

SDL_Surface* Animation::Rotate(SDL_Surface* surface, const SDL_Point* cntr, int degrees, SDL_Point& offset) {
	if (degrees == 0) {
		offset = { 0, 0 };
		return SDL_ConvertSurface(surface, surface->format, 0);
	}

	SDL_Point center{ surface->w / 2, surface->h / 2 };
	if (cntr)
		center = *cntr;

	double rad = degrees * M_PI / 180.0;

	auto rotate = [center, rad](SDL_Point& p, bool reverse = false) {
		int x = p.x;
		int y = p.y;
		double ang = rad * ((reverse * -2) + 1);
		p.x = center.x + (x - center.x) * cos(ang) - (y - center.y) * sin(ang);
		p.y = center.y + (x - center.x) * sin(ang) + (y - center.y) * cos(ang);
	};
	
	std::vector < SDL_Point > corners{ { 0, 0 }, { surface->w - 1, 0 }, { 0, surface->h - 1 }, { surface->w - 1, surface->h - 1 } };
	std::for_each(corners.begin(), corners.end(), rotate);

	int xMin = corners[0].x;
	int xMax = corners[0].x;
	int yMin = corners[0].y;
	int yMax = corners[0].y;
	for (int i = 1; i < 4; i++) {
		xMin = std::min(xMin, corners[i].x);
		xMax = std::max(xMax, corners[i].x);
		yMin = std::min(yMin, corners[i].y);
		yMax = std::max(yMax, corners[i].y);
	}

	SDL_Surface* retSurface = SDL_CreateRGBSurface(0, xMax - xMin + 1, yMax - yMin + 1, surface->format->BitsPerPixel, surface->format->Rmask, surface->format->Gmask, surface->format->Bmask, surface->format->Amask);
	SDL_FillRect(retSurface, NULL, SDL_MapRGB(retSurface->format, 1, 2, 3));

	for (int y = 0; y < retSurface->h; y++) {
		for (int x = 0; x < retSurface->w; x++) {
			SDL_Point temp{ x + xMin, y + yMin };
			rotate(temp, true);
			if (temp.x > 0 && temp.x < surface->w - 1 && temp.y > 0 && temp.y < surface->h - 1) {
				setPixel(retSurface, x, y, getPixel(surface, temp.x, temp.y));
			}
		}
	}

	SDL_SetColorKey(retSurface, SDL_RLEACCEL, SDL_MapRGBA(retSurface->format, 1, 2, 3, SDL_ALPHA_OPAQUE));

	offset = { xMin, yMin };

	return retSurface;
}

void Animation::setRenderer(SDL_Renderer* r) {
	renderer = r;
}

bool Animation::updateTexture() {
	SDL_DestroyTexture(tex);
	tex = SDL_CreateTextureFromSurface(globalObjects::renderer, SpriteSheet);
	if (tex)
		return true;
	std::cout << SDL_GetError() << "\n";
	return false;
}

Uint32 Animation::getPixel(const int& x, const int& y) const {
	return getPixel(SpriteSheet, x, y);
}

SDL_Surface* Animation::getSurfaceFrame(int frame) {
	SDL_Surface* temp = SDL_CreateRGBSurface(0, FrameWindows[frame].w, FrameWindows[frame].h, SpriteSheet->format->BitsPerPixel, SpriteSheet->format->Rmask, SpriteSheet->format->Gmask, SpriteSheet->format->Bmask, SpriteSheet->format->Amask);
	SDL_Rect dst{ 0, 0, FrameWindows[frame].w, FrameWindows[frame].h };
	SDL_FillRect(temp, NULL, SDL_MapRGBA(temp->format, 1, 2, 3, SDL_ALPHA_OPAQUE));
	SDL_Surface* tempSrc = SDL_ConvertSurface(SpriteSheet, SpriteSheet->format, SDL_RLEACCEL);
	SDL_BlitSurface(tempSrc, &FrameWindows[frame], temp, &dst);
	SDL_FreeSurface(tempSrc);
	SDL_SetColorKey(temp, SDL_RLEACCEL, SDL_MapRGB(temp->format, 1, 2, 3));
	return temp;
}

SDL_Surface* Animation::getSurfaceFrame() {
	return getSurfaceFrame(frame);
}