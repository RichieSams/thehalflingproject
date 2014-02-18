/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#ifndef COMMON_CLOCK_H
#define COMMON_CLOCK_H

#include "common/typedefs.h"


namespace Common {

class Clock {
public:
	Clock();

	double DeltaTime() const { return m_deltaTime; }

	void Tick();
	void Start();
	void Stop();
	void Reset();

	double TotalTime() const;

private:
	double m_milliSecondsPerCount;
	double m_deltaTime;

	int64 m_baseTime;
	int64 m_pausedTime;
	int64 m_stopTime;
	int64 m_prevTime;
	int64 m_currTime;

	bool m_isStopped;
};

} // End of Common

#endif
