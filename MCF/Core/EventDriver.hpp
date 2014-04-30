// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef MCF_EVENT_DRIVER_HPP_
#define MCF_EVENT_DRIVER_HPP_

#include "String.hpp"
#include <functional>
#include <memory>
#include <cstdint>

namespace MCF {

// 返回 true 则表示事件已被处理，不会继续传递。
// 否则事件传递给前一个注册的响应器。
typedef std::function<bool (std::uintptr_t, std::uintptr_t)> EventHandlerProc;

std::shared_ptr<void> RegisterEventHandler(const WideStringObserver &wsoName, EventHandlerProc fnProc);
void TriggerEvent(const WideStringObserver &wsoName, std::uintptr_t uParam1 = 0, std::uintptr_t uParam2 = 0);

}

#endif
