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
	typedef std::pair<std::uintptr_t, void *> EVENT_HANDLER_HANDLE;

	extern void UnregisterEventHandler(const EVENT_HANDLER_HANDLE &Internal) noexcept;

	struct EventHandlerDeleter {
		constexpr EVENT_HANDLER_HANDLE operator()() const noexcept {
			return std::make_pair(0, nullptr);
		}
		void operator()(const EVENT_HANDLER_HANDLE &Internal) const noexcept {
			UnregisterEventHandler(Internal);
		}
	};
}

typedef UniqueHandle<__MCF::EVENT_HANDLER_HANDLE, __MCF::EventHandlerDeleter> EventHandlerHolder;

extern EventHandlerHolder RegisterEventHandler(std::uintptr_t uEventId, std::function<bool(std::uintptr_t)> fnHandler);
extern void RaiseEvent(std::uintptr_t uEventId, std::uintptr_t uContext);

}

#endif
