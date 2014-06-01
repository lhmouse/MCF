// MCF Build
// Copyleft 2014, LH_Mouse. All wrongs reserved.

#ifndef MCFBUILD_JOB_SCHEDULER_HPP_
#define MCFBUILD_JOB_SCHEDULER_HPP_

#include "../MCF/Core/Utilities.hpp"
#include "../MCF/Thread/MonitorPtr.hpp"
#include <queue>
#include <functional>

namespace MCFBuild {

class JobScheduler : NO_COPY {
private:
	class xJobQueue {
	private:
		std::queue<std::function<void ()>> xm_queJobs;

	public:
		void Enqueue(std::function<void ()> fnJob){
			xm_queJobs.emplace(std::move(fnJob));
		}
		std::function<void ()> Dequeue(){
			if(xm_queJobs.empty()){
				return nullptr;
			}
			auto fnRet = std::move(xm_queJobs.front());
			xm_queJobs.pop();
			return std::move(fnRet);
		}
	};

private:
	MCF::MonitorPtr<xJobQueue> xm_pJobQueue;

public:
	void AddJob(std::function<void ()> fnJob);
	void CommitAll(std::size_t uThreadCount);
};

}

#endif
