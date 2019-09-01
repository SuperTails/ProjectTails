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

SDL_Point operator+ (SDL_Point lhs, const SDL_Point& rhs) {
	return (lhs += rhs);
}
SDL_Rect operator+ (SDL_Rect lhs, const SDL_Rect& rhs) {
	return (lhs += rhs);
}
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


SDL_Point operator- (SDL_Point lhs, const SDL_Point& rhs) {
	return (lhs -= rhs);
}
SDL_Rect operator- (SDL_Rect lhs, const SDL_Rect& rhs) {
	return (lhs -= rhs);
}
SDL_Rect operator- (SDL_Rect lhs, const SDL_Point& rhs) {
	return (lhs -= rhs);
}

SDL_Point getXY(const SDL_Rect& r) {
	return { r.x, r.y };
}

SDL_Rect getRelativePosition(const SDL_Rect& a, const SDL_Rect& b) {
	return SDL_Rect { a.x - b.x, a.y - b.y, a.w, a.h };
}

SDL_Point rotate90(int amount, SDL_Point point, SDL_Point center) {
	point -= center;
	switch (((amount % 4) + 4) % 4) {
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

SDL_Rect rotate90(int amount, SDL_Rect rect, SDL_Point center) {
	rect.x -= center.x;
	rect.y -= center.y;
	switch (((amount % 4) + 4) % 4) {
	case 0:
		return { center.x + rect.x, center.y + rect.y, rect.w, rect.h };
	case 1:
		return { center.x - rect.y - rect.h, center.y + rect.x, rect.h, rect.w };
	case 2:
		return { center.x - rect.x, center.y - rect.y - rect.h, rect.w, rect.h };
	case 3:
		return { center.x + rect.y, center.y - rect.x - rect.w, rect.h, rect.w };
	}
}

Rect rotate90(int amount, Rect rect, Point center) {
	rect.x -= center.x;
	rect.y -= center.y;
	switch (((amount % 4) + 4) % 4) {
	case 0:
		return { center.x + rect.x, center.y + rect.y, rect.w, rect.h };
	case 1:
		return { center.x - rect.y - rect.h, center.y + rect.x, rect.h, rect.w };
	case 2:
		return { center.x - rect.x, center.y - rect.y - rect.h, rect.w, rect.h };
	case 3:
		return { center.x + rect.y, center.y - rect.x - rect.w, rect.h, rect.w };
	}
}

SDL_Point rotate(const SDL_Point& p, int degrees) {
	const double rads = degrees * M_PI / 180.0;
	return SDL_Point { int(p.x * std::cos(rads) - p.y * std::sin(rads)), int(p.x * std::sin(rads) + p.y * std::cos(rads)) };
}

std::pair < double, double > rotate(const std::pair < double, double >& p, double degrees) {
	const double rads = degrees * M_PI / 180.0;
	return { p.first * std::cos(rads) - p.second * std::sin(rads), p.first * std::sin(rads) + p.second * std::cos(rads) };
}
