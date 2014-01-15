// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#include "StdMCF.hpp"
#include "Time.hpp"
#include <unordered_map>
#include "SymbolManagerClass.hpp"
using namespace MCF;

#ifndef NDEBUG

namespace {
	template<typename T>
	struct xMAllocAllocator {
		typedef T				*pointer;
		typedef const T			*const_pointer;
		typedef void			*void_pointer;
		typedef const void		*const_void_pointer;

		typedef T				&reference;
		typedef const T			&const_reference;

		typedef T				value_type;
		typedef std::size_t		size_type;
		typedef std::ptrdiff_t	difference_type;

		template<typename U>
		struct rebind {
			typedef xMAllocAllocator<U> other;
		};

		xMAllocAllocator(){
		}
		xMAllocAllocator(const xMAllocAllocator &){
		}
		template<typename U>
		xMAllocAllocator(const xMAllocAllocator<U> &){
		};
		~xMAllocAllocator(){
		}

		pointer allocate(size_type n){
			return (pointer)std::malloc(n * sizeof(T));
		}
		pointer allocate(size_type n, const_void_pointer){
			return allocate(n);
		}
		void deallocate(pointer p, size_type){
			std::free((void *)p);
		}
		size_type max_size() const {
			return std::numeric_limits<size_type>::max();
		}

		void construct(pointer p, const value_type &v){
			new((void *)p) value_type(v);
		}
		void construct(pointer p, value_type &&v){
			new((void *)p) value_type(std::move(v));
		}
		void destroy(pointer p){
			p->~T();
		}
	};

	void xMemDbgFatalError(const char *pszDesc, DWORD dwFunc1Addr, const char *pszFunc1Name, DWORD dwFunc2Addr = 0, const char *pszFunc2Name = nullptr){
		static char pszMessageBuffer[0x400];

		SymbolManagerClass SymbolManager;
		if((dwFunc2Addr == 0) || (pszFunc2Name == nullptr)){
			std::sprintf_s(
				pszMessageBuffer,
				"%s\n\n%s 调用自地址 0x%08lX（符号：%s）\n\n单击“确定”终止应用程序，单击“取消”调试应用程序。",
				pszDesc,
				pszFunc1Name,
				(unsigned long)dwFunc1Addr,
				SymbolManager.SymbolFromAddr(dwFunc1Addr)
			);
		} else {
			std::sprintf_s(
				pszMessageBuffer,
				"%s\n\n%s 调用自地址 0x%08lX（符号：%s）\n%s 调用自地址 0x%08lX（符号：%s）\n\n单击“确定”终止应用程序，单击“取消”调试应用程序。",
				pszDesc,
				pszFunc1Name,
				(unsigned long)dwFunc1Addr,
				SymbolManager.SymbolFromAddr(dwFunc1Addr),
				pszFunc2Name,
				(unsigned long)dwFunc2Addr,
				SymbolManager.SymbolFromAddr(dwFunc2Addr)
			);
		}

		if(xDebugMessageBoxA(pszMessageBuffer, "MCF Memory Debug Helper", MB_OKCANCEL | MB_ICONSTOP | MB_TASKMODAL | MB_TOPMOST) == IDOK){
			xBail(-2);
		} else {
			::__debugbreak();
		}
	}

	class xMemDebugHelper : NO_COPY_OR_ASSIGN {
	private:
		typedef std::unordered_map<
			const void *,
			std::pair<DWORD, bool>,
			std::hash<const void *>,
			std::equal_to<const void *>,
			xMAllocAllocator<std::pair<const void *, std::pair<DWORD, bool>>>
		> xRECORD_MAP;
	private:
		CRITICAL_SECTION xm_csMapLock;
		xRECORD_MAP xm_mapAllocRecords;
		xMAllocAllocator<BYTE> xm_InternalAllocator;

		BYTE xm_abyGuardBand[0x100];
	public:
		xMemDebugHelper(){
			::InitializeCriticalSectionAndSpinCount(&xm_csMapLock, 0x1000);

			std::srand(GenRandSeed());
			for(std::size_t i = 0; i < sizeof(xm_abyGuardBand); ++i){
				xm_abyGuardBand[i] = (BYTE)(rand() & 0xFF);
			}
		}
		~xMemDebugHelper(){
			if(!xm_mapAllocRecords.empty()){
				xMemDbgFatalError("侦测到内存泄漏。", xm_mapAllocRecords.begin()->second.first, xm_mapAllocRecords.begin()->second.second ? "operator new[]" : "operator new");
			}

			::DeleteCriticalSection(&xm_csMapLock);
		}
	public:
		void *__thiscall Alloc(std::size_t uSize, bool bIsVec, DWORD dwRetAddr){
			BYTE *const pbyRawBlock =xm_InternalAllocator.allocate(uSize + sizeof(xm_abyGuardBand) * 2 + sizeof(std::size_t));
			if(pbyRawBlock == nullptr){
				throw std::bad_alloc();
			}

			BYTE *const pbyUserBlock = pbyRawBlock + sizeof(std::size_t) + sizeof(xm_abyGuardBand);

			*(std::size_t *)pbyRawBlock = uSize;
			std::memcpy(pbyRawBlock + sizeof(std::size_t),	xm_abyGuardBand, sizeof(xm_abyGuardBand));
			std::memcpy(pbyUserBlock + uSize,				xm_abyGuardBand, sizeof(xm_abyGuardBand));

			::EnterCriticalSection(&xm_csMapLock);
				xm_mapAllocRecords.emplace(std::make_pair(pbyUserBlock, std::make_pair(dwRetAddr, bIsVec)));
			::LeaveCriticalSection(&xm_csMapLock);

			return pbyUserBlock;
		}
		void __thiscall Free(void *pbyBlock, bool bIsVec, DWORD dwRetAddr){
			if(pbyBlock == nullptr){
				return;
			}

			BYTE *const pbyUserBlock = (BYTE *)pbyBlock;

			::EnterCriticalSection(&xm_csMapLock);
				const auto iter = xm_mapAllocRecords.find(pbyBlock);
				const bool bIsValid = (iter != xm_mapAllocRecords.end());

				DWORD dwAllocAddr = 0;
				bool bWasVec = false;

				if(bIsValid){
					dwAllocAddr = iter->second.first;
					bWasVec = iter->second.second;

					xm_mapAllocRecords.erase(iter);
				}
			::LeaveCriticalSection(&xm_csMapLock);

			if(!bIsValid){
				xMemDbgFatalError("堆内存指针无效。", dwRetAddr, bIsVec ? "operator delete[]" : "operator delete");
			}

			BYTE *const pbyRawBlock = pbyUserBlock - (sizeof(xm_abyGuardBand) + sizeof(std::size_t));
			const std::size_t uSize = *(std::size_t *)pbyRawBlock;
			if((std::memcmp(pbyRawBlock + sizeof(std::size_t), xm_abyGuardBand, sizeof(xm_abyGuardBand)) != 0) || (std::memcmp(pbyUserBlock + uSize, xm_abyGuardBand, sizeof(xm_abyGuardBand)) != 0)){
				xMemDbgFatalError("堆内存边界被破坏。", dwAllocAddr, bWasVec ? "operator new[]" : "operator new", dwRetAddr, bIsVec ? "operator delete[]" : "operator delete");
			}
			if(bIsVec && !bWasVec){
				xMemDbgFatalError("试图使用 operator delete[] 释放由 operator new 分配的内存。", dwAllocAddr, "operator new", dwRetAddr, "operator delete[]");
			} else if(!bIsVec && bWasVec){
				xMemDbgFatalError("试图使用 operator delete 释放由 operator new[] 分配的内存。", dwAllocAddr, "operator new[]", dwRetAddr, "operator delete");
			}

			*(std::size_t *)pbyRawBlock = (std::size_t)-1;

			xm_InternalAllocator.deallocate(pbyRawBlock, 0);
		}
	};

	xMemDebugHelper xs_MemMgr __attribute__((init_priority(101)));
}

__declspec(naked, noinline) void *__cdecl operator new(std::size_t){
	__asm {
		push ebp
		mov ebp, esp

		lea ecx, dword ptr[xs_MemMgr]
		mov eax, dword ptr[ebp + 4]
		mov edx, dword ptr[ebp + 8]
		push eax
		push 0
		push edx
		call xMemDebugHelper::Alloc

		leave
		ret
	}
}
__declspec(naked, noinline) void *__cdecl operator new[](std::size_t){
	__asm {
		push ebp
		mov ebp, esp

		lea ecx, dword ptr[xs_MemMgr]
		mov eax, dword ptr[ebp + 4]
		mov edx, dword ptr[ebp + 8]
		push eax
		push 1
		push edx
		call xMemDebugHelper::Alloc

		leave
		ret
	}
}
__declspec(naked, noinline) void __cdecl operator delete(void *){
	__asm {
		push ebp
		mov ebp, esp

		lea ecx, dword ptr[xs_MemMgr]
		mov eax, dword ptr[ebp + 4]
		mov edx, dword ptr[ebp + 8]
		push eax
		push 0
		push edx
		call xMemDebugHelper::Free

		leave
		ret
	}
}
__declspec(naked, noinline) void __cdecl operator delete[](void *){
	__asm {
		push ebp
		mov ebp, esp

		lea ecx, dword ptr[xs_MemMgr]
		mov eax, dword ptr[ebp + 4]
		mov edx, dword ptr[ebp + 8]
		push eax
		push 1
		push edx
		call xMemDebugHelper::Free

		leave
		ret
	}
}

#endif
