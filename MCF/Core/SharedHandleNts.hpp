// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_SHARED_HANDLE_NTS_HPP_
#define MCF_SHARED_HANDLE_NTS_HPP_

#include "_SharedHandleTemplate.hpp"

namespace MCF {

template<class CloserT>
using WeakHandleNts = Impl::WeakHandleImpl<CloserT, Impl::NonAtomicRefCount>;

template<class CloserT>
using SharedHandleNts = Impl::SharedHandleTemplate<CloserT, Impl::NonAtomicRefCount>;

}

#endif
