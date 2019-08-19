#pragma once
#include <cstddef>
#include <chrono>
#include "Miscellaneous.h"

class Timer {
public:
	typedef std::chrono::milliseconds DurationType;

	Timer(const DurationType& d) noexcept;
	
	explicit Timer(int d) noexcept;

	Timer() noexcept = default;
	Timer(const Timer&) noexcept = default;
	Timer(Timer&&) noexcept = default;

	Timer& operator= (const Timer& rhs) noexcept = default;
	Timer& operator= (int) noexcept = delete;
	
	bool update();

	void reset();

	void start();
	void stop();

	bool isTiming() const;

	DurationType timeRemaining() const;

	DurationType duration;

	static DurationType getFrameTime();

	static DurationType getTime();

	static void updateFrameTime();

	static bool slowMotion;
	static bool paused;

	static void frameAdvance();

	friend void swap(Timer& a, Timer& b) noexcept;
private:
	std::uint32_t lastTick;

	bool timing;

	static bool singleFrame;

	static std::uint32_t time;
	static std::uint32_t lastTime;
};
