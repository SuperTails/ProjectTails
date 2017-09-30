#pragma once
#include <SDL.h>
#include <type_traits>

template < typename F, typename T >
auto& invoke_all_members(F&& func, SDL_Point& lhs, const T& rhs) {
	func(lhs.x, rhs);
	func(lhs.y, rhs);
	return lhs;
}

template < typename F >
auto& invoke_all_members(F&& func, SDL_Point& lhs, const SDL_Point& rhs) {
	func(lhs.x, rhs.x);
	func(lhs.y, rhs.y);
	return lhs;
}

template < typename F, typename T >
auto& invoke_all_members(F&& func, SDL_Rect& lhs, const T& rhs) {
	func(lhs.x, rhs);
	func(lhs.y, rhs);
	func(lhs.w, rhs);
	func(lhs.h, rhs);
	return lhs;
}

template < typename F >
auto& invoke_all_members(F&& func, SDL_Rect& lhs, const SDL_Rect& rhs) {
	func(lhs.x, rhs.x);
	func(lhs.y, rhs.y);
	func(lhs.w, rhs.w);
	func(lhs.h, rhs.h);
	return lhs;
}

template < typename T >
SDL_Point& operator*= (SDL_Point& lhs, const T& rhs) {
	static_assert(std::is_arithmetic<T>::value, "Right hand side of SDL_Point multiplication is not arithmetic type");
	return invoke_all_members([](auto& l, const auto& r) { return l *= r; }, lhs, rhs);
}
template < typename T >
SDL_Rect& operator*= (SDL_Rect& lhs, const T& rhs) {
	static_assert(std::is_arithmetic<T>::value, "Right hand side of SDL_Point multiplication is not arithmetic type");
	return invoke_all_members([](auto& l, const auto& r) { return l *= r; }, lhs, rhs);
}

template < typename T >
SDL_Point operator* (SDL_Point lhs, const T& rhs) {
	return (lhs *= rhs);
}
template < typename T >
SDL_Rect operator* (SDL_Rect lhs, const T& rhs) {
	return (lhs *= rhs);
}

template < typename T >
SDL_Point& operator/= (SDL_Point& lhs, const T& rhs) {
	static_assert(std::is_arithmetic_v<T>, "Right hand side of SDL_Point division is not arithmentic type");
	return invoke_all_members([](auto& l, const auto& r) { return l /= r; }, lhs, rhs);
}
template < typename T >
SDL_Rect& operator/= (SDL_Rect& lhs, const T& rhs) {
	static_assert(std::is_arithmetic_v<T>, "Right hand side of SDL_Point division is not arithmentic type");
	return invoke_all_members([](auto& l, const auto& r) { return l /= r; }, lhs, rhs);
}

template < typename T >
SDL_Point operator/ (SDL_Point lhs, const T& rhs) {
	return (lhs /= rhs);
}
template < typename T >
SDL_Rect operator/ (SDL_Rect lhs, const T& rhs) {
	return (lhs /= rhs);
}

