// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_PROGRESS_BAR_CLASS_HPP__
#define __MCF_PROGRESS_BAR_CLASS_HPP__

#include "StdMCF.hpp"
#include "WindowBaseClass.hpp"
#include <commctrl.h>

namespace MCF {
	class ProgressBarClass : public WindowBaseClass {
	public:
		ProgressBarClass();
		~ProgressBarClass();
	protected:
		virtual void xPostSubclass() override;
	public:
		bool IsSmooth() const;
		void SetSmooth(bool bToSet);

		WORD GetProgress() const;
		void SetProgress(WORD wNewProgress);

		bool IsMarquee() const;
		void SetMarquee(bool bToSet, unsigned int uInterval = 25);
	};
}

#endif
