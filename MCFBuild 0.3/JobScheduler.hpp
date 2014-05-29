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
		CRITICAL_SECTION xm_csLock;
		std::list<JOB> xm_lstJobs;

		std::size_t xm_uCountTotal;
		std::size_t xm_uCountCompleted;
		const wchar_t *xm_pwszPrefix;

		std::vector<DWORD> xm_vecThreadIDs;
		std::unique_ptr<Exception> xm_pCaughtException;
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
