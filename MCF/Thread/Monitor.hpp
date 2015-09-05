// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_MONITOR_HPP_
#define MCF_THREAD_MONITOR_HPP_

#include "../Utilities/Noncopyable.hpp"
#include <utility>

namespace MCF {

class Mutex;
class RecursiveMutex;
class KernelMutex;

template<class ObjectT, class MutexT>
class MonitorTemplate;

namespace Impl_Monitor {
	template<class ObjectT>
	class MonitorObserverAsReference {
	private:
		ObjectT &$vObject;

	public:
		explicit MonitorObserverAsReference(ObjectT &vObject) noexcept
			: $vObject(vObject)
		{
		}

	public:
		decltype(auto) operator&() const && noexcept {
			return &$vObject;
		}
		operator ObjectT &() const && noexcept {
			return $vObject;
		}
		template<typename ...ArgsT>
		decltype(auto) operator()(ArgsT &&...vArgs) const && {
			return $vObject(std::forward<ArgsT>(vArgs)...);
		}
	};
	template<class ObjectT>
	class MonitorObserverAsPointer {
	private:
		ObjectT &$vObject;

	public:
		explicit MonitorObserverAsPointer(ObjectT &vObject) noexcept
			: $vObject(vObject)
		{
		}

	public:
		ObjectT &operator*() const && noexcept {
			return $vObject;
		}
		ObjectT *operator->() const && noexcept {
			return std::addressof($vObject);
		}
	};
}

template<class ObjectT, class MutexT, class ObserverT>
class MonitorLock : private MutexT::UniqueLock, public ObserverT {
	friend MonitorTemplate<const volatile ObjectT, MutexT>;
	friend MonitorTemplate<const ObjectT, MutexT>;
	friend MonitorTemplate<volatile ObjectT, MutexT>;
	friend MonitorTemplate<ObjectT, MutexT>;

private:
	MonitorLock(typename MutexT::UniqueLock vLock, ObjectT &vObject) noexcept
		: MutexT::UniqueLock(std::move(vLock)), ObserverT(vObject)
	{
	}

	MonitorLock(MonitorLock &&rhs) noexcept = default;

	MonitorLock(const MonitorLock &) = delete;
	MonitorLock &operator=(const MonitorLock &) = delete;
	MonitorLock &operator=(MonitorLock &&) noexcept = delete;
};

template<class ObjectT, class MutexT>
class MonitorTemplate : NONCOPYABLE {
private:
	mutable MutexT $vMutex;
	ObjectT $vObject;

public:
	template<typename ...ParamsT>
	explicit MonitorTemplate(ParamsT &&...vParams)
		: $vObject(std::forward<ParamsT>(vParams)...)
	{
	}

public:
	auto operator*() const noexcept {
		return MonitorLock<const ObjectT, MutexT,
			MonitorObserverAsReference<const ObjectT>>($vMutex.GetLock(), $vObject);
	}
	auto operator*() noexcept {
		return MonitorLock<ObjectT, MutexT,
			MonitorObserverAsReference<ObjectT>>($vMutex.GetLock(), $vObject);
	}

	auto operator->() const noexcept {
		return MonitorLock<const ObjectT,
			MutexT, MonitorObserverAsPointer<const ObjectT>>($vMutex.GetLock(), $vObject);
	}
	auto operator->() noexcept {
		return MonitorLock<ObjectT, MutexT,
			MonitorObserverAsPointer<ObjectT>>($vMutex.GetLock(), $vObject);
	}
};

template<class ObjectT>
using Monitor = MonitorTemplate<ObjectT, Mutex>;
template<class ObjectT>
using RecursiveMonitor = MonitorTemplate<ObjectT, RecursiveMutex>;
template<class ObjectT>
using KernelMonitor = MonitorTemplate<ObjectT, KernelMutex>;

}

#endif
