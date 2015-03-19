// Copyleft 2013, LH_Mouse. All wrongs reserved.

#ifndef __JOB_SCHEDULER_HPP__
#define __JOB_SCHEDULER_HPP__

#include <list>
#include <vector>
#include <memory>
#include <functional>

namespace MCFBuild {
	class JobScheduler {
	public:
		typedef std::function<void(std::size_t)> JOB;
	private:
		static DWORD WINAPI xThreadProc(LPVOID pParam);
	private:
		CRITICAL_SECTION x_csLock;
		std::list<JOB> x_lstJobs;

		std::size_t x_uCountTotal;
		std::size_t x_uCountCompleted;
		const wchar_t *x_pwszPrefix;

		std::vector<DWORD> x_vecThreadIDs;
		std::unique_ptr<Exception> x_pCaughtException;
	public:
		JobScheduler();
		~JobScheduler();

		JobScheduler(const JobScheduler &) = delete;
		void operator=(const JobScheduler &) = delete;
	private:
		void xThreadJobProc();
		void xPrintStatus(bool bStartsNewLine) const;
	public:
		void PushJob(JOB &&fnJobProc);
		void Commit(std::size_t uProcessCount, const wchar_t *pwszPrefix);
	};
}

#endif
