#include "Shapes.h"

Vector2& operator+=(Vector2& lhs, Vector2 rhs) {
	lhs.x += rhs.x;
	lhs.y += rhs.y;
	return lhs;
}

Vector2& operator-=(Vector2& lhs, Vector2 rhs) {
	lhs.x -= rhs.x;
	lhs.y -= rhs.y;
	return lhs;
}

Vector2  operator+ (Vector2 lhs, Vector2 rhs) {
	return lhs += rhs;
}

Vector2  operator- (Vector2 lhs, Vector2 rhs) {
	return lhs -= rhs;
}

Vector2::Vector2(double x_, double y_) :
	x(x_),
	y(y_)
{

}

Vector2::Vector2(SDL_Point p) :
	x(p.x),
	y(p.y)
{

}

Vector2::operator SDL_Point() const {
	return SDL_Point{ int(x), int(y) };
}

Rect::Rect(double x_, double y_, double w_, double h_) :
	x(x_),
	y(y_),
	w(w_),
	h(h_)
{

}

Rect::Rect(SDL_Rect rect) :
	Rect(rect.x, rect.y, rect.w, rect.h)
{

}

Rect::operator SDL_Rect() const {
{
	return SDL_Rect{ int(x), int(y), int(w), int(h) }; };
}
