// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_X_POOLED_THREAD_CLASS_HPP__
#define __MCF_X_POOLED_THREAD_CLASS_HPP__

#include "StdMCF.hpp"
#include "ThreadPoolClass.hpp"
#include "AbstractThreadClass.hpp"

namespace MCF {
	class ThreadPoolClass::xPooledThreadClass final : public AbstractThreadClass {
	private:
		ThreadPoolClass *const xm_pOwner;
	public:
		xPooledThreadClass(ThreadPoolClass *pOwner);
		~xPooledThreadClass();
	private:
		virtual int xThreadProc() override;
	};
}

#endif
