/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#ifndef COMMON_CLOCK_H
#define COMMON_CLOCK_H

#include "common/typedefs.h"


namespace Common {

/**
 * A class for tracking the passage of time
 */
class Clock {
public:
	Clock();

	/**
	 * Returns the time (in ms) that has passed between the last two calls to Tick()
	 *
	 * Tick();            --+
	 * // Stuff             +-- delta time 
	 * Tick();            --+
	 *
	 * @return    The delta time in ms 
	 */
	double DeltaTime() const { return m_deltaTime; }

	/**
	 * Call this once every game loop. This will calculate and update m_deltaTime
	 */
	void Tick();
	/**
	 * Starts the clock. If the clock is not started, DeltaTime() will be zero.
	 *
	 * @return        
	 */
	void Start();
	/**
	 * Stops the clock. While the clock is stopped, DeltaTime() will be zero.
	 */
	void Stop();

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
