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
			std::list<std::shared_ptr<std::function<bool(std::uintptr_t)>>>
		> xm_mapHandlers;
	public:
		EventHandlerHolder RegisterHandler(std::uintptr_t uEventId, std::function<bool(std::uintptr_t)> &&fnHandler){
			auto pHandler = std::make_shared<std::function<bool(std::uintptr_t)>>(std::move(fnHandler));
			const auto pRaw = pHandler.get();
			xm_mapHandlers[uEventId].emplace_front(std::move(pHandler));
			return EventHandlerHolder(std::make_pair(uEventId, pRaw));
		}
		void UnregisterEventHandler(const __MCF::EVENT_HANDLER_HANDLE &Internal) noexcept {
			if(Internal.second){
				const auto it = xm_mapHandlers.find(Internal.first);
				if(it != xm_mapHandlers.end()){
					auto &lstHandlers = it->second;
					for(auto it2 = lstHandlers.cbegin(); it2 != lstHandlers.cend(); ++it2){
						if(it2->get() == Internal.second){
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
				for(const auto &pHandler : lstHandlers){
					if((*pHandler)(uContext)){
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
	void UnregisterEventHandler(const EVENT_HANDLER_HANDLE &Internal) noexcept {
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
			pNewDelegate->UnregisterEventHandler(Internal);
			CRITICAL_SECTION_SCOPE(g_csReadLock){
				g_pDelegate = std::move(pNewDelegate);
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
			g_pDelegate = std::move(pNewDelegate);
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
