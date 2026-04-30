#include <iostream>
#include <vector>
#include <unordered_map>
#include <string_view>

#include <windows.h>
#include "Profiler.h"

#include <algorithm>

Profiler* Profiler::Get()
{
	static Profiler profiler;
	return &profiler;
}

void Profiler::GetProfile() const
{
	unsigned __int64 countsPerSecond{ 0 };
	QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&countsPerSecond));

	constexpr unsigned __int64 millisecondsPerSecond = 1000;

	std::vector<Profile> profiles;

	for (const auto& kvp : myProfiles)
	{
		profiles.push_back(kvp.second);		
	}

	std::sort(profiles.begin(), profiles.end(), [countsPerSecond, millisecondsPerSecond](const Profile& aLhs, const Profile& aRhs) {
		const double lhsElapsedMs = static_cast<double>(aLhs.elapsedCycles) * static_cast<double>(millisecondsPerSecond) / static_cast<double>(countsPerSecond);
		const double rhsElapsedMs = static_cast<double>(aRhs.elapsedCycles) * static_cast<double>(millisecondsPerSecond) / static_cast<double>(countsPerSecond);

		return lhsElapsedMs > rhsElapsedMs;
		});

	std::cout << '\n' << "PROFILE SORTED BY ELAPSED MS" << '\n' << '\n';

	for (const auto& profile : profiles)
	{
		const double elapsedMs = static_cast<double>(profile.elapsedCycles) * static_cast<double>(millisecondsPerSecond) / static_cast<double>(countsPerSecond);

		std::cout << profile.id << ' ' << elapsedMs << "ms elapsed" << '\n';
	}

	std::sort(profiles.begin(), profiles.end(), [countsPerSecond, millisecondsPerSecond](const Profile& aLhs, const Profile& aRhs){
			const double lhsElapsedMs = static_cast<double>(aLhs.elapsedCycles) * static_cast<double>(millisecondsPerSecond) / static_cast<double>(countsPerSecond);
			const double lhsAverageMs = lhsElapsedMs / static_cast<double>(aLhs.callCount);
			const double rhsElapsedMs = static_cast<double>(aRhs.elapsedCycles) * static_cast<double>(millisecondsPerSecond) / static_cast<double>(countsPerSecond);
			const double rhsAverageMs = rhsElapsedMs / static_cast<double>(aRhs.callCount);

			return lhsAverageMs > rhsAverageMs;
		});

	std::cout << '\n' << "PROFILE SORTED BY AVG MS" << '\n' << '\n';

	for (const auto& profile : profiles)
	{
		const double elapsedMs = static_cast<double>(profile.elapsedCycles) * static_cast<double>(millisecondsPerSecond) / static_cast<double>(countsPerSecond);
		const double averageMs = elapsedMs / static_cast<double>(profile.callCount);

		std::cout << profile.id << ' ' << averageMs << "ms avg over " << profile.callCount << " call(s) " << '\n';
	}
}

void Profiler::Submit(CallRecord aCallRecord)
{
	myProfiles[aCallRecord.id].id = aCallRecord.id;
	myProfiles[aCallRecord.id].elapsedCycles += aCallRecord.elapsedCycles;
	myProfiles[aCallRecord.id].callCount++;
}

ScopedProfiler::ScopedProfiler(const char* aFunctionName)
	:
	myId(aFunctionName),
	myStartCount(0)
{
	QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&myStartCount));
}

ScopedProfiler::~ScopedProfiler()
{
	if (myStartCount)
	{
		unsigned __int64 stopCount{ 0 };
		QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&stopCount));

		Profiler::CallRecord result
		{
			myId,
			stopCount - myStartCount
		};

		Profiler* profiler = Profiler::Get();
		profiler->Submit(result);

		myStartCount = 0;
	}
}

void ScopedProfiler::Stop()
{
	if (myStartCount)
	{
		unsigned __int64 stopCount{ 0 };
		QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&stopCount));

		Profiler::CallRecord result
		{
			myId,
			stopCount - myStartCount
		};

		Profiler* profiler = Profiler::Get();
		profiler->Submit(result);

		myStartCount = 0;
	}
}

void ScopedProfiler::Ignore()
{
	myStartCount = 0;
}
