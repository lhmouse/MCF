// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_MONITOR_PTR_HPP_
#define MCF_MONITOR_PTR_HPP_

#include "CriticalSection.hpp"
#include "../Utilities/Utilities.hpp"
#include <utility>
#include <type_traits>
#include <memory>
#include <cstddef>

namespace MCF {

namespace Impl {
	template<typename Object, typename = void>
	struct ForwardArrowOperatorHelper {
		auto operator()(Object *pObject) const noexcept {
			return pObject;
		}
	};
	template<typename Object>
	struct ForwardArrowOperatorHelper<
		Object, decltype(std::declval<Object *>()->operator->(), (void)0)
	> {
		auto operator()(Object *pObject) const noexcept {
			return pObject->operator->();
		}
	};

	template<typename Object>
	auto ForwardArrowOperator(Object &vObject) noexcept {
		return ForwardArrowOperatorHelper<Object>()(std::addressof(vObject));
	}
}

template<class Object>
class MonitorPtr : NO_COPY {
private:
	template<typename Self>
	class xMonitorHolder : NO_COPY {
		friend MonitorPtr;

	private:
		Self *xm_pOwner;

	private:
		explicit xMonitorHolder(Self *pOwner) noexcept
			: xm_pOwner(pOwner)
		{
			auto vLock = xm_pOwner->xm_pcsMutex->GetLock();
			xm_pOwner->xm_vLock.Join(std::move(vLock));
		}
		xMonitorHolder(xMonitorHolder &&rhs) noexcept
			: xm_pOwner(std::exchange(rhs.xm_pOwner, nullptr))
		{
		}
		xMonitorHolder &operator=(xMonitorHolder &&rhs) noexcept {
			if(xm_pOwner){
				xm_pOwner->xm_vLock.Unlock();
			}
			xm_pOwner = std::exchange(rhs.xm_pOwner, nullptr);
			return *this;
		}

	public:
		~xMonitorHolder() noexcept {
			if(xm_pOwner){
				xm_pOwner->xm_vLock.Unlock();
			}
		}

	public:
		auto operator->() const noexcept {
			return Impl::ForwardArrowOperator(xm_pOwner->xm_vObject);
		}
	};

private:
	const std::unique_ptr<CriticalSection> xm_pcsMutex;
	mutable CriticalSection::Lock xm_vLock;
	Object xm_vObject;

public:
	template<typename ...Params>
	explicit MonitorPtr(Params &&...vParams)
		: xm_pcsMutex	(CriticalSection::Create())
		, xm_vLock		(xm_pcsMutex.get(), false)
		, xm_vObject	(std::forward<Params>(vParams)...)
	{
	}

public:
	auto operator->() const noexcept {
		return xMonitorHolder<const MonitorPtr>(this);
	}
	auto operator->() noexcept {
		return xMonitorHolder<MonitorPtr>(this);
	}
};

}

#endif
