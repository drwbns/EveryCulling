#pragma once

#include "EveryCullingCore.h"

#include <unordered_map>
#include <string_view>
#include <chrono>
#include <atomic>

namespace culling
{
	class EveryCulling;
	class EveryCullingProfiler
	{
		friend class EveryCulling;

	public:

		struct ProfilingData
		{
			std::chrono::steady_clock::time_point mStartTime;
			std::chrono::steady_clock::time_point mEndTime;
			double mElapsedTime;

		};

	private:

		static thread_local bool IsLocalThreadRecordProfilingData;
		static std::atomic<bool> IsProfilingDataRecordByOtherThread;

		std::unordered_map<std::string_view, ProfilingData> mProfilingDatas;
		void SetStartTime(const char* const cullingModuleName);
		void SetEndTime(const char* const cullingModuleName);

	public:

		EveryCullingProfiler();

		/// <summary>
		/// Zeroes every recorded time, so a module that does not run this frame
		/// reads as zero rather than keeping whatever it last cost.
		///
		/// Without this a disabled module still shows its old number, which reads
		/// as though it were still running and makes a comparison between two
		/// culling modes wrong in the direction that is hardest to notice.
		/// </summary>
		void ResetProfilingDatas();

		double GetElapsedTime(const char* const cullingModuleName);
		const std::unordered_map<std::string_view, ProfilingData>& GetProfilingDatas() const;
	};
}


