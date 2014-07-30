// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_SHARED_HANDLE_IMPL_HPP_
#define MCF_SHARED_HANDLE_IMPL_HPP_

#include "UniqueHandle.hpp"
#include "Utilities.hpp"

namespace MCF {

namespace Impl {
	class RefCountBase {
	protected:
		std::size_t xm_uCount;

	protected:
		constexpr RefCountBase() noexcept
			: xm_uCount(1)
		{
		}
		~RefCountBase() noexcept {
			ASSERT(xm_uCount == 0);
		}
	};

	class AtomicRefCount : private RefCountBase {
	public:
		std::size_t Get() const noexcept {
			return __atomic_load_n(&xm_uCount, __ATOMIC_RELAXED);
		}
		std::size_t Increment() noexcept {
			std::size_t uOld = __atomic_load_n(&xm_uCount, __ATOMIC_RELAXED);
			for(;;){
				if(uOld == 0){
					return 0;
				}
				if(EXPECT_NOT(__atomic_compare_exchange_n(
					&xm_uCount, &uOld, uOld + 1,
					false, __ATOMIC_RELAXED, __ATOMIC_RELAXED
				))){
					return uOld + 1;
				}
			}
		}
		std::size_t Decrement() noexcept {
			ASSERT(Get() != 0);

			return __atomic_sub_fetch(&xm_uCount, 1, __ATOMIC_RELAXED);
		}
		void Reset() noexcept {
			ASSERT(Get() == 0);

			__atomic_store_n(&xm_uCount, 1, __ATOMIC_RELAXED);
		}
	};
	class NonAtomicRefCount : private RefCountBase {
	public:
		std::size_t Get() const noexcept {
			return xm_uCount;
		}
		std::size_t Increment() noexcept {
			if(xm_uCount != 0){
				++xm_uCount;
			}
			return xm_uCount;
		}
		std::size_t Decrement() noexcept {
			ASSERT(Get() != 0);

			return --xm_uCount;
		}
		void Reset() noexcept {
			ASSERT(Get() == 0);

			xm_uCount = 1;
		}
	};

	template<class Closer, class RefCount>
	struct SharedNode {
		UniqueHandle<Closer> m_hObject;
		RefCount m_rcWeak;
		RefCount m_rcStrong;

		explicit SharedNode(UniqueHandle<Closer> &&hObject) noexcept
			: m_hObject(std::move(hObject))
		{
		}
	};

	template<class Closer, class RefCount>
	class SharedHandleTemplate;

	template<class Closer, class RefCount>
	class WeakHandleImpl {
		template<class, class>
		friend class SharedHandleTemplate;

	protected:
		typedef SharedNode<Closer, RefCount> xSharedNode;

	public:
		typedef decltype(Closer()()) Handle;

		static_assert(std::is_scalar<Handle>::value, "Handle must be a scalar type.");
		static_assert(noexcept(Closer()(Handle())), "Handle closer must not throw.");

		using WeakHandle	= WeakHandleImpl;
		using SharedHandle	= SharedHandleTemplate<Closer, RefCount>;
		using UniqueHandle	= UniqueHandle<Closer>;

	protected:
		xSharedNode *xm_pNode;

	public:
		constexpr WeakHandleImpl() noexcept
			: xm_pNode(nullptr)
		{
		}
		WeakHandleImpl(const WeakHandle &rhs) noexcept
			: WeakHandleImpl()
		{
			xJoin(rhs.pNode);
		}
		WeakHandleImpl(WeakHandle &&rhs) noexcept
			: xm_pNode(std::exchange(rhs.xm_pNode, nullptr))
		{
		}
		WeakHandleImpl &operator=(const WeakHandle &rhs) noexcept {
			Reset(rhs);
			return *this;
		}
		WeakHandleImpl &operator=(WeakHandle &&rhs) noexcept {
			Reset(std::move(rhs));
			return *this;
		}
		~WeakHandleImpl() noexcept {
			Reset();
		}

	private:
		void xTidy() noexcept {
			if(xm_pNode && (xm_pNode->m_rcWeak.Decrement() == 0)){
				delete xm_pNode;
			}
			xm_pNode = nullptr;
		}
		void xJoin(xSharedNode *pNode) noexcept {
			if(pNode && (pNode->m_rcWeak.Increment() != 0)){
				xm_pNode = pNode;
			}
		}

	public:
		std::size_t GetWeakRefCount() const noexcept {
			return xm_pNode ? (xm_pNode->m_rcWeak.Get()) : 0u;
		}
		std::size_t GetRefCount() const noexcept {
			return xm_pNode ? (xm_pNode->m_rcStrong.Get()) : 0u;
		}

		bool IsLockable() const noexcept {
			return GetWeakRefCount() != 0;
		}
		SharedHandle Lock() const noexcept;

		void Reset() noexcept {
			xTidy();
		}
		void Reset(const WeakHandle &rhs) noexcept {
			xTidy();
			xJoin(rhs.xm_pNode);
		}
		void Reset(WeakHandle &&rhs) noexcept {
			xTidy();
			xm_pNode = std::exchange(rhs.xm_pNode, nullptr);
		}

		void Swap(WeakHandle &rhs) noexcept {
			std::swap(xm_pNode, rhs.xm_pNode);
		}

	public:
		bool operator==(const WeakHandle &rhs) const noexcept {
			return xm_pNode == rhs.xm_pNode;
		}
		bool operator!=(const WeakHandle &rhs) const noexcept {
			return xm_pNode != rhs.xm_pNode;
		}
		bool operator<(const WeakHandle &rhs) const noexcept {
			return xm_pNode < rhs.xm_pNode;
		}
		bool operator<=(const WeakHandle &rhs) const noexcept {
			return xm_pNode <= rhs.xm_pNode;
		}
		bool operator>(const WeakHandle &rhs) const noexcept {
			return xm_pNode > rhs.xm_pNode;
		}
		bool operator>=(const WeakHandle &rhs) const noexcept {
			return xm_pNode >= rhs.xm_pNode;
		}
	};

	template<class Closer, class RefCount>
	class SharedHandleTemplate : public WeakHandleImpl<Closer, RefCount> {
	public:
		using WeakHandle	= WeakHandleImpl<Closer, RefCount>;
		using SharedHandle	= SharedHandleTemplate;
		using UniqueHandle	= UniqueHandle<Closer>;

	protected:
		using typename WeakHandle::xSharedNode;

	public:
		using typename WeakHandle::Handle;

	public:
		constexpr SharedHandleTemplate() noexcept
			: WeakHandle()
		{
		}
		SharedHandleTemplate(Handle hObject)
			: WeakHandle()
		{
			Reset(hObject);
		}
		SharedHandleTemplate(UniqueHandle &&rhs)
			: WeakHandle()
		{
			Reset(std::move(rhs));
		}
		SharedHandleTemplate(const WeakHandle &rhs) noexcept
			: WeakHandle()
		{
			xJoin(rhs.xm_pNode);
		}
		SharedHandleTemplate(const SharedHandle &rhs) noexcept
			: WeakHandle()
		{
			xJoin(rhs.xm_pNode);
		}
		SharedHandleTemplate(SharedHandle &&rhs) noexcept
			: WeakHandle()
		{
			std::swap(WeakHandle::xm_pNode, rhs.WeakHandle::xm_pNode);
		}
		SharedHandleTemplate &operator=(Handle hObject){
			Reset(hObject);
			return *this;
		}
		SharedHandleTemplate &operator=(UniqueHandle &&rhs){
			Reset(std::move(rhs));
			return *this;
		}
		SharedHandleTemplate &operator=(const WeakHandle &rhs) noexcept {
			Reset(rhs);
			return *this;
		}
		SharedHandleTemplate &operator=(const SharedHandle &rhs) noexcept {
			Reset(rhs);
			return *this;
		}
		SharedHandleTemplate &operator=(SharedHandle &&rhs) noexcept {
			Reset(std::move(rhs));
			return *this;
		}
		~SharedHandleTemplate() noexcept {
			Reset();
		}

	private:
		UniqueHandle xTidy() noexcept {
			UniqueHandle hRet;
			if(WeakHandle::xm_pNode){
				if(WeakHandle::xm_pNode->m_rcStrong.Decrement() == 0){
					hRet.Swap(WeakHandle::xm_pNode->m_hObject);
				}
				if(WeakHandle::xm_pNode->m_rcWeak.Decrement() == 0){
					delete WeakHandle::xm_pNode;
				}
				WeakHandle::xm_pNode = nullptr;
			}
			return std::move(hRet);
		}
		void xJoin(xSharedNode *pNode) noexcept {
			if(pNode && (pNode->m_rcWeak.Increment() != 0)){
				if(pNode->m_rcStrong.Increment() != 0){
					WeakHandle::xm_pNode = pNode;
				} else {
					pNode->m_rcWeak.Decrement();
				}
			}
		}

	public:
		bool IsGood() const noexcept {
			return WeakHandle::GetRefCount() != 0;
		}
		Handle Get() const noexcept {
			return (WeakHandle::GetRefCount() != 0) ? WeakHandle::xm_pNode->m_hObject.Get() : Closer()();
		}
		UniqueHandle Release() noexcept {
			return xTidy();
		}

		void Reset() noexcept {
			xTidy();
		}
		void Reset(Handle hObject){
			Reset(UniqueHandle(hObject));
		}
		void Reset(UniqueHandle &&rhs){
			if(!rhs){
				Reset();
				return;
			}

		ASSERT_NOEXCEPT_BEGIN
			if(WeakHandle::xm_pNode){
				if(WeakHandle::xm_pNode->m_rcStrong.Decrement() == 0){
					WeakHandle::xm_pNode->m_hObject.Reset();
				}
				if(WeakHandle::xm_pNode->m_rcWeak.Decrement() == 0){
					WeakHandle::xm_pNode->m_hObject.Reset(std::move(rhs));
					WeakHandle::xm_pNode->m_rcWeak.Reset();
					WeakHandle::xm_pNode->m_rcStrong.Reset();
					return;
				}
				WeakHandle::xm_pNode = nullptr;
			}
		ASSERT_NOEXCEPT_END

			WeakHandle::xm_pNode = new xSharedNode(std::move(rhs));
		}
		void Reset(const WeakHandle &rhs) noexcept {
			Reset(SharedHandleTemplate(rhs));
		}
		void Reset(const SharedHandle &rhs) noexcept {
			xTidy();
			xJoin(rhs.WeakHandle::xm_pNode);
		}
		void Reset(SharedHandle &&rhs) noexcept {
			xTidy();
			WeakHandle::xm_pNode = std::exchange(rhs.WeakHandle::xm_pNode, nullptr);
		}

		void Swap(SharedHandle &rhs) noexcept {
			std::swap(WeakHandle::xm_pNode, rhs.WeakHandle::xm_pNode);
		}

	public:
		explicit operator bool() const noexcept {
			return IsGood();
		}
		explicit operator Handle() const noexcept {
			return Get();
		}

		bool operator==(const SharedHandle &rhs) const noexcept {
			return Get() == rhs.Get();
		}
		bool operator!=(const SharedHandle &rhs) const noexcept {
			return Get() != rhs.Get();
		}
		bool operator<(const SharedHandle &rhs) const noexcept {
			return Get() < rhs.Get();
		}
		bool operator<=(const SharedHandle &rhs) const noexcept {
			return Get() <= rhs.Get();
		}
		bool operator>(const SharedHandle &rhs) const noexcept {
			return Get() > rhs.Get();
		}
		bool operator>=(const SharedHandle &rhs) const noexcept {
			return Get() >= rhs.Get();
		}

		bool operator==(Handle rhs) const noexcept {
			return Get() == rhs;
		}
		bool operator!=(Handle rhs) const noexcept {
			return Get() != rhs;
		}
		bool operator<(Handle rhs) const noexcept {
			return Get() < rhs;
		}
		bool operator<=(Handle rhs) const noexcept {
			return Get() <= rhs;
		}
		bool operator>(Handle rhs) const noexcept {
			return Get() > rhs;
		}
		bool operator>=(Handle rhs) const noexcept {
			return Get() >= rhs;
		}
	};

	template<class Closer, class RefCount>
	SharedHandleTemplate<Closer, RefCount>
		WeakHandleImpl<Closer, RefCount>::Lock() const noexcept
	{
		return SharedHandle(*this);
	}

	template<class Handle, class Closer, class RefCount>
	auto operator==(Handle lhs, const SharedHandleTemplate<Closer, RefCount> &rhs) noexcept
		-> typename std::enable_if<
			std::is_same<Handle, typename SharedHandleTemplate<Closer, RefCount>::Handle>::value, bool
			>::type
	{
		return lhs == rhs.Get();
	}
	template<class Handle, class Closer, class RefCount>
	auto operator!=(Handle lhs, const SharedHandleTemplate<Closer, RefCount> &rhs) noexcept
		-> typename std::enable_if<
			std::is_same<Handle, typename SharedHandleTemplate<Closer, RefCount>::Handle>::value, bool
			>::type
	{
		return lhs != rhs.Get();
	}
	template<class Handle, class Closer, class RefCount>
	auto operator<(Handle lhs, const SharedHandleTemplate<Closer, RefCount> &rhs) noexcept
		-> typename std::enable_if<
			std::is_same<Handle, typename SharedHandleTemplate<Closer, RefCount>::Handle>::value, bool
			>::type
	{
		return lhs < rhs.Get();
	}
	template<class Handle, class Closer, class RefCount>
	auto operator<=(Handle lhs, const SharedHandleTemplate<Closer, RefCount> &rhs) noexcept
		-> typename std::enable_if<
			std::is_same<Handle, typename SharedHandleTemplate<Closer, RefCount>::Handle>::value, bool
			>::type
	{
		return lhs <= rhs.Get();
	}
	template<class Handle, class Closer, class RefCount>
	auto operator>(Handle lhs, const SharedHandleTemplate<Closer, RefCount> &rhs) noexcept
		-> typename std::enable_if<
			std::is_same<Handle, typename SharedHandleTemplate<Closer, RefCount>::Handle>::value, bool
			>::type
	{
		return lhs > rhs.Get();
	}
	template<class Handle, class Closer, class RefCount>
	auto operator>=(Handle lhs, const SharedHandleTemplate<Closer, RefCount> &rhs) noexcept
		-> typename std::enable_if<
			std::is_same<Handle, typename SharedHandleTemplate<Closer, RefCount>::Handle>::value, bool
			>::type
	{
		return lhs >= rhs.Get();
	}
}

}

#endif
