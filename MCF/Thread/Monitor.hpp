// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_THREAD_MONITOR_HPP_
#define MCF_THREAD_MONITOR_HPP_

#include "../Utilities/Noncopyable.hpp"
#include "../Utilities/AddressOf.hpp"
#include <utility>

namespace MCF {

class Mutex;
class RecursiveMutex;
class KernelMutex;

template<class ObjectT, class MutexT>
class MonitorTemplate;

namespace Impl_Monitor {
	template<class ObjectT>
	class MonitorViewAsReference {
	private:
		ObjectT &x_vObject;

	public:
		explicit MonitorViewAsReference(ObjectT &vObject) noexcept
			: x_vObject(vObject)
		{
		}

	public:
		decltype(auto) operator&() const && noexcept {
			return &x_vObject;
		}
		operator ObjectT &() const && noexcept {
			return x_vObject;
		}
		template<typename ...ArgsT>
		decltype(auto) operator()(ArgsT &&...vArgs) const && {
			return x_vObject(std::forward<ArgsT>(vArgs)...);
		}
	};
	template<class ObjectT>
	class MonitorViewAsPointer {
	private:
		ObjectT &x_vObject;

	public:
		explicit MonitorViewAsPointer(ObjectT &vObject) noexcept
			: x_vObject(vObject)
		{
		}

	public:
		ObjectT &operator*() const && noexcept {
			return x_vObject;
		}
		ObjectT *operator->() const && noexcept {
			return AddressOf(x_vObject);
		}
	};

	template<class ObjectT, class MutexT, class ViewT>
	class MonitorLock : private MutexT::UniqueLock, public ViewT {
		friend MonitorTemplate<const volatile ObjectT, MutexT>;
		friend MonitorTemplate<const ObjectT, MutexT>;
		friend MonitorTemplate<volatile ObjectT, MutexT>;
		friend MonitorTemplate<ObjectT, MutexT>;

	private:
		MonitorLock(typename MutexT::UniqueLock &&vLock, ObjectT &vObject) noexcept
			: MutexT::UniqueLock(std::move(vLock)), ViewT(vObject)
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
		mutable MutexT x_vMutex;
		ObjectT x_vObject;

	public:
		template<typename ...ParamsT>
		explicit MonitorTemplate(ParamsT &&...vParams)
			: x_vObject(std::forward<ParamsT>(vParams)...)
		{
		}

	public:
		Impl_Monitor::MonitorLock<const ObjectT, MutexT, MonitorViewAsReference<const ObjectT>> operator*() const noexcept {
			return Impl_Monitor::MonitorLock<const ObjectT, MutexT, MonitorViewAsReference<const ObjectT>>(x_vMutex.GetLock(), x_vObject);
		}
		Impl_Monitor::MonitorLock<ObjectT, MutexT, MonitorViewAsReference<ObjectT>> operator*() noexcept {
			return Impl_Monitor::MonitorLock<ObjectT, MutexT, MonitorViewAsReference<ObjectT>>(x_vMutex.GetLock(), x_vObject);
		}

		Impl_Monitor::MonitorLock<const ObjectT, MutexT, MonitorViewAsPointer<const ObjectT>> operator->() const noexcept {
			return Impl_Monitor::MonitorLock<const ObjectT, MutexT, MonitorViewAsPointer<const ObjectT>>(x_vMutex.GetLock(), x_vObject);
		}
		Impl_Monitor::MonitorLock<ObjectT, MutexT, MonitorViewAsPointer<ObjectT>> operator->() noexcept {
			return Impl_Monitor::MonitorLock<ObjectT, MutexT, MonitorViewAsPointer<ObjectT>>(x_vMutex.GetLock(), x_vObject);
		}
	};
}

template<class ObjectT>
using Monitor          = Impl_Monitor::MonitorTemplate<ObjectT, Mutex>;
template<class ObjectT>
using RecursiveMonitor = Impl_Monitor::MonitorTemplate<ObjectT, RecursiveMutex>;
template<class ObjectT>
using KernelMonitor    = Impl_Monitor::MonitorTemplate<ObjectT, KernelMutex>;

}

#endif
