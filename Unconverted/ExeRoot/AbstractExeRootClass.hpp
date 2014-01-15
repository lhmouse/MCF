// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_ABSTRACT_EXE_ROOT_CLASS_HPP__
#define __MCF_ABSTRACT_EXE_ROOT_CLASS_HPP__

#include <MCF/StdMCF.hpp>
#include <MCF/AbstractThreadClass.hpp>

extern "C" void __cdecl __MCFStartup();

namespace MCF {
	class AbstractExeRootClass : public AbstractThreadClass {
		friend void ::__MCFStartup();
	private:
		static std::atomic<AbstractExeRootClass *> xs_pLastLatentExeRoot;
	private:
		AbstractExeRootClass *xm_pNext;

		std::vector<LPCTSTR> *xm_pvecArgVector;
		int xm_nCmdShow;
		HINSTANCE xm_hInstance;
	public:
		AbstractExeRootClass();
		virtual ~AbstractExeRootClass();
	private:
		virtual int xThreadProc();
	protected:
		virtual int xMCFMain(std::size_t uArgCount, const TCHAR *const *ppArgVector, int nCmdShow) = 0;
	public:
		HINSTANCE GetHInstance() const;
	};
}

#endif
