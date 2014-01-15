// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_ABSTRACT_MESSAGE_THREAD_CLASS_HPP__
#define __MCF_ABSTRACT_MESSAGE_THREAD_CLASS_HPP__

#include "StdMCF.hpp"
#include "AbstractThreadClass.hpp"

namespace MCF {
	class AbstractMessageThreadClass : public AbstractThreadClass {
	private:
		Event xm_ReadyEvent;
	public:
		AbstractMessageThreadClass();
		~AbstractMessageThreadClass();
	private:
		virtual int xThreadProc() override;
	private:
		int xDoMessageLoop();
	protected:
		virtual void xPostCreate();
		virtual void xPreTerminate(int nExitCode);

		virtual bool xDoOnIdle() = 0;
		virtual void xDoOnMessage(MSG *pMsg) = 0;
	public:
		void Create(bool bSuspended);

		void PostMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
		void QuitMessageLoop(int nExitCode);
	};
}

#endif
