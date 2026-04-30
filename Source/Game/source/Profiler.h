#pragma once

#include <unordered_map>
#include <string_view>

class Profiler
{
public:
	static Profiler* Get();

	void GetProfile() const;
private:
	friend class ScopedProfiler;

	Profiler() = default;

	struct CallRecord
	{
		const char* id;
		unsigned __int64 elapsedCycles;
	};

	struct Profile
	{
		const char* id;
		unsigned __int64 elapsedCycles;
		unsigned __int64 callCount;
	};

	void Submit(CallRecord aCallRecord);

	std::unordered_map<std::string_view, Profile> myProfiles;
};

class ScopedProfiler
{
public:
	ScopedProfiler(const char* aId);
	~ScopedProfiler();

	void Stop();

	void Ignore();
private:
	const char* myId;
	unsigned __int64 myStartCount;
};