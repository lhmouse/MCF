// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_TIME_HPP_
#define MCF_TIME_HPP_

#include "../Utilities/Utilities.hpp"
#include <functional>
#include <cstdint>

namespace MCF {

// NT 时间戳单位是 1e-7 秒。
// Unix 时间戳单位是秒。
extern std::uint64_t GetNtTime() noexcept;
extern std::uint64_t GetUnixTime() noexcept;
extern std::uint64_t NtTimeFromUnixTime(std::uint64_t u64UnixTime) noexcept;
extern std::uint64_t UnixTimeFromNtTime(std::uint64_t u64NtTime) noexcept;

// 单位是秒。
extern double GetHiResCounter() noexcept;

// 单位是毫秒。
enum : std::uint64_t {
	WAIT_FOREVER = (std::uint64_t)-1
};

template<std::uint64_t GRANULARITY = 0x7FFFFFFFu, typename Function>
bool WaitUntil(Function &&fnCallable, std::uint64_t u64MilliSeconds){
	if(u64MilliSeconds == WAIT_FOREVER){
		for(;;){
			if(fnCallable(GRANULARITY)){
				return true;
			}
		}
	} else {
		const auto u64Until = GetNtTime() / 10000u + u64MilliSeconds;
		auto u64ToWait = u64MilliSeconds;
		for(;;){
			if(u64ToWait > GRANULARITY){
				u64ToWait = GRANULARITY;
			}
			if(fnCallable(u64ToWait)){
				return true;
			}
			const std::uint64_t u64Now = GetNtTime() / 10000u;
			if(u64Until <= u64Now){
				return false;
			}
			u64ToWait = u64Until - u64Now;
		}
	}
}

}

#endif
