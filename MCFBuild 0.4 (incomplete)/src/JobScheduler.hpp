// MCF Build
// Copyleft 2014, LH_Mouse. All wrongs reserved.

#ifndef MCFBUILD_JOB_SCHEDULER_HPP_
#define MCFBUILD_JOB_SCHEDULER_HPP_

#include "../MCF/Utilities/Noncopyable.hpp"
#include "../MCF/Thread/CriticalSection.hpp"
#include <queue>
#include <functional>

namespace MCFBuild {

class JobScheduler : Noncopyable {
private:
	const std::unique_ptr<MCF::CriticalSection> xm_pcsLock;
	std::queue<std::function<void ()>> xm_queJobs;

public:
	JobScheduler();

public:
	void AddJob(std::function<void ()> fnJob);
	void CommitAll(std::size_t uThreadCount);
};

}

#endif
