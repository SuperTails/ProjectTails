#include <SDL.h>
#include "Timer.h"

Timer::Timer(const Timer::DurationType& d) :
	duration(d),
	lastTick(time),
	timing(false) {
	
}

Timer::Timer(int d) :
	Timer(DurationType{ d }) {

}

bool Timer::update() {
	if (timeRemaining().count() == 0) {
		lastTick += duration.count() * ((time - lastTick) / duration.count());
		return true;
	}
	else {
		return false;
	}
}

void Timer::reset() {
	lastTick = time;
}

void Timer::start() {
	timing = true;
	lastTick = time;
}

void Timer::stop() {
	timing = false;
}

bool Timer::isTiming() const {
	return timing;
}

Timer::DurationType Timer::timeRemaining() const {
	if (timing) {
		if (time >= lastTick + duration.count()) { 
			return DurationType{ 0 };
		}
		else {
			return DurationType{ time - lastTick };
		}
	}
	else {
		return duration;
	}
}

Timer::DurationType Timer::getFrameTime() {
	return DurationType{ (time - lastTime) % (1000 / 60) };
}

void Timer::updateFrameTime() {
	lastTime = time;
	time = SDL_GetTicks();
}

std::uint32_t Timer::time = 0;
std::uint32_t Timer::lastTime = 0;

void swap(Timer& a, Timer& b) noexcept {
	using std::swap;

	swap(a.duration, b.duration);
	swap(a.lastTick, b.lastTick);
	swap(a.timing, b.timing);
}
