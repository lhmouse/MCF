// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_X_THREAD_PARASITE_CLASS_HPP__
#define __MCF_X_THREAD_PARASITE_CLASS_HPP__

#include "StdMCF.hpp"
#include "StandaloneWindowClass.hpp"
#include "AbstractMessageThreadClass.hpp"

namespace MCF {
	class StandaloneWindowClass::xThreadParasiteClass final : protected AbstractMessageThreadClass {
	private:
		std::intptr_t xm_nContext;
		HACCEL xm_hAcceleratorTable;

		StandaloneWindowClass *xm_pMainWindow;
		bool xm_bTranslateDialogMessages;

		xThreadParasiteClass *xm_pPrevParasite;
	public:
		xThreadParasiteClass(StandaloneWindowClass *pMainWindow, bool bTranslateDialogMessages, std::intptr_t nContext, HACCEL hAcceleratorTable);
		~xThreadParasiteClass();
	public:
		virtual bool xDoOnIdle();
		virtual void xDoOnMessage(MSG *pMsg);
	public:
		std::intptr_t GetContext() const;

		int ParasitizeCurrentThread();
		void QuitMessageLoop(int nExitCode);
	};
}

#endif
