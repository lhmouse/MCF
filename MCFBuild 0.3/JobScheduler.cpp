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
	::InitializeCriticalSectionAndSpinCount(&x_csLock, 0x400);
}
JobScheduler::~JobScheduler(){
	::DeleteCriticalSection(&x_csLock);
}

void JobScheduler::PushJob(JobScheduler::JOB &&fnJobProc){
	x_lstJobs.emplace_back(std::move(fnJobProc));
}

void JobScheduler::xThreadJobProc(){
	const std::size_t uThreadIndex = (std::size_t)(std::find(x_vecThreadIDs.cbegin(), x_vecThreadIDs.cend(), ::GetCurrentThreadId()) - x_vecThreadIDs.cbegin() + 1);

	try {
		std::unique_ptr<JOB> pJob;

		for(;;){
			{
				LOCK_THROUGH(x_csLock);

				if(pJob){
					++x_uCountCompleted;
					xPrintStatus(false);
					pJob.reset();
				}
				if(!x_pCaughtException && !x_lstJobs.empty()){
					pJob.reset(new JOB(std::move(x_lstJobs.front())));
					x_lstJobs.pop_front();
				}
			}

			if(!pJob){
				break;
			}
			(*pJob)(uThreadIndex);
		}
	} catch(Exception &e){
		LOCK_THROUGH(x_csLock);

		if(!x_pCaughtException){
			x_pCaughtException.reset(new Exception(std::move(e)));
		}
	}
}
void JobScheduler::xPrintStatus(bool bStartsNewLine) const {
	int nDigitCount = 1;
	for(std::size_t i = x_uCountTotal; i > 9; i /= 10){
		++nDigitCount;
	}

	static const std::size_t PROGRESS_CHAR_COUNT = 40;

	const auto ulPercent = (unsigned long)(x_uCountCompleted * 100 / x_uCountTotal);
	wchar_t wchProgress[PROGRESS_CHAR_COUNT + 1];
	wchar_t *const pwchProgEnd = wchProgress + ulPercent * PROGRESS_CHAR_COUNT / 100;
	std::fill(wchProgress, pwchProgEnd, L'>');
	std::fill(pwchProgEnd, wchProgress + PROGRESS_CHAR_COUNT, L'=');
	wchProgress[PROGRESS_CHAR_COUNT] = 0;
	Status(
		L"%ls%*lu / %lu [%ls] %3lu%%     %lc",
		x_pwszPrefix,
		nDigitCount,
		(unsigned long)x_uCountCompleted,
		(unsigned long)x_uCountTotal,
		wchProgress,
		ulPercent,
		bStartsNewLine ? L'\n' : L' '
	);
}

void JobScheduler::Commit(std::size_t uProcessCount, const wchar_t *pwszPrefix){
	x_uCountTotal = x_lstJobs.size();
	if(x_uCountTotal == 0){
		return;
	}
	x_uCountCompleted = 0;
	x_pwszPrefix = (pwszPrefix == nullptr) ? L"" : pwszPrefix;

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
	using UniqueThreadHandle = MCF::UniqueHandle<HANDLE, ThreadCloser>;

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

		x_vecThreadIDs.push_back(dwThreadID);
	}
	std::sort(x_vecThreadIDs.begin(), x_vecThreadIDs.end());

	for(std::size_t i = 0; i < uProcessCount; ++i){
		::ResumeThread(vecNakedHandles[i]);
	}
	while(::WaitForMultipleObjects(vecNakedHandles.size(), vecNakedHandles.data(), TRUE, 15) == WAIT_TIMEOUT){
		LOCK_THROUGH(x_csLock);

		xPrintStatus(false);
	}
	xPrintStatus(true);

	if(x_pCaughtException){
		throw *x_pCaughtException;
	}
}
