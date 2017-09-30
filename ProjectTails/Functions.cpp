#include "Functions.h"

SDL_Point& operator+= (SDL_Point& lhs, const SDL_Point& rhs) {
	return invoke_all_members([](auto& l, const auto& r) { return l += r; }, lhs, rhs);
}
SDL_Rect& operator+= (SDL_Rect& lhs, const SDL_Rect& rhs) {
	return invoke_all_members([](auto& l, const auto& r) { return l += r; }, lhs, rhs);
}

SDL_Point operator+ (SDL_Point lhs, const SDL_Point& rhs) {
	return (lhs += rhs);
}
SDL_Rect operator+ (SDL_Rect lhs, const SDL_Rect& rhs) {
	return (lhs += rhs);
}

SDL_Point& operator-= (SDL_Point& lhs, const SDL_Point& rhs) {
	return invoke_all_members([](auto& l, const auto& r) { return l -= r; }, lhs, rhs);
}
SDL_Rect& operator-= (SDL_Rect& lhs, const SDL_Rect& rhs) {
	return invoke_all_members([](auto& l, const auto& r) { return l -= r; }, lhs, rhs);
}

SDL_Point operator- (SDL_Point lhs, const SDL_Point& rhs) {
	return (lhs -= rhs);
}
SDL_Rect operator- (SDL_Rect lhs, const SDL_Rect& rhs) {
	return (lhs -= rhs);
}

SDL_Point getXY(const SDL_Rect& r) {
	return { r.x, r.y };
}

SDL_Rect getRelativePosition(const SDL_Rect& a, const SDL_Rect& b) {
	return SDL_Rect { a.x - b.x, a.y - b.y, a.w, a.h };
}

SDL_Point rotate(const SDL_Point& p, int degrees) {
	const double rads = degrees * M_PI / 180.0;
	return SDL_Point { int(p.x * std::cos(rads) - p.y * std::sin(rads)), int(p.x * std::sin(rads) + p.y * std::cos(rads)) };
}

std::pair < double, double > rotate(const std::pair < double, double >& p, double degrees) {
	const double rads = degrees * M_PI / 180.0;
	return { p.first * std::cos(rads) - p.second * std::sin(rads), p.first * std::sin(rads) + p.second * std::cos(rads) };
}
