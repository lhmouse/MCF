// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_EVENT_HANDLER_CLASS_HPP__
#define __MCF_EVENT_HANDLER_CLASS_HPP__

#include "StdMCF.hpp"
#include <map>
#include <list>

namespace MCF {
	class EventHandlerClass final : NO_COPY_OR_ASSIGN {
	public:
		typedef std::function<bool(std::intptr_t, std::intptr_t, std::intptr_t)> HANDLER;
	public:
		class Delegate final {
		private:
			typedef struct xtagContents {
				EventHandlerClass *pOwner;
				DWORD dwEventID;
				std::list<HANDLER>::const_iterator iterPos;
				unsigned long ulRefCount;
			} xCONTENTS;
		private:
			xCONTENTS *xm_pContents;
		public:
			Delegate(){
				xm_pContents = nullptr;
			}
			Delegate(EventHandlerClass *pOwner, DWORD dwEventID, std::list<HANDLER>::const_iterator &&iterPos){
				xSet(pOwner, dwEventID, std::move(iterPos));
			}
			Delegate(const Delegate &src){
				xm_pContents = src.xm_pContents;
				if(xm_pContents != nullptr){
					::InterlockedIncrement(&xm_pContents->ulRefCount);
				}
			}
			Delegate(Delegate &&src){
				xm_pContents = src.xm_pContents;
				src.xm_pContents = nullptr;
			}
			Delegate &operator=(const Delegate &src){
				xRelease();
				xm_pContents = src.xm_pContents;
				if(xm_pContents != nullptr){
					::InterlockedIncrement(&xm_pContents->ulRefCount);
				}
				return *this;
			}
			Delegate &operator=(Delegate &&src){
				xRelease();
				xm_pContents = src.xm_pContents;
				src.xm_pContents = nullptr;
				return *this;
			}
			~Delegate(){
				xRelease();
			}
		private:
			inline __attribute__((always_inline)) void xSet(EventHandlerClass *pOwner, DWORD dwEventID, std::list<HANDLER>::const_iterator &&iterPos){
				xm_pContents = new xCONTENTS;
				xm_pContents->pOwner		= pOwner;
				xm_pContents->dwEventID		= dwEventID;
				xm_pContents->iterPos		= std::move(iterPos);
				xm_pContents->ulRefCount	= 1;
			}
			inline __attribute__((always_inline)) void xRelease(){
				if(xm_pContents != nullptr){
					if(::InterlockedDecrement(&xm_pContents->ulRefCount) == 0){
						xm_pContents->pOwner->xUnregisterHandler(xm_pContents->dwEventID, xm_pContents->iterPos);
						delete xm_pContents;
					}
					xm_pContents = nullptr;
				}
			}
		};
	private:
		typedef std::map<DWORD, std::list<HANDLER>> xHANDLER_MAP;
	private:
		ReadWriteLock *const xm_prwlExternalLock;
		xHANDLER_MAP xm_mapHandlers;
	public:
		EventHandlerClass(ReadWriteLock *prwlExternalLock = nullptr);
		~EventHandlerClass();
	private:
		void xUnregisterHandler(DWORD dwEventID, std::list<HANDLER>::const_iterator iterPos);
	public:
		Delegate RegisterHandler(DWORD dwEventID, HANDLER &&Handler);
		void Raise(DWORD dwEventID, std::intptr_t nArg1, std::intptr_t nArg2, std::intptr_t nArg3) const;
	};

	typedef EventHandlerClass::HANDLER		EVENT_HANDLER;
	typedef EventHandlerClass::Delegate		EventHandlerDelegate;

	extern EventHandlerDelegate RegisterGlobalEventHandler(DWORD dwEventID, EVENT_HANDLER &&EventHandler);
	extern void RaiseGlobalEvent(DWORD dwEventID, std::intptr_t nArg1 = 0, std::intptr_t nArg2 = 0, std::intptr_t nArg3 = 0);

	extern EventHandlerDelegate RegisterThreadEventHandler(DWORD dwEventID, EVENT_HANDLER &&EventHandler);
	extern void RaiseThreadEvent(DWORD dwEventID, std::intptr_t nArg1 = 0, std::intptr_t nArg2 = 0, std::intptr_t nArg3 = 0);
}

#endif
