/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013
 */

#include "common/profiler.h"

#include "common/halfling_sys.h"

#include <stdexcept>


namespace Common {

Profiler::Profiler()
	: m_milliSecondsPerCount(0.0) {
	int64 countsPerSec;
	QueryPerformanceFrequency((LARGE_INTEGER *)&countsPerSec);
	m_milliSecondsPerCount = 1000.0 / (double)countsPerSec;
}

void Profiler::StartEvent(const std::string eventName) {
	// Make sure the event isn't already in progress
	auto iter = m_startCounts.find(eventName);
	AssertMsg(iter == m_startCounts.end() || iter->second == 0, L"An event with that name has already been started");

	int64 currCount;
	QueryPerformanceCounter((LARGE_INTEGER *)&currCount);

	m_startCounts[eventName] = currCount;
}

void Profiler::EndEvent(const std::string eventName) {
	// Make sure the event exists
	auto iter = m_startCounts.find(eventName);
	AssertMsg(iter != m_startCounts.end() && iter->second != 0, L"No event with that name exists");

	int64 startCount = m_startCounts[eventName];

	int64 currCount;
	QueryPerformanceCounter((LARGE_INTEGER *)&currCount);

	m_events[eventName].push_back({startCount, currCount, (double)(currCount - startCount) * m_milliSecondsPerCount});
	m_startCounts[eventName] = 0;
}

double Profiler::GetEventTotalTime(const std::string eventName) {
	// Make sure the event exists
	auto iter = m_events.find(eventName);
	AssertMsg(iter != m_events.end(), L"No event with that name exists");

	double accumulatedTime = 0;
	for (auto listIter = iter->second.begin(); listIter != iter->second.end(); ++listIter) {
		accumulatedTime += listIter->DeltaTime;
	}

	return accumulatedTime;
}

std::unordered_map<std::string, double> Profiler::GetAllEventTimes() {
	std::unordered_map<std::string, double> outputMap;

	for (auto iter = m_events.begin(); iter != m_events.end(); ++iter) {
		double accumulatedTime = 0;
		for (auto listIter = iter->second.begin(); listIter != iter->second.end(); ++listIter) {
			accumulatedTime += listIter->DeltaTime;
		}

		outputMap[iter->first] = accumulatedTime;
	}

	return outputMap;
}

} // End of namespace Common
