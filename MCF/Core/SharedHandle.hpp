// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef MCF_SHARED_HANDLE_HPP_
#define MCF_SHARED_HANDLE_HPP_

#include "_SharedHandleTemplate.hpp"

namespace MCF {

template<class Closer_t>
using WeakHandle = Impl::WeakHandleImpl<Closer_t, Impl::AtomicRefCount>;

template<class Closer_t>
using SharedHandle = Impl::SharedHandleTemplate<Closer_t, Impl::AtomicRefCount>;

}

#endif
