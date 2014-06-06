// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef MCF_MONITOR_PTR_HPP_
#define MCF_MONITOR_PTR_HPP_

#include "CriticalSection.hpp"
#include "../Core/Utilities.hpp"
#include <utility>
#include <type_traits>
#include <cstddef>

namespace MCF {

namespace Impl {
	template<typename Ty, typename = void>
	struct ForwardArrowOperatorHelper {
		auto operator()(Ty &vSrc) const noexcept {
			return std::addressof(vSrc);
		}
	};
	template<typename Ty>
	struct ForwardArrowOperatorHelper<
		Ty, decltype(std::declval<Ty>().operator->(), (void)0)
	>{
		auto operator()(Ty &vSrc) const noexcept {
			return vSrc.operator->();
		}
	};

	template<typename Ty>
	auto ForwardArrowOperator(Ty &vObject) noexcept {
		return ForwardArrowOperatorHelper<Ty>()(vObject);
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
		: xm_vObject	(std::forward<Params_t>(vParams)...)
		, xm_pcsMutex	(CriticalSection::Create())
		, xm_vLock		(xm_pcsMutex.get(), false)
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
