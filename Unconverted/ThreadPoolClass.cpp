// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#include "StdMCF.hpp"
#include "ThreadPoolClass.hpp"
#include "xPooledThreadClass.hpp"
#include <climits>
using namespace MCF;

// 构造函数和析构函数。
ThreadPoolClass::ThreadPoolClass(std::size_t uMaxThreadCount) :
	xm_semJobsAvailable(0, LONG_MAX)
{
	xm_uLastJobID = 0;
	xm_bTerminateNow = false;

	xm_vecpPooledThreads.reserve(uMaxThreadCount);
	for(std::size_t i = 0; i < uMaxThreadCount; ++i){
		xPooledThreadClass *const pNewThread = new xPooledThreadClass(this);
		xm_vecpPooledThreads.emplace_back(pNewThread);
		pNewThread->Create(false);
	}

	xm_uFirstPendingJobID = 0;
}
ThreadPoolClass::~ThreadPoolClass(){
	xSHARED_EVENT hLastDone;

	xm_csLock.Enter();
		if(!xm_quePendingJobs.empty()){
			hLastDone = xm_quePendingJobs.back().second;
		}
	xm_csLock.Leave();

	if(hLastDone){
		::WaitForSingleObject(hLastDone, INFINITE);
	}

	xm_bTerminateNow = true;
	xm_semJobsAvailable.Post((long)xm_vecpPooledThreads.size());

	for(auto iter = xm_vecpPooledThreads.begin(); iter != xm_vecpPooledThreads.end(); ++iter){
		(*iter)->WaitAndRelease();
	}
}

// 其他非静态成员函数。
std::size_t ThreadPoolClass::KickInJob(std::function<void()> &&Job, bool bWaitTillDone){
	xSHARED_EVENT hDone(MakeSharedHandle<HANDLE, NULL>(::CreateEvent(nullptr, TRUE, FALSE, nullptr), &::CloseHandle));

	xm_csLock.Enter();
		const std::size_t uJobIndex = ++xm_uLastJobID;
		xm_quePendingJobs.emplace(std::make_pair(std::move(Job), hDone));
		xm_mapDoneEvents.emplace(std::make_pair(uJobIndex, hDone));

		xm_semJobsAvailable.Post();
	xm_csLock.Leave();

	if(bWaitTillDone){
		::WaitForSingleObject(hDone, INFINITE);
	}

	return uJobIndex;
}
ThreadPoolClass::JOB_STATE ThreadPoolClass::QueryJobState(std::size_t uJobID) const {
	JOB_STATE eState;

	xm_csLock.Enter();
		if((xm_uFirstPendingJobID <= uJobID) && (uJobID < xm_uFirstPendingJobID + xm_quePendingJobs.size())){
			eState = JOB_PENDING;
		} else if(xm_mapDoneEvents.find(uJobID) != xm_mapDoneEvents.end()){
			eState = JOB_EXECUTING;
		} else {
			eState = JOB_DONE;
		}
	xm_csLock.Leave();

	return eState;
}

void ThreadPoolClass::WaitForSingle(std::size_t uJobID) const {
	WaitForSingleTimeout(uJobID, INFINITE);
}
bool ThreadPoolClass::WaitForSingleTimeout(std::size_t uJobID, DWORD dwMilliSeconds) const {
	xSHARED_EVENT hDone;

	xm_csLock.Enter();
		const auto iterEvents = xm_mapDoneEvents.find(uJobID);
		if(iterEvents != xm_mapDoneEvents.end()){
			hDone = iterEvents->second;
		}
	xm_csLock.Leave();

	if(hDone){
		return ::WaitForSingleObject(hDone, dwMilliSeconds) != WAIT_TIMEOUT;
	}
	return true;
}
void ThreadPoolClass::WaitForAny(const std::size_t *puJobIDs, std::size_t uJobCount) const {
	WaitForAnyTimeout(puJobIDs, uJobCount, INFINITE);
}
bool ThreadPoolClass::WaitForAnyTimeout(const std::size_t *puJobIDs, std::size_t uJobCount, DWORD dwMilliSeconds) const {
	std::vector<xSHARED_EVENT> vechDone;
	vechDone.reserve(uJobCount);

	xm_csLock.Enter();
		for(std::size_t i = 0; i < uJobCount; ++i){
			const auto iterEvents = xm_mapDoneEvents.find(puJobIDs[i]);
			if(iterEvents != xm_mapDoneEvents.end()){
				vechDone.emplace_back(iterEvents->second);
			}
		}
	xm_csLock.Leave();

	if(!vechDone.empty()){
		std::vector<HANDLE> vecHandles(vechDone.size());
		std::copy(vechDone.begin(), vechDone.end(), vecHandles.begin());
		return ::WaitForMultipleObjects((DWORD)vecHandles.size(), vecHandles.data(), FALSE, dwMilliSeconds) != WAIT_TIMEOUT;
	}
	return true;
}
void ThreadPoolClass::WaitForAll(const std::size_t *puJobIDs, std::size_t uJobCount) const {
	WaitForAllTimeout(puJobIDs, uJobCount, INFINITE);
}
bool ThreadPoolClass::WaitForAllTimeout(const std::size_t *puJobIDs, std::size_t uJobCount, DWORD dwMilliSeconds) const {
	std::vector<xSHARED_EVENT> vechDone;
	vechDone.reserve(uJobCount);

	xm_csLock.Enter();
		for(std::size_t i = 0; i < uJobCount; ++i){
			const auto iterEvents = xm_mapDoneEvents.find(puJobIDs[i]);
			if(iterEvents != xm_mapDoneEvents.end()){
				vechDone.emplace_back(iterEvents->second);
			}
		}
	xm_csLock.Leave();

	if(!vechDone.empty()){
		std::vector<HANDLE> vecHandles(vechDone.size());
		std::copy(vechDone.begin(), vechDone.end(), vecHandles.begin());
		return ::WaitForMultipleObjects((DWORD)vecHandles.size(), vecHandles.data(), TRUE, dwMilliSeconds) != WAIT_TIMEOUT;
	}
	return true;
}
