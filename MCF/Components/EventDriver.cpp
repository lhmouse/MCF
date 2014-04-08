// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "EventDriver.hpp"
#include "../Core/Utilities.hpp"
#include "../Core/CriticalSection.hpp"
#include <memory>
#include <map>
#include <list>
#include <functional>
#include <algorithm>
#include <iterator>
using namespace MCF;

namespace {

typedef std::function<bool (std::intptr_t)> HandlerProc;
typedef std::list<std::shared_ptr<HandlerProc>> HandlerList;

// 使用 Copy-On-Write 策略。
CriticalSection g_csWriteLock;
CriticalSection g_csReadLock;
std::map<std::intptr_t, std::shared_ptr<const HandlerList>> g_mapDelegates;

void UnregisterEventHandler(std::intptr_t nEventId, const HandlerProc *pProc) noexcept {
	CRITICAL_SECTION_SCOPE(g_csWriteLock){
		std::shared_ptr<const HandlerList> pOldList;
		CRITICAL_SECTION_SCOPE(g_csReadLock){
			const auto itList = g_mapDelegates.find(nEventId);
			if(itList != g_mapDelegates.end()){
				pOldList = itList->second;
			}
		}
		if(!pOldList){
			return;
		}
		auto it = std::find_if(
			pOldList->cbegin(),
			pOldList->cend(),
			[&](const std::shared_ptr<HandlerProc> &sp) noexcept {
				return sp.get() == pProc;
			}
		);
		if(it == pOldList->cend()){
			return;
		}

		auto pNewList(std::make_shared<HandlerList>());
		std::copy(pOldList->cbegin(), it, std::back_inserter(*pNewList));
		std::copy(++it, pOldList->cend(), std::back_inserter(*pNewList));

		CRITICAL_SECTION_SCOPE(g_csReadLock){
			const auto itList = g_mapDelegates.find(nEventId);

			ASSERT(itList != g_mapDelegates.end());

			if(pNewList->empty()){
				g_mapDelegates.erase(itList);
			} else {
				itList->second = std::move(pNewList);
			}
		}
	}
}

class HandlerHolder : NO_COPY {
private:
	const std::intptr_t xm_nEventId;
	const HandlerProc *xm_pProc;

public:
	HandlerHolder(std::intptr_t nEventId, const HandlerProc *pProc) noexcept
		: xm_nEventId(nEventId)
		, xm_pProc(pProc)
	{
	}
	~HandlerHolder(){
		UnregisterEventHandler(xm_nEventId, xm_pProc);
	}
};

}

namespace MCF {

std::shared_ptr<void> RegisterEventHandler(std::intptr_t nEventId, std::function<bool (std::intptr_t)> fnHandler){
	std::shared_ptr<void> pRet;
	CRITICAL_SECTION_SCOPE(g_csWriteLock){
		std::shared_ptr<const HandlerList> pOldList;
		CRITICAL_SECTION_SCOPE(g_csReadLock){
			const auto itList = g_mapDelegates.find(nEventId);
			if(itList != g_mapDelegates.end()){
				pOldList = itList->second;
			}
		}

		std::shared_ptr<HandlerList> pNewList;
		if(pOldList){
			pNewList = std::make_shared<HandlerList>(*pOldList);
		} else {
			pNewList = std::make_shared<HandlerList>();
		}
		pNewList->emplace_front(std::make_shared<HandlerProc>(std::move(fnHandler)));
		const HandlerProc *const pNewHandler = pNewList->front().get();

		CRITICAL_SECTION_SCOPE(g_csReadLock){
			g_mapDelegates[nEventId] = std::move(pNewList);
		}
		pRet = std::make_shared<HandlerHolder>(nEventId, pNewHandler);
	}
	return std::move(pRet);
}
void RaiseEvent(std::intptr_t nEventId, std::intptr_t nContext){
	std::shared_ptr<const HandlerList> pList;
	CRITICAL_SECTION_SCOPE(g_csReadLock){
		const auto itList = g_mapDelegates.find(nEventId);
		if(itList != g_mapDelegates.end()){
			pList = itList->second;
		}
	}
	if(!pList){
		return;
	}
	for(const auto &pHandler : *pList){
		if((*pHandler)(nContext)){
			break;
		}
	}
}

}
