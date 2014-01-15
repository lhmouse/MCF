// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_MEMORY_POOL_CLASS_HPP__
#define __MCF_MEMORY_POOL_CLASS_HPP__

#include "StdMCF.hpp"
#include <vector>

namespace MCF {
	class MemoryPoolClass : NO_COPY_OR_ASSIGN {
	private:
		typedef struct xtagBlockHeader {
			unsigned long ulIndex;
			xtagBlockHeader *pNext;
		} xBLOCKHEADER;
	private:
		xBLOCKHEADER *xm_arpPool[sizeof(void *) * CHAR_BIT];
	public:
		MemoryPoolClass(){
			std::fill_n(xm_arpPool, COUNTOF(xm_arpPool), nullptr);
		}
		~MemoryPoolClass(){
			Flush();
		}
	public:
		void *Alloc(std::size_t uCountBytes){
			if(uCountBytes < sizeof(xBLOCKHEADER)){
				uCountBytes = sizeof(xBLOCKHEADER);
			}

#ifdef __i386__
			const unsigned long ulIndex = (unsigned long)__builtin_clz(uCountBytes - 1) + 1;
#else
			const unsigned long ulIndex = (unsigned long)__builtin_clzll(uCountBytes - 1) + 1;
#endif

			xBLOCKHEADER *pBlock = xm_arpPool[ulIndex];
			if(pBlock == nullptr){
				pBlock = (xBLOCKHEADER *)::operator new((((std::size_t)1) << ulIndex) + offsetof(xBLOCKHEADER, pNext));
				pBlock->ulIndex = ulIndex;
			} else {
				xm_arpPool[ulIndex] = pBlock->pNext;
			}
			return (void *)(((BYTE *)pBlock) + offsetof(xBLOCKHEADER, pNext));
		}
		void Free(void *pBuff){
			if(pBuff == nullptr){
				return;
			}

			xBLOCKHEADER *const pBlock = (xBLOCKHEADER *)(((BYTE *)pBuff) - offsetof(xBLOCKHEADER, pNext));
			const DWORD ulIndex = pBlock->ulIndex;

			pBlock->pNext = xm_arpPool[ulIndex];
			xm_arpPool[ulIndex] = pBlock;
		}
		bool Flush(){
			bool ret = false;

			for(std::size_t i = 0; i < COUNTOF(xm_arpPool); ++i){
				xBLOCKHEADER *pCurrent = xm_arpPool[i];
				while(pCurrent != nullptr){
					xBLOCKHEADER *pNext = pCurrent->pNext;
					::operator delete(pCurrent);
					pCurrent = pNext;

					ret = true;
				}
				xm_arpPool[i] = nullptr;
			}

			return ret;
		}
	};
}

#endif
