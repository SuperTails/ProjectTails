#pragma once
#include <SDL.h>

struct Vector2 {
	Vector2() = default;
	Vector2(const Vector2&) = default;
	Vector2(Vector2&&) = default;

	Vector2& operator= (const Vector2&) = default;

	Vector2(double x_, double y_);

	explicit Vector2(SDL_Point);
	
	explicit operator SDL_Point() const;

	double x, y;
};

typedef Vector2 Point;

struct Rect {
	Rect() = default;
	Rect(const Rect&) = default;
	Rect(Rect&&) = default;

	Rect& operator= (const Rect&) = default;

	Rect(double x_, double y_, double w_, double h_);

	explicit Rect(SDL_Rect rect);

	explicit operator SDL_Rect() const;

	double x{0}, y{0}, w{0}, h{0};
};
