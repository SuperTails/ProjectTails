#include <SDL.h>
#include "Timer.h"
#include "InputComponent.h"

Timer::Timer(const Timer::DurationType& d) noexcept :
	duration(d),
	lastTick(time),
	timing(false) {
	
}

Timer::Timer(int d) noexcept :
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
		if (time - lastTick >= duration.count()) { 
			return DurationType{ 0 };
		}
		else {
			return DurationType{ duration.count() - (time - lastTick) };
		}
	}
	else {
		return duration;
	}
}

Timer::DurationType Timer::getFrameTime() {
	const auto& timeElapsed = (time - lastTime) % (1000 / 60);
	if (slowMotion) {
		return DurationType{ timeElapsed / 2 };
	}
	else if (paused) {
		return DurationType{ singleFrame ? (1000 / 60 / 2) : 0};
	}
	else {
		return DurationType{ timeElapsed };
	}
}

Timer::DurationType Timer::getTime() {
	return DurationType{ time };
}

void Timer::updateFrameTime() {
	lastTime = time;
	time = SDL_GetTicks();
	singleFrame = false;
}

void Timer::frameAdvance() {
	singleFrame = true;
}

std::uint32_t Timer::time = 0;
std::uint32_t Timer::lastTime = 0;

bool Timer::slowMotion = false;
bool Timer::paused = false;
bool Timer::singleFrame = false;

void swap(Timer& a, Timer& b) noexcept {
	using std::swap;

	swap(a.duration, b.duration);
	swap(a.lastTick, b.lastTick);
	swap(a.timing, b.timing);
}
