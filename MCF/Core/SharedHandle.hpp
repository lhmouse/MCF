// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef MCF_SHARED_HANDLE_HPP_
#define MCF_SHARED_HANDLE_HPP_

#include "_SharedHandleImpl.hpp"

namespace MCF {

template<class Closer_t>
struct WeakHandle
	: public Impl::WeakHandleImpl<Closer_t, Impl::AtomicRefCount>
{
	using Impl::WeakHandleImpl<Closer_t, Impl::AtomicRefCount>::WeakHandleImpl;
};

template<class Closer_t>
struct SharedHandle
	: public Impl::SharedHandleImpl<Closer_t, Impl::AtomicRefCount>
{
	using Impl::SharedHandleImpl<Closer_t, Impl::AtomicRefCount>::SharedHandleImpl;
};

}

#endif
