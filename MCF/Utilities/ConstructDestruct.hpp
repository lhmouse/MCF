// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_UTILITIES_CONSTRUCT_DESTRUCT_HPP_
#define MCF_UTILITIES_CONSTRUCT_DESTRUCT_HPP_

#include <type_traits>
#include <utility>
#include <new>

namespace MCF {

namespace Impl_ConstructDestruct {
	template<typename ObjectT>
	struct DirectConstructor {
		template<typename ...ParamsT>
		static void Construct(ObjectT *pObject, ParamsT &&...vParams){
			::new(static_cast<void *>(pObject)) ObjectT(std::forward<ParamsT>(vParams)...);
		}

		static void DefaultConstruct(ObjectT *pObject){
#ifndef NDEBUG
			__builtin_memset(pObject, 0xCC, sizeof(ObjectT));
#endif
			::new(static_cast<void *>(pObject)) ObjectT;
		}
		template<typename ...ParamsT>
		static void DefaultConstruct(ObjectT *pObject, ParamsT &&...vParams){
			::new(static_cast<void *>(pObject)) ObjectT(std::forward<ParamsT>(vParams)...);
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
	Impl_ConstructDestruct::DirectConstructor<ObjectT>::Construct(pObject, std::forward<ParamsT>(vParams)...);
}
template<typename ObjectT, typename ...ParamsT>
inline void DefaultConstruct(ObjectT *pObject, ParamsT &&...vParams)
	noexcept(std::is_nothrow_constructible<ObjectT, ParamsT &&...>::value)
{
	Impl_ConstructDestruct::DirectConstructor<ObjectT>::DefaultConstruct(pObject, std::forward<ParamsT>(vParams)...);
}
template<typename ObjectT>
inline void Destruct(ObjectT *pObject)
	noexcept(std::is_nothrow_destructible<ObjectT>::value)
{
	Impl_ConstructDestruct::DirectConstructor<ObjectT>::Destruct(pObject);
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

#define FRIEND_CONSTRUCT_DESTRUCT(type_)		friend class ::MCF::Impl_ConstructDestruct::DirectConstructor<type_>

#endif
