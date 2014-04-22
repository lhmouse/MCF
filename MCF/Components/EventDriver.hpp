// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef MCF_EVENT_DRIVER_HPP_
#define MCF_EVENT_DRIVER_HPP_

#include <functional>
#include <memory>
#include <cstdint>

namespace MCF {

extern std::shared_ptr<void> RegisterEventHandler(std::intptr_t nEventId, std::function<bool (std::intptr_t)> fnHandler);
extern void RaiseEvent(std::intptr_t nEventId, std::intptr_t nContext);

}

#endif
