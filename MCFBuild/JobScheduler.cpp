// Copyleft 2013, LH_Mouse. All wrongs reserved.

#include "General.hpp"
#include "JobScheduler.hpp"
#include <memory>
#include <algorithm>
#include "UniqueHandle.hpp"
using namespace MCFBuild;

DWORD WINAPI JobScheduler::xThreadProc(LPVOID pParam){
	((JobScheduler *)pParam)->xThreadJobProc();
	return 0;
}

JobScheduler::JobScheduler(){
	::InitializeCriticalSectionAndSpinCount(&xm_csLock, 0x400);
}
JobScheduler::~JobScheduler(){
	::DeleteCriticalSection(&xm_csLock);
}

void JobScheduler::PushJob(JobScheduler::JOB &&fnJobProc){
	xm_lstJobs.emplace_back(std::move(fnJobProc));
}

void JobScheduler::xThreadJobProc(){
	const std::size_t uThreadIndex = (std::size_t)(std::find(xm_vecThreadIDs.cbegin(), xm_vecThreadIDs.cend(), ::GetCurrentThreadId()) - xm_vecThreadIDs.cbegin() + 1);

	try {
		std::unique_ptr<JOB> pJob;

		for(;;){
			{
				LOCK_THROUGH(xm_csLock);

				if(pJob){
					++xm_uCountCompleted;
					xPrintStatus(false);
					pJob.reset();
				}
				if(!xm_pCaughtException && !xm_lstJobs.empty()){
					pJob.reset(new JOB(std::move(xm_lstJobs.front())));
					xm_lstJobs.pop_front();
				}
			}

			if(!pJob){
				break;
			}
			(*pJob)(uThreadIndex);
		}
	} catch(Exception &e){
		LOCK_THROUGH(xm_csLock);

		if(!xm_pCaughtException){
			xm_pCaughtException.reset(new Exception(std::move(e)));
		}
	}
}
void JobScheduler::xPrintStatus(bool bStartsNewLine) const {
	int nDigitCount = 1;
	for(std::size_t i = xm_uCountTotal; i > 9; i /= 10){
		++nDigitCount;
	}

	static const std::size_t PROGRESS_CHAR_COUNT = 40;

	const auto ulPercent = (unsigned long)(xm_uCountCompleted * 100 / xm_uCountTotal);
	wchar_t wchProgress[PROGRESS_CHAR_COUNT + 1];
	wchar_t *const pwchProgEnd = wchProgress + ulPercent * PROGRESS_CHAR_COUNT / 100;
	std::fill(wchProgress, pwchProgEnd, L'>');
	std::fill(pwchProgEnd, wchProgress + PROGRESS_CHAR_COUNT, L'=');
	wchProgress[PROGRESS_CHAR_COUNT] = 0;
	Status(
		L"%ls%*lu / %lu [%ls] %3lu%%     %lc",
		xm_pwszPrefix,
		nDigitCount,
		(unsigned long)xm_uCountCompleted,
		(unsigned long)xm_uCountTotal,
		wchProgress,
		ulPercent,
		bStartsNewLine ? L'\n' : L' '
	);
}

void JobScheduler::Commit(std::size_t uProcessCount, const wchar_t *pwszPrefix){
	xm_uCountTotal = xm_lstJobs.size();
	if(xm_uCountTotal == 0){
		return;
	}
	xm_uCountCompleted = 0;
	xm_pwszPrefix = (pwszPrefix == nullptr) ? L"" : pwszPrefix;

	xPrintStatus(false);

	if(uProcessCount < 1){
		uProcessCount = 1;
	}

	struct ThreadCloser {
		constexpr HANDLE operator()(){
			return NULL;
		}
		void operator()(HANDLE hObj){
			::CloseHandle(hObj);
		}
	};
	typedef MCF::UniqueHandle<HANDLE, ThreadCloser> UniqueThreadHandle;

	std::vector<UniqueThreadHandle> vecNewThreads;
	std::vector<HANDLE> vecNakedHandles;

	for(std::size_t i = 0; i < uProcessCount; ++i){
		DWORD dwThreadID;
		const HANDLE hThread = ::CreateThread(nullptr, 0, &xThreadProc, this, CREATE_SUSPENDED, &dwThreadID);
		if(hThread == NULL){
			throw Exception{ERROR_PROCESS_ABORTED, L"创建线程失败。"};
		}
		vecNewThreads.push_back(UniqueThreadHandle(hThread));
		vecNakedHandles.push_back(hThread);

		xm_vecThreadIDs.push_back(dwThreadID);
	}
	std::sort(xm_vecThreadIDs.begin(), xm_vecThreadIDs.end());

	for(std::size_t i = 0; i < uProcessCount; ++i){
		::ResumeThread(vecNakedHandles[i]);
	}
	while(::WaitForMultipleObjects(vecNakedHandles.size(), vecNakedHandles.data(), TRUE, 15) == WAIT_TIMEOUT){
		LOCK_THROUGH(xm_csLock);

		xPrintStatus(false);
	}
	xPrintStatus(true);

	if(xm_pCaughtException){
		throw *xm_pCaughtException;
	}
}
