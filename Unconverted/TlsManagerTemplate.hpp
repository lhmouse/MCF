// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_TLS_MANAGER_TEMPLATE_HPP__
#define __MCF_TLS_MANAGER_TEMPLATE_HPP__

#include "StdMCF.hpp"
#include <map>
#include <forward_list>

namespace MCF {
	extern bool AtThreadExit(std::function<void(bool)> &&AtThreadExitProc);

	template<typename ELEMENT_TYPE>
	class TLSManagerTemplate : NO_COPY_OR_ASSIGN {
	private:
		struct xPtrHolder {
			std::unique_ptr<ELEMENT_TYPE> m_ptr;

			xPtrHolder(ELEMENT_TYPE *ptr) : m_ptr(ptr) { }
			xPtrHolder(xPtrHolder &&src) : m_ptr(std::move(src.m_ptr)) { }
			xPtrHolder &operator=(xPtrHolder &&src){ m_ptr = std::move(src.m_ptr); return *this; }
		};
	private:
		DWORD xm_dwTlsIndex;
		std::map<DWORD, std::forward_list<xPtrHolder>> xm_mapElements;
		CriticalSection xm_csMapLock;
	public:
		TLSManagerTemplate(){
			xm_dwTlsIndex = ::TlsAlloc();
			VERIFY(xm_dwTlsIndex != TLS_OUT_OF_INDEXES);
		}
		~TLSManagerTemplate(){
			::TlsFree(xm_dwTlsIndex);
		}
	private:
		void xAtThreadExitProc(bool bIsParasite){
			if(!bIsParasite){
				xm_csMapLock.Enter();
					const auto iterElementList = xm_mapElements.find(::GetCurrentThreadId());
					if(iterElementList != xm_mapElements.end()){
						xm_mapElements.erase(iterElementList);
					}
				xm_csMapLock.Leave();
			}
		}
	public:
		ELEMENT_TYPE *GetPtr(){
			auto pRet = (ELEMENT_TYPE *)::TlsGetValue(xm_dwTlsIndex);
			if(pRet == nullptr){
				xm_csMapLock.Enter();
					pRet = new ELEMENT_TYPE();
					xm_mapElements[::GetCurrentThreadId()].emplace_front(pRet);
				xm_csMapLock.Leave();

				::TlsSetValue(xm_dwTlsIndex, pRet);
				AtThreadExit(std::bind(&TLSManagerTemplate::xAtThreadExitProc, this, std::placeholders::_1));
			}
			return pRet;
		}
	public:
		operator ELEMENT_TYPE *(){
			return GetPtr();
		}
		ELEMENT_TYPE *operator->(){
			return GetPtr();
		}
		ELEMENT_TYPE &operator*(){
			return *GetPtr();
		}
	};
}

#endif
