/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#include "timer.h"

#include <windows.h>


namespace Common {

Timer::Timer() 
		: m_milliSecondsPerCount(0.0f),
		  m_deltaTime(-1.0f),
		  m_baseTime(0),
		  m_pausedTime(0),
		  m_prevTime(0),
		  m_currTime(0),
		  m_isStopped(false) {
	int64 countsPerSec;
	QueryPerformanceFrequency((LARGE_INTEGER *)&countsPerSec);
	m_milliSecondsPerCount = 1000.0 / (double)countsPerSec;
}

void Timer::Tick() {
	if (m_isStopped) {
		m_deltaTime = 0.0f;
		return;
	}

	// Query for the time
	int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER *)&currTime);
	m_currTime = currTime;

	m_deltaTime = (m_currTime - m_prevTime) * m_milliSecondsPerCount;

	// Prepare for next frame.
	m_prevTime = m_currTime;

	// Force nonnegative.  The DXSDK's CDXUTTimer mentions that if the 
	// processor goes into a power save mode or we get shuffled to another
	// processor, then m_deltaTime can be negative.
	if (m_deltaTime < 0.0) {
		m_deltaTime = 0.0;
	}
}

void Timer::Start() {
	if (m_isStopped) {
		int64 startTime;
		QueryPerformanceCounter((LARGE_INTEGER *)&startTime);

		m_pausedTime += (startTime - m_stopTime);
		m_prevTime = startTime;

		m_stopTime = 0;
		m_isStopped = false;
	}
}

void Timer::Stop() {
	if (!m_isStopped) {
		int64 currTime;
		QueryPerformanceCounter((LARGE_INTEGER *)&currTime);

		m_stopTime = currTime;
		m_isStopped = true;
	}
}

void Timer::Reset() {
	int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER *)&currTime);

	m_baseTime = currTime;
	m_prevTime = currTime;
	m_stopTime = 0;
	m_isStopped = false;
}

double Timer::TotalTime() const {
	if (m_isStopped) {
		return ((m_stopTime - m_pausedTime) - m_baseTime) * m_milliSecondsPerCount;
	}

	return ((m_currTime - m_pausedTime) - m_baseTime) * m_milliSecondsPerCount;
}

} // End of Common
