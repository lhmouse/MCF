// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_CONSTRUCT_DESTRUCT_HPP_
#define MCF_CORE_CONSTRUCT_DESTRUCT_HPP_

#include <type_traits>
#include <utility>
#include <new>

namespace MCF {

namespace Impl_ConstructDestruct {
	struct Tag { };
}

}

inline void *operator new(std::size_t, void *p, const ::MCF::Impl_ConstructDestruct::Tag &){
	return p;
}
inline void operator delete(void *, void *, const ::MCF::Impl_ConstructDestruct::Tag &) noexcept {
}

namespace MCF {

namespace Impl_ConstructDestruct {
	template<typename ObjectT>
	struct DirectConstructor {
		template<typename ...ParamsT>
		static void Construct(ObjectT *pObject, ParamsT &&...vParams){
			const auto pStorage = const_cast<void *>(static_cast<const volatile void *>(pObject));
			(void)pStorage;

#ifndef NDEBUG
			__builtin_memset(pStorage, 0xBB, sizeof(ObjectT));
#endif
			::new(pStorage, Tag()) ObjectT(std::forward<ParamsT>(vParams)...);
		}

		static void DefaultConstruct(ObjectT *pObject){
			const auto pStorage = const_cast<void *>(static_cast<const volatile void *>(pObject));
			(void)pStorage;

#ifndef NDEBUG
			__builtin_memset(pStorage, 0xCC, sizeof(ObjectT));
#endif
			::new(pStorage, Tag()) ObjectT;
		}
		template<typename ...ParamsT>
		static void DefaultConstruct(ObjectT *pObject, ParamsT &&...vParams){
			const auto pStorage = const_cast<void *>(static_cast<const volatile void *>(pObject));
			(void)pStorage;

#ifndef NDEBUG
			__builtin_memset(pStorage, 0xDD, sizeof(ObjectT));
#endif
			::new(pStorage, Tag()) ObjectT(std::forward<ParamsT>(vParams)...);
		}

		static void Destruct(ObjectT *pObject){
			const auto pStorage = const_cast<void *>(static_cast<const volatile void *>(pObject));
			(void)pStorage;

			pObject->~ObjectT();
#ifndef NDEBUG
			__builtin_memset(pStorage, 0xEE, sizeof(ObjectT));
#endif
		}
	};
}

template<typename ObjectT, typename ...ParamsT>
inline void Construct(ObjectT *pObject, ParamsT &&...vParams) noexcept(std::is_nothrow_constructible<ObjectT, ParamsT &&...>::value) {
	Impl_ConstructDestruct::DirectConstructor<ObjectT>::Construct(pObject, std::forward<ParamsT>(vParams)...);
}
template<typename ObjectT, typename ...ParamsT>
inline void DefaultConstruct(ObjectT *pObject, ParamsT &&...vParams) noexcept(std::is_nothrow_constructible<ObjectT, ParamsT &&...>::value) {
	Impl_ConstructDestruct::DirectConstructor<ObjectT>::DefaultConstruct(pObject, std::forward<ParamsT>(vParams)...);
}
template<typename ObjectT>
inline void Destruct(ObjectT *pObject) noexcept(std::is_nothrow_destructible<ObjectT>::value) {
	Impl_ConstructDestruct::DirectConstructor<ObjectT>::Destruct(pObject);
}

template<typename ObjectT, typename ...ParamsT>
inline void ConstructArray(ObjectT *pBegin, std::size_t uCount, const ParamsT &...vParams) noexcept(std::is_nothrow_constructible<ObjectT, const ParamsT &...>::value) {
	static_assert(std::is_nothrow_destructible<ObjectT>::value, "ObjectT shall be nothrow destructible.");

	auto pCur = pBegin;
	try {
		for(std::size_t i = 0; i < uCount; ++i){
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
inline void DefaultConstructArray(ObjectT *pBegin, std::size_t uCount, const ParamsT &...vParams) noexcept(std::is_nothrow_constructible<ObjectT, const ParamsT &...>::value) {
	static_assert(std::is_nothrow_destructible<ObjectT>::value, "ObjectT shall be nothrow destructible.");

	auto pCur = pBegin;
	try {
		for(std::size_t i = 0; i < uCount; ++i){
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

#endif
