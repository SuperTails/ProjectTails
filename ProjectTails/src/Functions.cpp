#include "Functions.h"

bool operator== (SDL_Point lhs, SDL_Point rhs) {
	return lhs.x == rhs.x && lhs.y == rhs.y;
}

bool operator!= (SDL_Point lhs, SDL_Point rhs) {
	return !(lhs == rhs);
}

double mod(double a, double b) {
	return a - b * floor(a / b);
}

SDL_Point& operator+= (SDL_Point& lhs, const SDL_Point& rhs) {
	return invoke_all_members([](auto& l, const auto& r) { return l += r; }, lhs, rhs);
}
SDL_Rect& operator+= (SDL_Rect& lhs, const SDL_Rect& rhs) {
	return invoke_all_members([](auto& l, const auto& r) { return l += r; }, lhs, rhs);
}
SDL_Rect& operator+= (SDL_Rect& lhs, const SDL_Point& rhs) {
	return invoke_all_members([](auto& l, const auto& r) { return l += r; }, lhs, rhs);
}
Vector2&  operator+= (Vector2&  lhs, const Vector2& rhs) {
	return invoke_all_members([](auto& l, const auto& r) { return l += r; }, lhs, rhs);
}

#define OP_MACRO(TYPE, OP)                      \
TYPE operator OP (TYPE lhs, const TYPE &rhs) { \
	return (lhs OP##= rhs);                 \
}

OP_MACRO(SDL_Point, +)
OP_MACRO(SDL_Rect, +)
OP_MACRO(Vector2, +)

SDL_Rect operator+ (SDL_Rect lhs, const SDL_Point& rhs) {
	return (lhs += rhs);
}

SDL_Point& operator-= (SDL_Point& lhs, const SDL_Point& rhs) {
	return invoke_all_members([](auto& l, const auto& r) { return l -= r; }, lhs, rhs);
}
SDL_Rect& operator-= (SDL_Rect& lhs, const SDL_Rect& rhs) {
	return invoke_all_members([](auto& l, const auto& r) { return l -= r; }, lhs, rhs);
}
SDL_Rect& operator-= (SDL_Rect& lhs, const SDL_Point& rhs) {
	return invoke_all_members([](auto& l, const auto& r) { return l -= r; }, lhs, rhs);
}
Vector2&  operator-= (Vector2& lhs, const Vector2& rhs) {
	return invoke_all_members([](auto& l, const auto& r) { return l -= r; }, lhs, rhs);
}

OP_MACRO(SDL_Point, -)
OP_MACRO(SDL_Rect, -)
OP_MACRO(Vector2, -)

SDL_Rect operator- (SDL_Rect lhs, const SDL_Point& rhs) {
	return (lhs -= rhs);
}

#undef OP_MACRO

SDL_Point getXY(const SDL_Rect& r) {
	return { r.x, r.y };
}

SDL_Rect getRelativePosition(const SDL_Rect& a, const SDL_Rect& b) {
	return SDL_Rect { a.x - b.x, a.y - b.y, a.w, a.h };
}

SDL_Point rotate90(int amount, SDL_Point point, SDL_Point center) {
	point -= center;
	switch (wrap(amount, 4)) {
	case 0:
		return SDL_Point{ point.x, point.y }   + center;
	case 1:
		return SDL_Point{ -point.y, point.x }  + center;
	case 2:
		return SDL_Point{ -point.x, -point.y } + center;
	case 3:
		return SDL_Point{ point.y, -point.x }  + center;
	}
}


Point rotate90(int amount, Point point, Point center) {
	point -= center;
	switch (wrap(amount, 4)) {
	case 0:
		return Point{ point.x, point.y }   + center;
	case 1:
		return Point{ -point.y, point.x }  + center;
	case 2:
		return Point{ -point.x, -point.y } + center;
	case 3:
		return Point{ point.y, -point.x }  + center;
	}
}

SDL_Rect rotate90(int amount, SDL_Rect rect, SDL_Point center) {
	SDL_Point corner1{ rect.x, rect.y };
	SDL_Point corner2 = corner1 + SDL_Point{ rect.w, rect.h };
	corner1 = rotate90(amount, corner1, center);
	corner2 = rotate90(amount, corner2, center);

	SDL_Point topLeft{ std::min(corner1.x, corner2.x), std::min(corner1.y, corner2.y) };
	SDL_Point bottomRight{ std::max(corner1.x, corner2.x), std::max(corner1.y, corner2.y) };
	
	return SDL_Rect{ topLeft.x, topLeft.y, bottomRight.x - topLeft.x, bottomRight.y - topLeft.y };
	
}

Rect rotate90(int amount, Rect rect, Point center) {
	Point corner1{ rect.x, rect.y };
	Point corner2 = corner1 + Point{ rect.w, rect.h };
	corner1 = rotate90(amount, corner1, center);
	corner2 = rotate90(amount, corner2, center);

	Point topLeft{ std::min(corner1.x, corner2.x), std::min(corner1.y, corner2.y) };
	Point bottomRight{ std::max(corner1.x, corner2.x), std::max(corner1.y, corner2.y) };

	return Rect{ topLeft.x, topLeft.y, bottomRight.x - topLeft.x, bottomRight.y - topLeft.y };
}

SDL_Point rotate(const SDL_Point& p, int degrees) {
	const double rads = degrees * M_PI / 180.0;
	return SDL_Point { int(p.x * std::cos(rads) - p.y * std::sin(rads)), int(p.x * std::sin(rads) + p.y * std::cos(rads)) };
}

std::pair < double, double > rotate(const std::pair < double, double >& p, double degrees) {
	const double rads = degrees * M_PI / 180.0;
	return { p.first * std::cos(rads) - p.second * std::sin(rads), p.first * std::sin(rads) + p.second * std::cos(rads) };
}

int wrap(int x, int m) {
	return ((x % m) + m) % m;
}
