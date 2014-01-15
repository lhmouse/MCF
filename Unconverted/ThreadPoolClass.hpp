// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_THREAD_POOL_CLASS_HPP__
#define __MCF_THREAD_POOL_CLASS_HPP__

#include "StdMCF.hpp"
#include <vector>
#include <queue>
#include "SharedHandleTemplate.hpp"

namespace MCF {
	class ThreadPoolClass final : NO_COPY_OR_ASSIGN {
	public:
		typedef enum {
			JOB_PENDING,
			JOB_EXECUTING,
			JOB_DONE
		} JOB_STATE;
	private:
		typedef SharedHandleTemplate<HANDLE, NULL, decltype(&::CloseHandle)> xSHARED_EVENT;

		class xPooledThreadClass;
	private:
		std::size_t xm_uLastJobID;
		volatile bool xm_bTerminateNow;
		std::vector<std::unique_ptr<xPooledThreadClass>> xm_vecpPooledThreads;
		Semaphore xm_semJobsAvailable;

		std::queue<std::pair<std::function<void()>, xSHARED_EVENT>> xm_quePendingJobs;
		std::size_t xm_uFirstPendingJobID;
		std::map<std::size_t, xSHARED_EVENT> xm_mapDoneEvents;

		CriticalSection xm_csLock;
	public:
		ThreadPoolClass(std::size_t uMaxThreadCount);
		~ThreadPoolClass();
	public:
		std::size_t KickInJob(std::function<void()> &&Job, bool bWaitTillDone);
		JOB_STATE QueryJobState(std::size_t uJobID) const;

		void WaitForSingle(std::size_t uJobID) const;
		bool WaitForSingleTimeout(std::size_t uJobID, DWORD dwMilliSeconds) const;
		void WaitForAny(const std::size_t *puJobIDs, std::size_t uJobCount) const;
		bool WaitForAnyTimeout(const std::size_t *puJobIDs, std::size_t uJobCount, DWORD dwMilliSeconds) const;
		void WaitForAll(const std::size_t *puJobIDs, std::size_t uJobCount) const;
		bool WaitForAllTimeout(const std::size_t *puJobIDs, std::size_t uJobCount, DWORD dwMilliSeconds) const;
	};
}

#endif
