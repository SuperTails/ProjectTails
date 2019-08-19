#pragma once
#include <SDL.h>
#include <type_traits>
#include <functional>

template < typename F, typename T >
auto& invoke_all_members(F&& func, SDL_Point& lhs, const T& rhs);

template < typename F >
auto& invoke_all_members(F&& func, SDL_Point& lhs, const SDL_Point& rhs);

template < typename F, typename T >
auto& invoke_all_members(F&& func, SDL_Rect& lhs, const T& rhs);

template < typename F >
auto& invoke_all_members(F&& func, SDL_Rect& lhs, const SDL_Rect& rhs);

SDL_Point& operator+= (SDL_Point& lhs, const SDL_Point& rhs);
SDL_Rect& operator+= (SDL_Rect& lhs, const SDL_Rect& rhs);

SDL_Point operator+ (SDL_Point lhs, const SDL_Point& rhs);
SDL_Rect operator+ (SDL_Rect lhs, const SDL_Rect& rhs);

SDL_Point& operator-= (SDL_Point& lhs, const SDL_Point& rhs);
SDL_Rect& operator-= (SDL_Rect& lhs, const SDL_Rect& rhs);

SDL_Point operator- (SDL_Point lhs, const SDL_Point& rhs);
SDL_Rect operator- (SDL_Rect lhs, const SDL_Rect& rhs);

template < typename T >
SDL_Point& operator*= (SDL_Point& lhs, const T& rhs);
template < typename T >
SDL_Rect& operator*= (SDL_Rect& lhs, const T& rhs);

template < typename T >
SDL_Point operator* (SDL_Point lhs, const T& rhs);
template < typename T >
SDL_Rect operator* (SDL_Rect lhs, const T& rhs);

template < typename T >
SDL_Point& operator/= (SDL_Point& lhs, const T& rhs);
template < typename T >
SDL_Rect& operator/= (SDL_Rect& lhs, const T& rhs);

template < typename T >
SDL_Point operator/ (SDL_Point lhs, const T& rhs);
template < typename T >
SDL_Rect operator/ (SDL_Rect lhs, const T& rhs);

SDL_Point getXY(const SDL_Rect& r);

SDL_Rect getRelativePosition(const SDL_Rect& a, const SDL_Rect& b);

SDL_Point rotate(const SDL_Point& p, int degrees);

std::pair < double, double > rotate(const std::pair < double, double >& p, double degrees);

#include <FunctionsImpl.h>
