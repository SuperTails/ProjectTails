#pragma once
#include <SDL2/SDL.h>
#include <type_traits>
#include <functional>
#include <optional>

__attribute__((const)) bool operator== (SDL_Point lhs, SDL_Point rhs);

double mod(double a, double b);

template < typename F, typename T >
auto& invoke_all_members(F&& func, SDL_Point& lhs, const T& rhs);

template < typename F >
auto& invoke_all_members(F&& func, SDL_Point& lhs, const SDL_Point& rhs);

template < typename F, typename T >
auto& invoke_all_members(F&& func, SDL_Rect& lhs, const T& rhs);

template < typename F >
auto& invoke_all_members(F&& func, SDL_Rect& lhs, const SDL_Rect& rhs);

template < typename F >
auto& invoke_all_members(F&& func, SDL_Rect& lhs, const SDL_Point& rhs);

SDL_Point& operator+= (SDL_Point& lhs, const SDL_Point& rhs);
SDL_Rect&  operator+= (SDL_Rect& lhs , const SDL_Rect&  rhs);
SDL_Rect&  operator+= (SDL_Rect& lhs , const SDL_Point& rhs);

__attribute__((const)) SDL_Point operator+ (SDL_Point lhs, const SDL_Point& rhs);
__attribute__((const)) SDL_Rect operator+ (SDL_Rect lhs, const SDL_Rect& rhs);

SDL_Point& operator-= (SDL_Point& lhs, const SDL_Point& rhs);
SDL_Rect&  operator-= (SDL_Rect& lhs , const SDL_Rect&  rhs);
SDL_Rect&  operator-= (SDL_Rect& lhs , const SDL_Point& rhs);

__attribute__((const)) SDL_Point operator- (SDL_Point lhs, const SDL_Point& rhs);
__attribute__((const)) SDL_Rect  operator- (SDL_Rect lhs , const SDL_Rect&  rhs);
__attribute__((const)) SDL_Rect  operator- (SDL_Rect lhs , const SDL_Point& rhs);

template < typename T >
SDL_Point& operator*= (SDL_Point& lhs, const T& rhs);
template < typename T >
SDL_Rect& operator*= (SDL_Rect& lhs, const T& rhs);

template < typename T >
__attribute__((const)) SDL_Point operator* (SDL_Point lhs, const T& rhs);
template < typename T >
__attribute__((const)) SDL_Rect operator* (SDL_Rect lhs, const T& rhs);

template < typename T >
SDL_Point& operator/= (SDL_Point& lhs, const T& rhs);
template < typename T >
SDL_Rect& operator/= (SDL_Rect& lhs, const T& rhs);

template < typename T >
__attribute__((const)) SDL_Point operator/ (SDL_Point lhs, const T& rhs);
template < typename T >
__attribute__((const)) SDL_Rect operator/ (SDL_Rect lhs, const T& rhs);

__attribute__((const)) SDL_Point getXY(const SDL_Rect& r);

__attribute__((const)) SDL_Rect getRelativePosition(const SDL_Rect& a, const SDL_Rect& b);

__attribute__((const)) SDL_Point rotate(const SDL_Point& p, int degrees);

std::pair < double, double > rotate(const std::pair < double, double >& p, double degrees);

// Returns point rotated by 90 * amount degrees clockwise around center
__attribute__((const)) SDL_Point rotate90(int amount, SDL_Point point, const SDL_Point& center = { 0, 0 });

__attribute__((const)) SDL_Rect rotate90(int amount, SDL_Rect rect, const SDL_Point& center = { 0, 0 });

template < typename F, typename... Args >
auto applyOptional(F&& f, const std::optional<Args...>& t) ->
	std::optional< std::invoke_result_t< F, std::add_const_t< Args >... > >;

template < typename F, typename... Args >
std::optional < std::invoke_result_t< F, Args... > > applyOptional(F&& f, std::optional<Args...>& t);

#include <FunctionsImpl.h>
