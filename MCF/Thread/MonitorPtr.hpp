// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_MONITOR_PTR_HPP_
#define MCF_THREAD_MONITOR_PTR_HPP_

#include "CriticalSection.hpp"
#include "../Utilities/NoCopy.hpp"
#include <utility>
#include <type_traits>
#include <memory>
#include <cstddef>

namespace MCF {

namespace Impl {
	template<typename ObjectT, typename = void>
	struct ForwardArrowOperatorHelper {
		auto operator()(ObjectT *pObject) const noexcept {
			return pObject;
		}
	};
	template<typename ObjectT>
	struct ForwardArrowOperatorHelper<
		ObjectT, decltype(std::declval<ObjectT *>()->operator->(), (void)0)
	> {
		auto operator()(ObjectT *pObject) const noexcept {
			return pObject->operator->();
		}
	};

	template<typename ObjectT>
	auto ForwardArrowOperator(ObjectT &vObject) noexcept {
		return ForwardArrowOperatorHelper<ObjectT>()(std::addressof(vObject));
	}
}

template<class ObjectT>
class MonitorPtr : NO_COPY {
private:
	template<typename SelfT>
	class xMonitorHolder : NO_COPY {
		friend MonitorPtr;

	private:
		SelfT *xm_pOwner;

	private:
		explicit xMonitorHolder(SelfT *pOwner) noexcept
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
			ASSERT(&rhs != this);

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
	ObjectT xm_vObject;

public:
	template<typename ...ParamsT>
	explicit MonitorPtr(ParamsT &&...vParams)
		: xm_pcsMutex	(CriticalSection::Create())
		, xm_vLock		(xm_pcsMutex.get(), false)
		, xm_vObject	(std::forward<ParamsT>(vParams)...)
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
