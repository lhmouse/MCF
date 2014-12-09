// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_MONITOR_PTR_HPP_
#define MCF_THREAD_MONITOR_PTR_HPP_

#include "UserRecursiveMutex.hpp"
#include "../Utilities/Noncopyable.hpp"
#include <utility>
#include <type_traits>
#include <cstddef>

namespace MCF {

template<class ObjectT>
class MonitorPtr : NONCOPYABLE {
private:
	template<typename SelfT>
	class xMonitorHolder {
		friend MonitorPtr;

	private:
		SelfT *const xm_pOwner;
		UserRecursiveMutex::UniqueLock xm_vLock;

	private:
		explicit xMonitorHolder(SelfT *pOwner) noexcept
			: xm_pOwner(pOwner), xm_vLock(pOwner->xm_vMutex)
		{
		}

		xMonitorHolder(xMonitorHolder &&) noexcept = default;

		xMonitorHolder(const xMonitorHolder &) = delete;
		void operator=(const xMonitorHolder &) = delete;
		void operator=(xMonitorHolder &&) = delete;

	public:
		auto &operator*() const noexcept {
			return xm_pOwner->xm_vObject;
		}
		auto operator->() const noexcept {
			return &**this;
		}
	};

private:
	UserRecursiveMutex xm_vMutex;
	ObjectT xm_vObject;

public:
	template<typename ...ParamsT>
	explicit MonitorPtr(ParamsT &&...vParams)
		: xm_vObject(std::forward<ParamsT>(vParams)...)
	{
	}

public:
	auto Get() const noexcept {
		return xMonitorHolder<const MonitorPtr>(this);
	}
	auto Get() noexcept {
		return xMonitorHolder<MonitorPtr>(this);
	}

public:
	auto operator->() const noexcept {
		return Get();
	}
	auto operator->() noexcept {
		return Get();
	}
};

}

#endif
