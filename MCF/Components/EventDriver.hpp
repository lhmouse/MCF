// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef __MCF_EVENT_DRIVER_HPP__
#define __MCF_EVENT_DRIVER_HPP__

#include "../Core/UniqueHandle.hpp"
#include <functional>
#include <memory>
#include <utility>
#include <cstdint>

namespace MCF {

namespace __MCF {
	extern void UnregisterEventHandler(void *pInternal) noexcept;

	struct EventHandlerDeleter {
		constexpr void *operator()() const noexcept {
			return nullptr;
		}
		void operator()(void *pInternal) const noexcept {
			UnregisterEventHandler(pInternal);
		}
	};
}

typedef UniqueHandle<void *, __MCF::EventHandlerDeleter> EventHandlerHolder;

extern EventHandlerHolder RegisterEventHandler(std::uintptr_t uEventId, std::function<bool(std::uintptr_t)> fnHandler);
extern void RaiseEvent(std::uintptr_t uEventId, std::uintptr_t uContext);

}

#endif
