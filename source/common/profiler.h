/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#ifndef COMMON_PROFILER_H
#define COMMON_PROFILER_H

#include "common/typedefs.h"

#include <unordered_map>
#include <string>
#include <vector>

namespace Common {

struct TimeSegment {
	int64 StartCount;
	int64 EndCount;
	double DeltaTime;
};

class Profiler {
public:
	Profiler();

private:
	double m_milliSecondsPerCount;
	std::unordered_map<std::string, int64> m_startCounts;
	std::unordered_map<std::string, std::vector<TimeSegment> > m_events;

public:
	void StartEvent(const std::string eventName);
	void EndEvent(const std::string eventName);
	double GetEventTotalTime(const std::string eventName);
	std::unordered_map<std::string, double> GetAllEventTimes();
};

} // End of namespace Common

#endif