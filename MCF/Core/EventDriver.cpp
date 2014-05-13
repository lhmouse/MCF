// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "EventDriver.hpp"
#include "MultiIndexedMap.hpp"
#include "VVector.hpp"
#include "Utilities.hpp"
#include "../Thread/CriticalSection.hpp"
//#include "../Thread/ReadWriteLock.hpp"
#include <memory>
using namespace MCF;

#define MCF_READER_SCOPE	MCF_CRIT_SECT_SCOPE
#define MCF_WRITER_SCOPE	MCF_CRIT_SECT_SCOPE

namespace {

// 使用写时拷贝策略。
typedef VVector<std::shared_ptr<EventHandlerProc>, 0x400>	HandlerList;
typedef MultiIndexedMap<HandlerList, WideString>			HandlerMap;

const auto g_pLock	= CriticalSection::Create();
HandlerMap g_mapHandlerList;

class HandlerHolder : NO_COPY {
private:
	const WideString *xm_pwcsName;
	const EventHandlerProc *xm_pfnProc;

public:
	HandlerHolder(const WideStringObserver &wsoName, EventHandlerProc &&fnProc){
		MCF_WRITER_SCOPE(g_pLock){
			auto pNode = g_mapHandlerList.Find<0>(wsoName);
			if(!pNode){
				pNode = g_mapHandlerList.Insert(HandlerList(), wsoName);
			}

			auto &vecHandlerList = pNode->GetElement();
			const auto &pNewHandler = vecHandlerList.Push(std::make_shared<EventHandlerProc>(std::move(fnProc)));

			xm_pwcsName = &(pNode->GetIndex<0>());
			xm_pfnProc = pNewHandler.get();
		}
	}
	~HandlerHolder() noexcept {
		ASSERT_NOEXCEPT_BEGIN
		{
			MCF_WRITER_SCOPE(g_pLock){
				auto pNode = g_mapHandlerList.Find<0>(*xm_pwcsName);
				ASSERT(pNode);

				auto &vecHandlerList = pNode->GetElement();

				auto ppfnBegin = vecHandlerList.GetBegin();
				const auto ppfnEnd = vecHandlerList.GetEnd();
				for(;;){
					if(ppfnBegin == ppfnEnd){
						break;
					}
					if(ppfnBegin->get() == xm_pfnProc){
						break;
					}
					++ppfnBegin;
				}
				ASSERT(ppfnBegin != ppfnEnd);

				auto ppfnNext = ppfnBegin;
				for(;;){
					++ppfnNext;
					if(ppfnNext == ppfnEnd){
						break;
					}
					*ppfnBegin = std::move(*ppfnNext);
					ppfnBegin = ppfnNext;
				}
				vecHandlerList.Pop();
			}
		}
		ASSERT_NOEXCEPT_END
	}
};

}

namespace MCF {

std::shared_ptr<void> RegisterEventHandler(const WideStringObserver &wsoName, EventHandlerProc fnProc){
	return std::make_shared<HandlerHolder>(wsoName, std::move(fnProc));
}
void TriggerEvent(const WideStringObserver &wsoName, std::uintptr_t uParam1, std::uintptr_t uParam2){
	HandlerList vecHandlerList;
	MCF_READER_SCOPE(g_pLock){
		const auto pNode = g_mapHandlerList.Find<0>(wsoName);
		if(pNode){
			vecHandlerList = pNode->GetElement();
		}
	}

	const auto ppfnBegin = vecHandlerList.GetBegin();
	auto ppfnEnd = vecHandlerList.GetEnd();
	while(ppfnEnd != ppfnBegin){
		if((**--ppfnEnd)(uParam1, uParam2)){
			break;
		}
	}
}

}
