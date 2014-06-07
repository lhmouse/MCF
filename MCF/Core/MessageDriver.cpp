// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "MessageDriver.hpp"
#include "MultiIndexedMap.hpp"
#include "VVector.hpp"
#include "Utilities.hpp"
#include "../Thread/ReaderWriterLock.hpp"
#include <memory>
using namespace MCF;

namespace {

typedef Message::HandlerProc HandlerProc;

typedef VVector<std::shared_ptr<HandlerProc>, 32>	HandlerVector;
typedef MultiIndexedMap<HandlerVector, Utf8String>	HandlerMap;

const auto g_pLock = ReaderWriterLock::Create();
HandlerMap g_mapHandlerVector;

class HandlerHolder : NO_COPY {
private:
	const Utf8String *xm_pu8csName;
	const HandlerProc *xm_pfnProc;

public:
	HandlerHolder(const Utf8StringObserver &u8soName, HandlerProc &&fnProc){
		const auto vLock = g_pLock->GetWriterLock();

		auto pNode = g_mapHandlerVector.Find<0>(u8soName);
		if(!pNode){
			pNode = g_mapHandlerVector.Insert(HandlerVector(), Utf8String(u8soName));
		}

		auto &vecHandlers = pNode->GetElement();
		const auto &pNewHandler = vecHandlers.Push(std::make_shared<HandlerProc>(std::move(fnProc)));

		xm_pu8csName = &(pNode->GetIndex<0>());
		xm_pfnProc = pNewHandler.get();
	}
	~HandlerHolder() noexcept {
		ASSERT_NOEXCEPT_BEGIN
		{
			const auto vLock = g_pLock->GetWriterLock();

			auto pNode = g_mapHandlerVector.Find<0>(*xm_pu8csName);
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

// 静态成员函数。
std::shared_ptr<void> Message::RegisterHandler(const Utf8StringObserver &u8soName, HandlerProc fnProc){
	return std::make_shared<HandlerHolder>(u8soName, std::move(fnProc));
}

// 构造函数和析构函数。
Message::Message(Utf8String u8sName) noexcept
	: m_u8sName(std::move(u8sName))
{
}
Message::~Message() noexcept {
}

// 其他非静态成员函数。
void Message::Dispatch() const {
	HandlerVector vecHandlers;
	{
		const auto vLock = g_pLock->GetReaderLock();

		const auto pNode = g_mapHandlerVector.Find<0>(m_u8sName);
		if(pNode){
			vecHandlers = pNode->GetElement();
		}
	}

	for(auto ppfnNext = vecHandlers.GetEnd(); ppfnNext != vecHandlers.GetBegin(); --ppfnNext){
		if((*(ppfnNext[-1]))(*this)){
			break;
		}
	}
}

