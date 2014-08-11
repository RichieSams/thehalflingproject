/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#pragma once

#include "common/typedefs.h"


namespace Engine {

class Timer {
public:
	Timer(int64 performanceCounterFreq = 0);

private:
	double m_milliSecondsPerCount;

	int64 m_startCount;
	int64 m_accumulatedTicks;

	bool m_isRunning;

public:
	void Start();
	void Stop();
	double GetTime();
};

} // End of namespace Engine
