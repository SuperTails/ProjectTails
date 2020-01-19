#include "Sprite.h"
#include <SDL.h>
#include <SDL_image.h>
#include <iostream>
#include <stdexcept>
#include <iomanip>

void std::default_delete <SDL_Surface>::operator() (SDL_Surface* ptr) const {
	SDL_FreeSurface(ptr);
}

void std::default_delete < SDL_Texture >::operator() (SDL_Texture* ptr) const {
	SDL_DestroyTexture(ptr);
}

SDL_PixelFormat getFormat() {
	SDL_PixelFormat format;
	memset(&format, 0, sizeof(format));
	format.BitsPerPixel = 32;
	format.BytesPerPixel = 4;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	format.Rmask = 0xFF000000;
	format.Gmask = 0x00FF0000;
	format.Bmask = 0x0000FF00;
	format.Amask = 0x000000FF;
#else
	format.Rmask = 0x000000FF;
	format.Gmask = 0x0000FF00;
	format.Bmask = 0x00FF0000;
	format.Amask = 0xFF000000;
#endif
	format.Rshift = __builtin_ctz(format.Rmask);
	format.Gshift = __builtin_ctz(format.Gmask);
	format.Bshift = __builtin_ctz(format.Bmask);
	format.Ashift = __builtin_ctz(format.Amask);
	return format;
}

void Surface::setFlags() {
	SDL_Surface* temp = SDL_ConvertSurface(surface, &imageFormat, SDL_RLEACCEL);
	SDL_FreeSurface(surface);
	surface = temp;
	SDL_SetSurfaceRLE(surface, SDL_TRUE);
	SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_BLEND);
}

Surface::Surface() noexcept :
	surface(nullptr) {

}

Surface::Surface(const Surface& s) :
	surface(nullptr) {
	if(s.surface != nullptr) {
		surface = SDL_ConvertSurface(s.surface, s.surface->format, 0);
		if (surface == nullptr) {
			std::cerr << "Could not copy surface! Error: " << SDL_GetError() << "\n";
			throw std::runtime_error("Could not copy surface!");
		}
		else {
			setFlags();
		}
	}
}

Surface::Surface(Surface&& s) noexcept :
	Surface() {
	
	swap(*this, s);
}

Surface::Surface(const std::string& path) :
	surface(IMG_Load(path.data())) {
	
	if (surface == nullptr) {
		std::cerr << "Could not load surface from filepath " << std::quoted(std::string{ path }) << "! Error: " << SDL_GetError() << "\n";
		throw std::runtime_error("Could not load surface from file " + std::string{ path });
	}
	setFlags();
}

Surface::Surface(SDL_Surface* s) :
	surface(s) {
		setFlags();
		s = nullptr;
}

Surface::Surface(SDL_Point size) :
	surface(nullptr) {
	const auto& format = &imageFormat;
	reset(SDL_CreateRGBSurface(0, size.x, size.y, format->BitsPerPixel, format->Rmask, format->Gmask, format->Bmask, format->Amask));
	SDL_FillRect(surface, nullptr, SDL_MapRGBA(surface->format, 0, 0, 0, SDL_ALPHA_TRANSPARENT));
	setFlags();
	if (surface == nullptr) {
		std::cerr << "Could not create blank sprite! Error: " << SDL_GetError() << "\n";
	}
}


Surface::~Surface() {
	release();
}

SDL_Surface* Surface::get() const {
	return surface;
}

Surface& Surface::operator= (const Surface& s) {
	*this = Surface{ s };

	return *this;
}

Surface& Surface::operator= (Surface&& s) noexcept {
	swap(*this, s);

	return *this;
}

void Surface::release() {
	if (surface != nullptr) {
		std::default_delete< SDL_Surface > deleter;
		deleter(surface);
		surface = nullptr;
	}
}

void Surface::reset(SDL_Surface* s) {
	release();

	surface = s;
	s = nullptr;

	setFlags();
}

SDL_Point Surface::size() const {
	return SDL_Point { surface->w, surface->h };
}

void swap(Surface& a, Surface& b) noexcept {
	using std::swap;
	swap(a.surface, b.surface);
}

bool operator== (const Surface& s, std::nullptr_t) {
	return s.get() == nullptr;
}
bool operator== (std::nullptr_t, const Surface& s) {
	return s.get() == nullptr;
}

bool operator!= (const Surface& s, std::nullptr_t) {
	return !(s == nullptr);
}
bool operator!= (std::nullptr_t, const Surface& s) {
	return !(nullptr == s);
}

Texture::Texture() noexcept : 
	texture(nullptr) {

}

Texture::Texture(Texture&& t) noexcept :
	Texture() {

	swap(*this, t);
}

Texture::Texture(const Surface& s) :
	Texture() {
	if (s != nullptr) {
		texture = SDL_CreateTextureFromSurface(globalObjects::renderer, s.get());
		if (texture == nullptr) {
			std::cerr << "Could not create texture from surface! Error: " << SDL_GetError() << "\n";
			throw std::runtime_error("Could not create texture from surface!");
		}
	}
}

Texture::~Texture() {
	release();
}

Texture& Texture::operator= (Texture&& t) {
	swap(*this, t);

	return *this;
}

void Texture::release() {
	if (texture != nullptr) {
		std::default_delete<SDL_Texture> deleter;
		deleter(texture);
		texture = nullptr;
	}
}

SDL_Texture* Texture::get() const {
	return texture;
}

void swap(Texture& a, Texture& b) noexcept {
	using std::swap;
	swap(a.texture, b.texture);
}

bool operator== (const Texture& texture, std::nullptr_t) {
	return texture.get() == nullptr;
}
bool operator== (std::nullptr_t, const Texture& texture) {
	return texture.get() == nullptr;
}

Sprite::Sprite(const Sprite& sprite) : spriteSheet(sprite.spriteSheet),
	texture(spriteSheet) {

	if (sprite.spriteSheet != nullptr && (spriteSheet == nullptr || texture == nullptr)) {
		std::cerr << "Could not create sprite! Error: " << SDL_GetError() << "\n";
	}
}

Sprite::Sprite(const std::string& path) :
	spriteSheet(path),
	texture(spriteSheet) {

	if (spriteSheet == nullptr || texture == nullptr) {
		std::cerr << "Could not load sprite from file! Error: " << SDL_GetError() << "\n";
	}
}

Sprite::Sprite(const Surface& s) :
	spriteSheet(s),
	texture(spriteSheet) {
	if (spriteSheet == nullptr || texture == nullptr) {
		std::cerr << "Could not create sprite from surface!\n";
	}
}

Sprite::Sprite(Surface&& s) :
	spriteSheet(s),
	texture(spriteSheet) {
	if (spriteSheet == nullptr || texture == nullptr) {
		std::cerr << "Could not create sprite from surface!\n";
	}
}

Sprite::Sprite(SDL_Surface* s) :
	Sprite(Surface{s}) {

	s = nullptr;
}

const Surface& Sprite::getSpriteSheet() const {
	return spriteSheet;
}

const Texture& Sprite::getTexture() const {
	return texture;
}

Surface& Sprite::getSpriteSheet() {
	return spriteSheet;
}

Texture& Sprite::getTexture() {
	return texture;
}

void Sprite::setSpriteSheet(const Surface& s) {
	spriteSheet = s;
	texture = Texture(spriteSheet);
}

void Sprite::updateTexture() {
	spriteSheet.setFlags();
	texture = Texture(spriteSheet);
}

SDL_Point Sprite::size() const {
	return spriteSheet.size();
}

bool Sprite::empty() const {
	return (spriteSheet == nullptr);
}

void Sprite::render(SDL_Rect position) const {
	SDL_RenderCopy(globalObjects::renderer, texture.get(), nullptr, &position);
}

Sprite& Sprite::operator= (const Sprite& sprite) {
	*this = Sprite{ sprite };

	return *this;
}

Sprite& Sprite::operator= (Sprite&& sprite) noexcept {
	swap(*this, sprite);
	
	return *this;
}

void swap(Sprite& lhs, Sprite& rhs) noexcept {
	using std::swap;

	swap(lhs.spriteSheet, rhs.spriteSheet);
	swap(lhs.texture, rhs.texture);
}

SDL_Point getSize(const Sprite& sprite) {
	return { sprite.getSpriteSheet().get()->w, sprite.getSpriteSheet().get()->h };
}

Uint32* begin(SDL_Surface* surface) {
	return static_cast< Uint32* >(surface->pixels);
}

Uint32* end(SDL_Surface* surface) {
	return static_cast< Uint32* >(surface->pixels) + surface->w * surface->h;
}

Uint32* begin(Surface& surface) {
	return begin(surface.get());
}

Uint32* end(Surface& surface) {
	return end(surface.get());
}
