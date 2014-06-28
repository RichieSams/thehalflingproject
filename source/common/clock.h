/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#pragma once

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

	/**
	 * Returns the total time that has passed since the Clock was started. This subtracts
	 * off any time when the Clock was stopped.
	 *
	 * @return    The total time the Clock has been running
	 */
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
