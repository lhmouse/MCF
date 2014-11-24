// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_UTILITIES_CONSTRUCT_DESTRUCT_HPP_
#define MCF_UTILITIES_CONSTRUCT_DESTRUCT_HPP_

#include "../../MCFCRT/ext/assert.h"
#include <type_traits>
#include <utility>

namespace MCF {

namespace Impl {
	struct DirectConstructTag {
	};
}

}

// 我只能说 GCC 是个白痴！为什么要检查 placement new 的返回值是否为 nullptr？
inline void *operator new(std::size_t, void *p, const ::MCF::Impl::DirectConstructTag &){
	return p;
}
inline void operator delete(void *, void *, const ::MCF::Impl::DirectConstructTag &) noexcept {
}

namespace MCF {

namespace Impl {
	template<typename ObjectT>
	struct DirectConstructor {
		template<typename ...ParamsT>
		static ObjectT *Construct(ObjectT *pObject, ParamsT &&...vParams)
			noexcept(std::is_nothrow_constructible<ObjectT, ParamsT &&...>::value)
		{
			return ::new(pObject, DirectConstructTag()) ObjectT(std::forward<ParamsT>(vParams)...);
		}
		static void Destruct(ObjectT *pObject)
			noexcept(std::is_nothrow_destructible<ObjectT>::value)
		{
			pObject->~ObjectT();
		}
	};
}

template<typename ObjectT, typename ...ParamsT>
inline ObjectT *Construct(ObjectT *pObject, ParamsT &&...vParams)
	noexcept(std::is_nothrow_constructible<ObjectT, ParamsT &&...>::value)
{
	return Impl::DirectConstructor<ObjectT>::template Construct<ParamsT &&...>(
		pObject, std::forward<ParamsT>(vParams)...);
}
template<typename ObjectT>
inline void Destruct(ObjectT *pObject)
	noexcept(std::is_nothrow_destructible<ObjectT>::value)
{
	ASSERT(pObject);

	Impl::DirectConstructor<ObjectT>::Destruct(pObject);
}

}

#define FRIEND_CONSTRUCT_DESTRUCT(type)		friend class ::MCF::Impl::DirectConstructor<type>

#endif
