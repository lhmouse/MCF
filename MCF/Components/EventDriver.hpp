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
	typedef std::pair<std::intptr_t, const void *> EventHandlerHandle;

	extern void UnregisterEventHandler(const EventHandlerHandle &Handle) noexcept;

	struct EventHandlerDeleter {
		constexpr EventHandlerHandle operator()() const noexcept {
			return EventHandlerHandle();
		}
		void operator()(const EventHandlerHandle &Handle) const noexcept {
			UnregisterEventHandler(Handle);
		}
	};
}

typedef UniqueHandle<__MCF::EventHandlerDeleter> EventHandlerHolder;

extern EventHandlerHolder RegisterEventHandler(std::intptr_t nEventId, std::function<bool (std::intptr_t)> fnHandler);
extern void RaiseEvent(std::intptr_t nEventId, std::intptr_t nContext);

}

#endif
