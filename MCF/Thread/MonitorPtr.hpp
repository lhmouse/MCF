// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014 LH_Mouse. All wrongs reserved.

#ifndef MCF_MONITOR_PTR_HPP_
#define MCF_MONITOR_PTR_HPP_

#include "CriticalSection.hpp"
#include "../Core/Utilities.hpp"
#include <utility>
#include <type_traits>
#include <cstddef>

namespace MCF {

namespace Impl {
	template<typename Object_t, typename = void>
	struct ForwardArrowOperatorHelper {
		auto operator()(Object_t *pObject) const noexcept {
			return pObject;
		}
	};
	template<typename Object_t>
	struct ForwardArrowOperatorHelper<
		Object_t, decltype(std::declval<Object_t *>()->operator->(), (void)0)
	> {
		auto operator()(Object_t *pObject) const noexcept {
			return pObject->operator->();
		}
	};

	template<typename Object_t>
	auto ForwardArrowOperator(Object_t &vObject) noexcept {
		return ForwardArrowOperatorHelper<Object_t>()(
			(Object_t *)&reinterpret_cast<const volatile char &>(vObject)
		);
	}
}

template<class Object_t>
class MonitorPtr : NO_COPY {
private:
	template<typename Self_t>
	class xMonitorHolder : NO_COPY {
		friend MonitorPtr;

	private:
		Self_t *xm_pOwner;

	private:
		explicit xMonitorHolder(Self_t *pOwner) noexcept
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
	Object_t xm_vObject;

public:
	template<typename... Params_t>
	explicit MonitorPtr(Params_t &&... vParams)
		: xm_pcsMutex	(CriticalSection::Create())
		, xm_vLock		(xm_pcsMutex.get(), false)
		, xm_vObject	(std::forward<Params_t>(vParams)...)
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
