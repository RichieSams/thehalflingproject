/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#include "common/timer.h"

#include "common/halfling_sys.h"


namespace Common {

Timer::Timer(int64 performanceCounterFreq) 
		: m_milliSecondsPerCount(0.0),
		  m_accumulatedTicks(0),
		  m_startCount(0),
		  m_isRunning(false) {
	if (performanceCounterFreq == 0)
		QueryPerformanceFrequency((LARGE_INTEGER *)&performanceCounterFreq);
	m_milliSecondsPerCount = 1000.0 / (double)performanceCounterFreq;
}

void Timer::Start() {
	QueryPerformanceCounter((LARGE_INTEGER *)&m_startCount);

	m_isRunning = true;
}

void Timer::Stop() {
	int64 currCount;
	QueryPerformanceCounter((LARGE_INTEGER *)&currCount);

	m_accumulatedTicks = currCount - m_startCount;

	m_isRunning = false;
}

double Timer::GetTime() {
	if (!m_isRunning)
		 return (double)m_accumulatedTicks * m_milliSecondsPerCount;

	int64 currCount;
	QueryPerformanceCounter((LARGE_INTEGER *)&currCount);

	return (double)(currCount - m_startCount) * m_milliSecondsPerCount;

}

} // End of namespace Common
