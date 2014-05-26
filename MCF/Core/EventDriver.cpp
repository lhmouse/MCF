// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "EventDriver.hpp"
#include "MultiIndexedMap.hpp"
#include "VVector.hpp"
#include "Utilities.hpp"
#include "../Thread/ReaderWriterLock.hpp"
#include <memory>
using namespace MCF;

namespace {

typedef VVector<std::shared_ptr<EventHandlerProc>, 32>	HandlerVector;
typedef MultiIndexedMap<HandlerVector, WideString>		HandlerMap;

const auto g_pLock = ReaderWriterLock::Create();
HandlerMap g_mapHandlerVector;

class HandlerHolder : NO_COPY {
private:
	const WideString *xm_pwcsName;
	const EventHandlerProc *xm_pfnProc;

public:
	HandlerHolder(const WideStringObserver &wsoName, EventHandlerProc &&fnProc){
		const auto vLock = g_pLock->GetWriterLock();

		auto pNode = g_mapHandlerVector.Find<0>(wsoName);
		if(!pNode){
			pNode = g_mapHandlerVector.Insert(HandlerVector(), WideString(wsoName));
		}

		auto &vecHandlers = pNode->GetElement();
		const auto &pNewHandler = vecHandlers.Push(std::make_shared<EventHandlerProc>(std::move(fnProc)));

		xm_pwcsName = &(pNode->GetIndex<0>());
		xm_pfnProc = pNewHandler.get();
	}
	~HandlerHolder() noexcept {
		ASSERT_NOEXCEPT_BEGIN
		{
			const auto vLock = g_pLock->GetWriterLock();

			auto pNode = g_mapHandlerVector.Find<0>(*xm_pwcsName);
			ASSERT(pNode);

			auto &vecHandlers = pNode->GetElement();

			const auto ppfnEnd = vecHandlers.GetEnd();
			auto ppfnCur = ppfnEnd;
			for(;;){
				ASSERT(ppfnCur != vecHandlers.GetBegin());

				--ppfnCur;
				if(ppfnCur->get() == xm_pfnProc){
					break;
				}
			}

			for(auto ppfnNext = ppfnCur + 1; ppfnNext != ppfnEnd; ++ppfnNext){
				std::swap(ppfnNext[-1], ppfnNext[0]);
			}
			vecHandlers.Pop();
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
	HandlerVector vecHandlers;
	{
		const auto vLock = g_pLock->GetReaderLock();

		const auto pNode = g_mapHandlerVector.Find<0>(wsoName);
		if(pNode){
			vecHandlers = pNode->GetElement();
		}
	}

	for(auto ppfnNext = vecHandlers.GetEnd(); ppfnNext != vecHandlers.GetBegin(); --ppfnNext){
		if((*(ppfnNext[-1]))(uParam1, uParam2)){
			break;
		}
	}
}

}
