// MCF Build
// Copyleft 2014, LH_Mouse. All wrongs reserved.

#include "MCFBuild.hpp"
#include "JobScheduler.hpp"
#include "../MCF/Core/Exception.hpp"
#include "../MCF/Core/VVector.hpp"
#include "../MCF/Thread/Thread.hpp"
#include <exception>
using namespace MCFBuild;

// 构造函数和析构函数。
JobScheduler::JobScheduler()
	: xm_pcsLock(MCF::CriticalSection::Create())
{
}

// 其他非静态成员函数。
void JobScheduler::AddJob(std::function<void ()> fnJob){
	const auto vLock = xm_pcsLock->GetLock();
	xm_queJobs.emplace(std::move(fnJob));
}
void JobScheduler::CommitAll(std::size_t uThreadCount){
	MCF::VVector<std::shared_ptr<MCF::Thread>, 16u> vecThreads(uThreadCount);
	volatile bool bExitNow = false;
	std::exception_ptr pException;

	for(auto &pThread : vecThreads){
		pThread = MCF::Thread::Create([&]() noexcept {
			try {
				for(;;){
					if(__atomic_load_n(&bExitNow, __ATOMIC_ACQUIRE)){
						break;
					}
					std::function<void ()> fnJob;
					{
						const auto vLock = xm_pcsLock->GetLock();
						if(xm_queJobs.empty()){
							break;
						}
						fnJob = std::move(xm_queJobs.front());
						xm_queJobs.pop();
					}
					fnJob();
				}
			} catch(...){
				if(__atomic_exchange_n(&bExitNow, true, __ATOMIC_ACQ_REL) == false){
					pException = std::current_exception();
				}
			}
		});
	}
	for(auto &pThread : vecThreads){
		pThread->Wait();
	}

	if(pException){
		std::rethrow_exception(pException);
	}
}
