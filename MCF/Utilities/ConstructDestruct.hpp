// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_UTILITIES_CONSTRUCT_DESTRUCT_HPP_
#define MCF_UTILITIES_CONSTRUCT_DESTRUCT_HPP_

#include <type_traits>
#include <utility>

namespace MCF {

namespace Impl {
	struct DirectConstructTag {
	};
}

}

// FIXME: 我只能说 GCC 是个白痴！为什么要检查 placement new 的返回值是否为 nullptr？
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
		static void Construct(ObjectT *pObject, ParamsT &&...vParams){
			::new(pObject, DirectConstructTag()) ObjectT(std::forward<ParamsT>(vParams)...);
		}
		template<typename ...ParamsT>
		static void DefaultConstruct(ObjectT *pObject, ParamsT &&...vParams){
			if(std::is_trivial<ObjectT>::value && (sizeof...(vParams) == 0)){
#ifndef NDEBUG
				__builtin_memset(pObject, 0xCC, sizeof(ObjectT));
#endif
			} else {
				Construct(pObject, std::forward<ParamsT>(vParams)...);
			}
		}
		static void Destruct(ObjectT *pObject){
			pObject->~ObjectT();
		}
	};
}

template<typename ObjectT, typename ...ParamsT>
inline void Construct(ObjectT *pObject, ParamsT &&...vParams)
	noexcept(std::is_nothrow_constructible<ObjectT, ParamsT &&...>::value)
{
	Impl::DirectConstructor<ObjectT>::Construct(pObject, std::forward<ParamsT>(vParams)...);
}
template<typename ObjectT, typename ...ParamsT>
inline void DefaultConstruct(ObjectT *pObject, ParamsT &&...vParams)
	noexcept(std::is_nothrow_constructible<ObjectT, ParamsT &&...>::value)
{
	Impl::DirectConstructor<ObjectT>::DefaultConstruct(pObject, std::forward<ParamsT>(vParams)...);
}
template<typename ObjectT>
inline void Destruct(ObjectT *pObject)
	noexcept(std::is_nothrow_destructible<ObjectT>::value)
{
	Impl::DirectConstructor<ObjectT>::Destruct(pObject);
}

template<typename ObjectT, typename ...ParamsT>
inline void ConstructArray(ObjectT *pBegin, std::size_t uCount, const ParamsT &...vParams)
	noexcept(std::is_nothrow_constructible<ObjectT, const ParamsT &...>::value)
{
	static_assert(std::is_nothrow_destructible<ObjectT>::value, "ObjectT shall be nothrow destructible.");

	const auto pEnd = pBegin + uCount;
	auto pCur = pBegin;
	try {
		while(pCur != pEnd){
			Construct(pCur, vParams...);
			++pCur;
		}
	} catch(...){
		while(pCur != pBegin){
			--pCur;
			Destruct(pCur);
		}
		throw;
	}
}
template<typename ObjectT, typename ...ParamsT>
inline void DefaultConstructArray(ObjectT *pBegin, std::size_t uCount, const ParamsT &...vParams)
	noexcept(std::is_nothrow_constructible<ObjectT, const ParamsT &...>::value)
{
	static_assert(std::is_nothrow_destructible<ObjectT>::value, "ObjectT shall be nothrow destructible.");

	const auto pEnd = pBegin + uCount;
	auto pCur = pBegin;
	try {
		while(pCur != pEnd){
			DefaultConstruct(pCur, vParams...);
			++pCur;
		}
	} catch(...){
		while(pCur != pBegin){
			--pCur;
			Destruct(pCur);
		}
		throw;
	}
}
template<typename ObjectT>
inline void DestructArray(ObjectT *pBegin, std::size_t uCount) noexcept {
	static_assert(std::is_nothrow_destructible<ObjectT>::value, "ObjectT shall be nothrow destructible.");

	auto pCur = pBegin + uCount;
	while(pCur != pBegin){
		--pCur;
		Destruct(pCur);
	}
}

}

#define FRIEND_CONSTRUCT_DESTRUCT(type_)		friend class ::MCF::Impl::DirectConstructor<type_>

#endif
