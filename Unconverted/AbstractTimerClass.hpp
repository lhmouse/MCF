// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_ABSTRACT_TIMER_CLASS_HPP__
#define __MCF_ABSTRACT_TIMER_CLASS_HPP__

#include "StdMCF.hpp"
#include <mmsystem.h>

namespace MCF {
	class AbstractTimerClass : NO_COPY_OR_ASSIGN {
	private:
		static void CALLBACK xTimerProc(UINT uTimerID, UINT, DWORD_PTR dwUser, DWORD_PTR, DWORD_PTR);
	private:
		const unsigned int xm_uInterval;
		const unsigned int xm_uPrecision;
		MMRESULT xm_TimerID;
	public:
		AbstractTimerClass(unsigned int uInterval, unsigned int uPrecision);
		virtual ~AbstractTimerClass();
	protected:
		virtual void xTimerLoop() = 0;
	public:
		void Start();
		void Stop();
		bool IsActive() const;
	};
}

#endif
