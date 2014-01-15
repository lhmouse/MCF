// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_DEBUG_HELPERS_HPP__
#define __MCF_DEBUG_HELPERS_HPP__

#include "Utilities.hpp"
#include "GenericString.hpp"
#include <tchar.h>
#include <windows.h>

namespace MCF {
	inline TString GetErrorDescription(DWORD dwErrorCode){
		LPVOID pDescriptionBuffer;
		TString strRet;

		if(::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, dwErrorCode, 0, (LPTSTR)&pDescriptionBuffer, 0, nullptr) != 0){
			strRet = (LPCTSTR)pDescriptionBuffer;
		}
		::LocalFree((HLOCAL)pDescriptionBuffer);

		return strRet;
	}

	inline void __attribute__((noreturn)) __Bail(int nExitCode){
		::TerminateProcess(::GetCurrentProcess(), (std::size_t)nExitCode);
		throw;
	}

	inline int __DebugMessageBoxA(LPCSTR pszText, LPCSTR pszCaption, UINT uType){
		class MessageBoxThreadCreatorClass {
		private:
			static DWORD WINAPI xMessageBoxThread(LPVOID pParam){
				MessageBoxThreadCreatorClass *const pCreator = (MessageBoxThreadCreatorClass *)pParam;

				MSG MsgDummy;
				::PeekMessage(&MsgDummy, NULL, WM_USER, WM_USER, PM_NOREMOVE);

				pCreator->xm_nExitCode = ::MessageBoxA(NULL, pCreator->xm_pszText, pCreator->xm_pszCaption, pCreator->xm_uType);

				return 0;
			}
		private:
			const LPCSTR xm_pszText;
			const LPCSTR xm_pszCaption;
			const UINT xm_uType;
			int xm_nExitCode;
		public:
			MessageBoxThreadCreatorClass(LPCSTR pszText, LPCSTR pszCaption, UINT uType) :
				xm_pszText		(pszText),
				xm_pszCaption	(pszCaption),
				xm_uType		(uType),
				xm_nExitCode	(MB_OK)
			{
				const HANDLE hMessageBoxThread = ::CreateThread(nullptr, 0, &xMessageBoxThread, (LPVOID)this, 0, nullptr);
				::WaitForSingleObject(hMessageBoxThread, INFINITE);
				::CloseHandle(hMessageBoxThread);
			}
			~MessageBoxThreadCreatorClass(){
			}
		private:
			MessageBoxThreadCreatorClass(const MessageBoxThreadCreatorClass &);
			void operator=(const MessageBoxThreadCreatorClass &);
		public:
			int GetExitCode() const {
				return xm_nExitCode;
			}
		};

		return MessageBoxThreadCreatorClass(pszText, pszCaption, uType).GetExitCode();
	}
	inline int __DebugMessageBoxW(LPCWSTR pwszText, LPCWSTR pwszCaption, UINT uType){
		class MessageBoxThreadCreatorClass {
		private:
			static DWORD WINAPI xMessageBoxThread(LPVOID pParam){
				MessageBoxThreadCreatorClass *const pCreator = (MessageBoxThreadCreatorClass *)pParam;

				MSG MsgDummy;
				::PeekMessage(&MsgDummy, NULL, WM_USER, WM_USER, PM_NOREMOVE);

				pCreator->xm_nExitCode = ::MessageBoxW(NULL, pCreator->xm_pwszText, pCreator->xm_pwszCaption, pCreator->xm_uType);

				return 0;
			}
		private:
			const LPCWSTR xm_pwszText;
			const LPCWSTR xm_pwszCaption;
			const UINT xm_uType;
			int xm_nExitCode;
		public:
			MessageBoxThreadCreatorClass(LPCWSTR pwszText, LPCWSTR pwszCaption, UINT uType) :
				xm_pwszText	(pwszText),
				xm_pwszCaption	(pwszCaption),
				xm_uType		(uType),
				xm_nExitCode	(MB_OK)
			{
				const HANDLE hMessageBoxThread = ::CreateThread(nullptr, 0, &xMessageBoxThread, (LPVOID)this, 0, nullptr);
				::WaitForSingleObject(hMessageBoxThread, INFINITE);
				::CloseHandle(hMessageBoxThread);
			}
			~MessageBoxThreadCreatorClass(){
			}
		private:
			MessageBoxThreadCreatorClass(const MessageBoxThreadCreatorClass &);
			void operator=(const MessageBoxThreadCreatorClass &);
		public:
			int GetExitCode() const {
				return xm_nExitCode;
			}
		};

		return MessageBoxThreadCreatorClass(pwszText, pwszCaption, uType).GetExitCode();
	}

// ======================================================================================================================================================
// ======================================================================================================================================================

	// 断言。
	// 用法和标准库的 assert() 无甚区别。
#ifndef NDEBUG
	extern inline void __ShowAssertion(const char *pszExpression, const char *pszFile, long lLine, const char *pszDescription){
		const DWORD dwLastError = ::GetLastError();
		if(__DebugMessageBoxA(
			MBString().Format(
				"调试断言失败！\n\n表达式：%s\n文件　：%s\n行号　：%ld\n描述　：%s\n\n单击“确定”终止应用程序，单击“取消”调试应用程序。",
				pszExpression,
				pszFile,
				lLine,
				pszDescription
			),
			"断言失败",
			MB_OKCANCEL | MB_ICONSTOP | MB_TASKMODAL | MB_TOPMOST
		) == IDOK){
			__Bail(-3);
		} else {
			::SetLastError(dwLastError);
			::DebugBreak();
		}
	}
#endif	// NDEBUG

#ifndef NDEBUG
#	define ASSERT(exp)					((void)(!!(exp) || (::MCF::__ShowAssertion(#exp, __FILE__, __LINE__, ""), false)))
#	define ASSERTD(exp, description)	((void)(!!(exp) || (::MCF::__ShowAssertion(#exp, __FILE__, __LINE__, description), false)))
#else	// NDEBUG
#	define ASSERT(exp)					((void)0)
#	define ASSERTD(exp, description)	((void)0)
#endif	// NDEBUG

// ======================================================================================================================================================
// ======================================================================================================================================================

	// 验证。
	// 验证和断言的区别有两个：
	// 1. 验证是一个内联函数，如果被验证的值转换为 bool 结果为 true，则返回该值本身。
	//    因此可以有如下语法：int *pnSomeInt = VERIFY(new int());
	// 2. 断言在 Release 配置下无效，验证仍有效但验证失败时不提供调试选项。
#ifndef NDEBUG
	template<typename T>
	extern inline T &&__ShowVerification(T &&arg, const char *pszExpression, const char *pszFile, long lLine, const char *pszDescription){
		if(!arg){
			const DWORD dwLastError = ::GetLastError();
			if(__DebugMessageBoxA(
				MBString().Format(
					"验证失败！\n\n表达式：%s\n文件　：%s\n行号　：%ld\n描述　：%s\n\n错误代码：%lu\n错误描述：%s\n单击“确定”终止应用程序，单击“取消”调试应用程序。",
					pszExpression,
					pszFile,
					lLine,
					pszDescription,
					(unsigned long)dwLastError,
					(LPCSTR)MBString().ConvFrom(GetErrorDescription(dwLastError))
				),
				"验证失败",
				MB_OKCANCEL | MB_ICONSTOP | MB_TASKMODAL | MB_TOPMOST
			) == IDOK){
				__Bail(-3);
			} else {
				::SetLastError(dwLastError);
				::DebugBreak();
			}
		}
		return std::forward<T>(arg);
	}
#else	// NDEBUG
	template<typename T>
	extern inline T &&__ShowVerification(T &&arg, const char *pszExpression, const char *pszFile, std::size_t uLine, const char *pszDescription){
		if(!arg){
			const DWORD dwLastError = ::GetLastError();
			__DebugMessageBoxA(
				MBString().Format(
					"应用程序运行时出现错误，请联系作者以了解有关此问题的解决方案。\n表达式：%s\n\n文件　：%s\n行号　：%d\n描述　：%s\n\n错误代码：%lu\n错误描述：%s\n单击“确定”终止应用程序。",
					pszExpression,
					pszFile,
					uLine,
					pszDescription,
					(unsigned long)dwLastError,
					(LPCSTR)MBString().ConvFrom(GetErrorDescription(dwLastError))
				),
				"验证失败",
				MB_OK | MB_ICONSTOP | MB_TASKMODAL | MB_TOPMOST
			);
			__Bail(-3);
		}
		return std::forward<T>(arg);
	}
#endif	// NDEBUG
}

#define VERIFY(exp)						(::MCF::__ShowVerification((exp), #exp, __FILE__, __LINE__, ""))
#define VERIFYD(exp, description)		(::MCF::__ShowVerification((exp), #exp, __FILE__, __LINE__, description))

// ======================================================================================================================================================
// ======================================================================================================================================================

// 调试警告框。
// 可以用于程序运行时即时导出变量的值。
// 用法和 JavaScript 中的 alert() 类似。
// 这里提供 ANSI 和 Unicode 字符串的重载版本，使用时不必加入 xT()。
inline void ALERT(LPCSTR pszMessage){
	::MCF::__DebugMessageBoxA(pszMessage, "Debug Alert", MB_ICONWARNING | MB_OK | MB_TASKMODAL);
}
inline void ALERTF(LPCSTR pszFormat, ...){
	std::va_list ArgList;

	va_start(ArgList, pszFormat);
	::MCF::__DebugMessageBoxA(::MCF::MBString().VFormat(pszFormat, ArgList), "Debug Alert", MB_ICONWARNING | MB_OK | MB_TASKMODAL);
	va_end(ArgList);
}
inline void ALERT(LPCWSTR pwszMessage){
	::MCF::__DebugMessageBoxW(pwszMessage, L"Debug Alert", MB_ICONWARNING | MB_OK | MB_TASKMODAL);
}
inline void ALERTF(LPCWSTR pwszFormat, ...){
	std::va_list ArgList;

	va_start(ArgList, pwszFormat);
	::MCF::__DebugMessageBoxW(::MCF::WCString().VFormat(pwszFormat, ArgList), L"Debug Alert", MB_ICONWARNING | MB_OK | MB_TASKMODAL);
	va_end(ArgList);
}

// 输出调试字符串。
// 这里提供 ANSI 和 Unicode 字符串的重载版本，使用时不必加入 xT()。
inline void TRACE(LPCSTR pszMessage){
	::OutputDebugStringA(::MCF::MBString(pszMessage) += '\n');
}
inline void TRACEF(LPCSTR pszFormat, ...){
	std::va_list ArgList;

	va_start(ArgList, pszFormat);
	::OutputDebugStringA(::MCF::MBString().VFormat(pszFormat, ArgList) += '\n');
	va_end(ArgList);
}
inline void TRACE(LPCWSTR pwszMessage){
	::OutputDebugStringW(::MCF::WCString(pwszMessage) += L'\n');
}
inline void TRACEF(LPCWSTR pwszFormat, ...){
	std::va_list ArgList;

	va_start(ArgList, pwszFormat);
	::OutputDebugStringW(::MCF::WCString().VFormat(pwszFormat, ArgList) += L'\n');
	va_end(ArgList);
}

#endif
