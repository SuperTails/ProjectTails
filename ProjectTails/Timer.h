#pragma once
#include <cstddef>
#include <chrono>
#include "Miscellaneous.h"

class Timer {
public:
	typedef std::chrono::milliseconds DurationType;

	Timer(const DurationType& d);
	
	Timer(int d);

	Timer() = default;
	Timer(const Timer&) = default;
	Timer(Timer&&) = default;

	Timer& operator= (const Timer& rhs) = default;
	Timer& operator= (int) = delete;
	
	bool update();

	void reset();

	void start();
	void stop();

	bool isTiming() const;

	DurationType timeRemaining() const;

	DurationType duration;

	static DurationType getFrameTime();

	static void updateFrameTime();

	friend void swap(Timer& a, Timer& b) noexcept;
private:
	std::uint32_t lastTick;

	bool timing;

	static std::uint32_t time;
	static std::uint32_t lastTime;
};
