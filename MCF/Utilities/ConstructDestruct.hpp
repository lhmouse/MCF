// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_CONSTRUCT_DESTRUCT_HPP_
#define MCF_CONSTRUCT_DESTRUCT_HPP_

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
inline __attribute__((__always_inline__, __returns_nonnull__))
void *operator new(
	std::size_t, void *p,
	const ::MCF::Impl::DirectConstructTag &
){
	return p;
}
inline __attribute__((__always_inline__))
void operator delete(
	void *, void *,
	const ::MCF::Impl::DirectConstructTag &
) noexcept {
}

namespace MCF {

namespace Impl {
	template<typename Object>
	struct DirectConstructor {
		template<typename ...Params>
		static Object *Construct(Object *pObject, Params &&...vParams)
			noexcept(std::is_nothrow_constructible<Object, Params &&...>::value)
		{
			return ::new(pObject, DirectConstructTag()) Object(std::forward<Params>(vParams)...);
		}
		static void Destruct(Object *pObject)
			noexcept(std::is_nothrow_destructible<Object>::value)
		{
			pObject->~Object();
		}
	};
}

template<typename Object, typename ...Params>
inline __attribute__((__returns_nonnull__))
Object *Construct(Object *pObject, Params &&...vParams)
	noexcept(std::is_nothrow_constructible<Object, Params &&...>::value)
{
	return Impl::DirectConstructor<Object>::template Construct<Params &&...>(
		pObject, std::forward<Params>(vParams)...
	);
}
template<typename Object>
inline void Destruct(Object *pObject)
	noexcept(std::is_nothrow_destructible<Object>::value)
{
	ASSERT(pObject);

	Impl::DirectConstructor<Object>::Destruct(pObject);
}

}

#define FRIEND_CONSTRUCT_DESTRUCT(type)		friend class ::MCF::Impl::DirectConstructor<type>

#endif
