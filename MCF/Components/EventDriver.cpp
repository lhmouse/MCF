// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "EventDriver.hpp"
#include "../Core/CriticalSection.hpp"
#include <memory>
#include <map>
#include <list>
#include <functional>
#include <algorithm>
#include <iterator>
#include <climits>
using namespace MCF;

namespace {
	typedef std::function<bool (std::intptr_t)> HANDLER_PROC;
	typedef std::shared_ptr<HANDLER_PROC> PHANDLER_PROC;
	typedef std::list<PHANDLER_PROC> HANDLER_LIST;
	typedef std::shared_ptr<const HANDLER_LIST> PCHANDLER_LIST;
	typedef std::shared_ptr<HANDLER_LIST> PHANDLER_LIST;

	// 使用 Copy-On-Write 策略。
	CriticalSection g_csWriteLock;
	CriticalSection g_csReadLock;
	std::map<std::intptr_t, PCHANDLER_LIST> g_mapDelegates;
}

namespace MCF {

namespace __MCF {
	void UnregisterEventHandler(const EVENT_HANDLER_HANDLE &Handle) noexcept {
		if(!Handle.second){
			return;
		}
		CRITICAL_SECTION_SCOPE(g_csWriteLock){
			PCHANDLER_LIST pOldList;
			CRITICAL_SECTION_SCOPE(g_csReadLock){
				const auto itList = g_mapDelegates.find(Handle.first);
				if(itList != g_mapDelegates.end()){
					pOldList = itList->second;
				}
			}
			if(pOldList){
				PHANDLER_LIST pNewList;

				auto it = pOldList->cbegin();
				while(it != pOldList->cend()){
					if(it->get() == Handle.second){
						pNewList = std::make_shared<HANDLER_LIST>();
						std::copy(pOldList->cbegin(), it, std::back_inserter(*pNewList));
						++it;
						std::copy(it, pOldList->cend(), std::back_inserter(*pNewList));
						break;
					}
					++it;
				}

				if(pNewList){
					CRITICAL_SECTION_SCOPE(g_csReadLock){
						const auto itList = g_mapDelegates.find(Handle.first);
						ASSERT(itList != g_mapDelegates.end());

						if(pNewList->empty()){
							g_mapDelegates.erase(itList);
						} else {
							itList->second = std::move(pNewList);
						}
					}
				}
			}
		}
	}
}

EventHandlerHolder RegisterEventHandler(std::intptr_t nEventId, std::function<bool (std::intptr_t)> fnHandler){
	EventHandlerHolder Holder;
	CRITICAL_SECTION_SCOPE(g_csWriteLock){
		PCHANDLER_LIST pOldList;
		CRITICAL_SECTION_SCOPE(g_csReadLock){
			const auto itList = g_mapDelegates.find(nEventId);
			if(itList != g_mapDelegates.end()){
				pOldList = itList->second;
			}
		}
		PHANDLER_LIST pNewList;
		if(pOldList){
			pNewList = std::make_shared<HANDLER_LIST>(*pOldList);
		} else {
			pNewList = std::make_shared<HANDLER_LIST>();
		}
		pNewList->emplace_front(std::make_shared<HANDLER_PROC>(std::move(fnHandler)));
		const auto pRaw = pNewList->front().get();
		CRITICAL_SECTION_SCOPE(g_csReadLock){
			g_mapDelegates[nEventId] = std::move(pNewList);
		}
		Holder = std::make_pair(nEventId, pRaw);
	}
	return std::move(Holder);
}
void RaiseEvent(std::intptr_t nEventId, std::intptr_t nContext){
	PCHANDLER_LIST pList;
	CRITICAL_SECTION_SCOPE(g_csReadLock){
		const auto itList = g_mapDelegates.find(nEventId);
		if(itList != g_mapDelegates.end()){
			pList = itList->second;
		}
	}
	if(pList){
		for(const auto &pHandler : *pList){
			if((*pHandler)(nContext)){
				break;
			}
		}
	}
}

}
