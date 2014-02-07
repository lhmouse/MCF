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
#include <climits>
using namespace MCF;

namespace {
	class EventDelegate {
	private:
		std::map<
			std::uintptr_t,
			std::list<std::shared_ptr<std::pair<
				std::uintptr_t,
				std::function<bool(std::uintptr_t)>
			>>>
		> xm_mapHandlers;
	public:
		EventHandlerHolder RegisterHandler(std::uintptr_t uEventId, std::function<bool(std::uintptr_t)> &&fnHandler){
			auto pPair = std::make_shared<std::pair<std::uintptr_t, std::function<bool(std::uintptr_t)>>>(uEventId, std::move(fnHandler));
			const auto pRaw = pPair.get();
			xm_mapHandlers[uEventId].emplace_front(std::move(pPair));
			return EventHandlerHolder(pRaw);
		}
		void UnregisterEventHandler(void *pInternal) noexcept {
			if(pInternal){
				const auto pPair = (const std::pair<std::uintptr_t, std::function<bool(std::uintptr_t)>> *)pInternal;
				const auto it = xm_mapHandlers.find(pPair->first);
				if(it != xm_mapHandlers.end()){
					auto &lstHandlers = it->second;
					for(auto it2 = lstHandlers.cbegin(); it2 != lstHandlers.cend(); ++it2){
						if(it2->get() == pPair){
							lstHandlers.erase(it2);
							break;
						}
					}
				}
			}
		}

		void RaiseEvent(std::uintptr_t uEventId, std::uintptr_t uContext) const {
			const auto it = xm_mapHandlers.find(uEventId);
			if(it != xm_mapHandlers.end()){
				auto &lstHandlers = it->second;
				for(const auto &pPair : lstHandlers){
					if(pPair->second(uContext)){
						break;
					}
				}
			}
		}
	};

	// 使用 Copy-On-Write 策略。
	CriticalSection g_csWriteLock;
	CriticalSection g_csReadLock;
	std::shared_ptr<const EventDelegate> g_pDelegate;
}

namespace MCF {

namespace __MCF {
	void UnregisterEventHandler(void *pInternal) noexcept {
		CRITICAL_SECTION_SCOPE(g_csWriteLock){
			std::shared_ptr<const EventDelegate> pDelegate;
			CRITICAL_SECTION_SCOPE(g_csReadLock){
				pDelegate = g_pDelegate;
			}
			std::shared_ptr<EventDelegate> pNewDelegate;
			if(pDelegate){
				pNewDelegate.reset(new EventDelegate(*pDelegate));
			} else {
				pNewDelegate.reset(new EventDelegate);
			}
			pNewDelegate->UnregisterEventHandler(pInternal);
			CRITICAL_SECTION_SCOPE(g_csReadLock){
				g_pDelegate.swap(pNewDelegate);
			}
		}
	}
}

EventHandlerHolder RegisterEventHandler(std::uintptr_t uEventId, std::function<bool(std::uintptr_t)> fnHandler){
	EventHandlerHolder ret;
	CRITICAL_SECTION_SCOPE(g_csWriteLock){
		std::shared_ptr<const EventDelegate> pDelegate;
		CRITICAL_SECTION_SCOPE(g_csReadLock){
			pDelegate = g_pDelegate;
		}
		std::shared_ptr<EventDelegate> pNewDelegate;
		if(pDelegate){
			pNewDelegate.reset(new EventDelegate(*pDelegate));
		} else {
			pNewDelegate.reset(new EventDelegate);
		}
		ret = pNewDelegate->RegisterHandler(uEventId, std::move(fnHandler));
		CRITICAL_SECTION_SCOPE(g_csReadLock){
			g_pDelegate.swap(pNewDelegate);
		}
	}
	return std::move(ret);
}
void RaiseEvent(std::uintptr_t uEventId, std::uintptr_t uContext){
	std::shared_ptr<const EventDelegate> pDelegate;
	CRITICAL_SECTION_SCOPE(g_csReadLock){
		pDelegate = g_pDelegate;
	}
	if(pDelegate){
		pDelegate->RaiseEvent(uEventId, uContext);
	}
}

}
