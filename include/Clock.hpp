#pragma once
#include "PCH.hpp"
#include "Time.hpp"

class Clock
{
public:

	U64 now()
	{
		auto t = std::chrono::time_point_cast<std::chrono::microseconds>(clock.now());
		auto t2 = t.time_since_epoch();
		return t2.count();
	}

	Time time()
	{
		Time t;
		t.setMicroSeconds(now());
		return t;
	}

private:
	std::chrono::high_resolution_clock clock;
};