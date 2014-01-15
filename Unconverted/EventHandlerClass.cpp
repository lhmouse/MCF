// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#include "StdMCF.hpp"
#include "EventHandlerClass.hpp"
#include "Memory.hpp"
using namespace MCF;

// 构造函数和析构函数。
EventHandlerClass::EventHandlerClass(ReadWriteLock *prwlExternalLock) :
	xm_prwlExternalLock(prwlExternalLock)
{
}
EventHandlerClass::~EventHandlerClass(){
}

// 其他非静态成员函数。
void EventHandlerClass::xUnregisterHandler(DWORD dwEventID, std::list<EventHandlerClass::HANDLER>::const_iterator iterPos){
	if(xm_prwlExternalLock != nullptr){
		xm_prwlExternalLock->AcquireWriteLock();
	}

		const auto iterList = xm_mapHandlers.find(dwEventID);
		if(iterList != xm_mapHandlers.end()){
			iterList->second.erase(iterPos);
		}

	if(xm_prwlExternalLock != nullptr){
		xm_prwlExternalLock->ReleaseWriteLock();
	}
}

EventHandlerClass::Delegate EventHandlerClass::RegisterHandler(DWORD dwEventID, EventHandlerClass::HANDLER &&Handler){
	if(xm_prwlExternalLock != nullptr){
		xm_prwlExternalLock->AcquireWriteLock();
	}

		auto &HandlerList = xm_mapHandlers[dwEventID];
		HandlerList.emplace_front(std::move(Handler));
		Delegate ret(this, dwEventID, HandlerList.cbegin());

	if(xm_prwlExternalLock != nullptr){
		xm_prwlExternalLock->ReleaseWriteLock();
	}
	return std::move(ret);
}
void EventHandlerClass::Raise(DWORD dwEventID, std::intptr_t nArg1, std::intptr_t nArg2, std::intptr_t nArg3) const {
	if(xm_prwlExternalLock != nullptr){
		xm_prwlExternalLock->AcquireReadLock();
	}

		const auto iterList = xm_mapHandlers.find(dwEventID);
		if(iterList != xm_mapHandlers.end()){
			for(auto cur = iterList->second.cbegin(); cur != iterList->second.cend(); ++cur){
				if((*cur)(nArg1, nArg2, nArg3)){
					break;
				}
			}
		}

	if(xm_prwlExternalLock != nullptr){
		xm_prwlExternalLock->ReleaseReadLock();
	}
}

namespace MCF {
	// 全局事件响应器。
	namespace {
		ReadWriteLock		GlobalHandlerLock						__attribute__((init_priority(2000)));
		EventHandlerClass	GlobalEventHandler(&GlobalHandlerLock)	__attribute__((init_priority(2000)));
	}

	extern EventHandlerDelegate RegisterGlobalEventHandler(DWORD dwEventID, EVENT_HANDLER &&EventHandler){
		return GlobalEventHandler.RegisterHandler(dwEventID, std::move(EventHandler));
	}
	extern void RaiseGlobalEvent(DWORD dwEventID, std::intptr_t nArg1, std::intptr_t nArg2, std::intptr_t nArg3){
		GlobalEventHandler.Raise(dwEventID, nArg1, nArg2, nArg3);
	}

	// 线程事件响应器。
	namespace {
		TLSManagerTemplate<EventHandlerClass> ThreadEventHandler;
	}

	extern EventHandlerDelegate RegisterThreadEventHandler(DWORD dwEventID, EVENT_HANDLER &&EventHandler){
		return ThreadEventHandler->RegisterHandler(dwEventID, std::move(EventHandler));
	}
	extern void RaiseThreadEvent(DWORD dwEventID, std::intptr_t nArg1, std::intptr_t nArg2, std::intptr_t nArg3){
		ThreadEventHandler->Raise(dwEventID, nArg1, nArg2, nArg3);
	}
}
