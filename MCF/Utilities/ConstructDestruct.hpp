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
	struct Tag {
		char chUnused;
	};
}

}

inline void *operator new(std::size_t, ::MCF::Impl_ConstructDestruct::Tag *p){
	return p;
}
inline void operator delete(void *, ::MCF::Impl_ConstructDestruct::Tag *) noexcept {
}

namespace MCF {

namespace Impl_ConstructDestruct {
	template<typename ObjectT>
	struct DirectConstructor {
		template<typename ...ParamsT>
		static void Construct(ObjectT *pObject, ParamsT &&...vParams){
			::new(reinterpret_cast<Tag *>(pObject)) ObjectT(std::forward<ParamsT>(vParams)...);
		}

		static void DefaultConstruct(ObjectT *pObject){
#ifndef NDEBUG
			__builtin_memset(pObject, 0xCC, sizeof(ObjectT));
#endif
			::new(reinterpret_cast<Tag *>(pObject)) ObjectT;
		}
		template<typename ...ParamsT>
		static void DefaultConstruct(ObjectT *pObject, ParamsT &&...vParams){
			::new(reinterpret_cast<Tag *>(pObject)) ObjectT(std::forward<ParamsT>(vParams)...);
		}

		static void Destruct(ObjectT *pObject){
			pObject->~ObjectT();
#ifndef NDEBUG
			__builtin_memset(pObject, 0xDD, sizeof(ObjectT));
#endif
		}
	};
}

template<typename ObjectT, typename ...ParamsT>
inline void Construct(ObjectT *pObject, ParamsT &&...vParams) noexcept(std::is_nothrow_constructible<std::remove_cv_t<ObjectT>, ParamsT &&...>::value) {
	Impl_ConstructDestruct::DirectConstructor<std::remove_cv_t<ObjectT>>::Construct(pObject, std::forward<ParamsT>(vParams)...);
}
template<typename ObjectT, typename ...ParamsT>
inline void DefaultConstruct(ObjectT *pObject, ParamsT &&...vParams) noexcept(std::is_nothrow_constructible<std::remove_cv_t<ObjectT>, ParamsT &&...>::value) {
	Impl_ConstructDestruct::DirectConstructor<std::remove_cv_t<ObjectT>>::DefaultConstruct(pObject, std::forward<ParamsT>(vParams)...);
}
template<typename ObjectT>
inline void Destruct(ObjectT *pObject) noexcept(std::is_nothrow_destructible<std::remove_cv_t<ObjectT>>::value) {
	Impl_ConstructDestruct::DirectConstructor<std::remove_cv_t<ObjectT>>::Destruct(pObject);
}

template<typename ObjectT, typename ...ParamsT>
void Construct(const ObjectT *pObject, ParamsT &&...vParams) noexcept = delete;
template<typename ObjectT, typename ...ParamsT>
void DefaultConstruct(const ObjectT *pObject, ParamsT &&...vParams) noexcept = delete;
template<typename ObjectT>
void Destruct(const ObjectT *pObject) noexcept = delete;

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

#define FRIEND_CONSTRUCT_DESTRUCT(type_)		friend class ::MCF::Impl_ConstructDestruct::DirectConstructor<type_>

#endif
