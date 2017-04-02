// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "PolyIntrusivePtr.hpp"
#include "UniquePtr.hpp"

namespace MCF {

namespace Impl_PolyIntrusivePtr {
	Deletable::~Deletable(){
	}

	Unknown::~Unknown(){
	}
}

template class IntrusivePtr     <Impl_PolyIntrusivePtr::Unknown, DefaultDeleter<Impl_PolyIntrusivePtr::Deletable>>;
template class IntrusiveWeakPtr <Impl_PolyIntrusivePtr::Unknown, DefaultDeleter<Impl_PolyIntrusivePtr::Deletable>>;

}
