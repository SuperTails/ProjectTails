#pragma once
#include <SDL.h>
#include <memory>

struct SDL_Window;
struct SDL_Renderer;
struct SDL_Surface;
struct SDL_Texture;
struct SDL_Point;

namespace globalObjects {
	extern SDL_Window* window;
	extern SDL_Renderer* renderer;
}


SDL_PixelFormat getFormat();

const SDL_PixelFormat imageFormat = getFormat();

class Surface {
	SDL_Surface* surface;
public:
	struct PixelLock {
		PixelLock(Surface& srf) : surface(srf) { if (SDL_MUSTLOCK(surface.get())) SDL_LockSurface(surface.get()); };

		~PixelLock() { if (SDL_MUSTLOCK(surface.get())) SDL_UnlockSurface(surface.get()); };

		Surface& surface;
	};

	Surface() noexcept;
	Surface(const Surface& s);
	Surface(Surface&& s) noexcept;
	Surface(const std::string& str);

	void setFlags();

	explicit Surface(SDL_Surface* s);

	explicit Surface(SDL_Point size);

	~Surface();

	Surface& operator= (const Surface& s);

	Surface& operator= (Surface&& s) noexcept;

	void release();

	void reset(SDL_Surface* s);

	SDL_Surface* get() const;

	SDL_Point size() const;

	friend void swap(Surface& a, Surface& b) noexcept;
};

bool operator== (const Surface& surface, std::nullptr_t);
bool operator== (std::nullptr_t, const Surface& surface);

bool operator!= (const Surface& surface, std::nullptr_t);
bool operator!= (std::nullptr_t, const Surface& surface);

class Texture {
	SDL_Texture* texture;
public:
	Texture() noexcept;
	Texture(const Texture& t) = delete;
	Texture(Texture&& t) noexcept;

	Texture(const Surface& s);
	Texture(Surface&& s) = delete;

	~Texture();

	Texture& operator= (Texture&& t);

	void release();

	SDL_Texture* get() const;

	friend void swap(Texture& a, Texture& b) noexcept;
};

bool operator== (const Texture& texture, std::nullptr_t);
bool operator== (std::nullptr_t, const Surface& surface);

class Sprite {
	Surface spriteSheet;
	Texture texture;

public:
	Sprite() = default;
	Sprite(Sprite&& sprite) noexcept = default; 
	Sprite(const Sprite& sprite);

	Sprite(const std::string& path);
	explicit Sprite(const Surface& s);
	explicit Sprite(Surface&& s);
	Sprite(SDL_Surface* s);

	const Surface& getSpriteSheet() const;
	const Texture& getTexture() const;

	Surface& getSpriteSheet();
	Texture& getTexture();

	void setSpriteSheet(const Surface& s);

	void updateTexture();

	SDL_Point size() const;

	bool empty() const;

	void render(SDL_Rect position) const;

	Sprite& operator= (const Sprite& sprite);

	Sprite& operator= (Sprite&& sprite) noexcept;

	friend void swap(Sprite& lhs, Sprite& rhs) noexcept;
};

SDL_Point getSize(const Sprite& s);

inline Uint32& pixelAt(SDL_Surface* surface, int x, int y) {
	return (static_cast< Uint32* >(surface->pixels))[x + y * surface->w];
}

inline const Uint32& pixelAt(const SDL_Surface* surface, int x, int y) {
	return (static_cast< const Uint32* >(surface->pixels))[x + y * surface->w];
}

inline Uint32 getPixel(const SDL_Surface* surface, int x, int y) {
	return pixelAt(surface, x, y);
}

inline void setPixel(SDL_Surface* surface, int x, int y, Uint32 pixel) {
	pixelAt(surface, x, y) = pixel;
}

Uint32* begin(SDL_Surface* surface);
Uint32* end(SDL_Surface* surface);

Uint32* begin(Surface& surface);
Uint32* end(Surface& surface);

namespace std {
	template <>
	struct default_delete < SDL_Surface > {
		constexpr default_delete() noexcept = default;
		void operator()(SDL_Surface* ptr) const;
	};
	template <>
	struct default_delete < SDL_Texture > {
		constexpr default_delete() noexcept = default;
		void operator()(SDL_Texture* ptr) const;
	};
}
