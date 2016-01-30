// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_CONTAINERS_FLAT_CONTAINER_HPP_
#define MCF_CONTAINERS_FLAT_CONTAINER_HPP_

#include "../Utilities/Assert.hpp"
#include "../Utilities/ConstructDestruct.hpp"
#include "../Utilities/DeclVal.hpp"
#include "../Core/Exception.hpp"
#include <utility>
#include <new>
#include <type_traits>
#include <cstddef>

namespace MCF {

namespace Impl_FlatContainer {
	template<typename ElementT, class MoveCasterT>
	class FlatContainer {
	public:
		using Element    = ElementT;
		using MoveCaster = MoveCasterT;

		static_assert(noexcept(MoveCaster()(DeclVal<Element &>())), "Bad MoveCaster.");

	private:
		void *x_pStorage;
		std::size_t x_uSize;
		std::size_t x_uCapacity;

	public:
		constexpr FlatContainer() noexcept
			: x_pStorage(nullptr), x_uSize(0), x_uCapacity(0)
		{
		}
		FlatContainer(const FlatContainer &rhs)
			: FlatContainer()
		{
			Reserve(rhs.GetSize());

			for(auto p = rhs.GetBegin(); p != rhs.GetEnd(); ++p){
				X_UncheckedPush(*p);
			}
		}
		FlatContainer(FlatContainer &&rhs) noexcept
			: FlatContainer()
		{
			rhs.Swap(*this);
		}
		FlatContainer &operator=(const FlatContainer &rhs){
			FlatContainer(rhs).Swap(*this);
			return *this;
		}
		FlatContainer &operator=(FlatContainer &&rhs) noexcept {
			rhs.Swap(*this);
			return *this;
		}
		~FlatContainer(){
			Clear();
			::operator delete[](x_pStorage);
		}

	private:
		template<typename ...ParamsT>
		Element &X_Push(ParamsT &&...vParams){
			ReserveMore(1);
			return X_UncheckedPush(std::forward<ParamsT>(vParams)...);
		}
		template<typename ...ParamsT>
		Element &X_UncheckedPush(ParamsT &&...vParams) noexcept(std::is_nothrow_constructible<Element, ParamsT &&...>::value) {
			ASSERT(x_uCapacity - x_uSize > 0);

			const auto pBegin = GetBegin();
			const auto pElem = pBegin + x_uSize;
			DefaultConstruct(pElem, std::forward<ParamsT>(vParams)...);
			++x_uSize;

			return *pElem;
		}
		void X_Pop(std::size_t uCount = 1) noexcept {
			ASSERT(uCount <= x_uSize);

			const auto pBegin = GetBegin();
			for(std::size_t i = 0; i < uCount; ++i){
				Destruct(pBegin + x_uSize - i - 1);
			}
			x_uSize -= uCount;
		}

	public:
		bool IsEmpty() const noexcept {
			return x_uSize == 0;
		}
		void Clear() noexcept {
			X_Pop(x_uSize);
		}
		template<typename OutputIteratorT>
		OutputIteratorT Extract(OutputIteratorT itOutput){
			try {
				for(auto p = GetBegin(); p != GetEnd(); ++p){
					*itOutput = MoveCaster()(*p);
					++itOutput;
				}
			} catch(...){
				Clear();
				throw;
			}
			Clear();
			return itOutput;
		}

		void Swap(FlatContainer &rhs) noexcept {
			using std::swap;
			swap(x_pStorage,  rhs.x_pStorage);
			swap(x_uSize,     rhs.x_uSize);
			swap(x_uCapacity, rhs.x_uCapacity);
		}

		const Element *GetData() const noexcept {
			return static_cast<const Element *>(x_pStorage);
		}
		Element *GetData() noexcept {
			return static_cast<Element *>(x_pStorage);
		}
		std::size_t GetSize() const noexcept {
			return x_uSize;
		}
		std::size_t GetCapacity() const noexcept {
			return x_uCapacity;
		}

		const Element *GetBegin() const noexcept {
			return GetData();
		}
		Element *GetBegin() noexcept {
			return GetData();
		}
		const Element *GetEnd() const noexcept {
			return GetData() + x_uSize;
		}
		Element *GetEnd() noexcept {
			return GetData() + x_uSize;
		}

		void Reserve(std::size_t uNewCapacity){
			const auto uOldCapacity = x_uCapacity;
			if(uNewCapacity <= uOldCapacity){
				return;
			}

			auto uElementsToAlloc = uOldCapacity + 1;
			uElementsToAlloc += (uElementsToAlloc >> 1);
			uElementsToAlloc = (uElementsToAlloc + 0x0F) & (std::size_t)-0x10;
			if(uElementsToAlloc < uNewCapacity){
				uElementsToAlloc = uNewCapacity;
			}
			const std::size_t uBytesToAlloc = sizeof(Element) * uElementsToAlloc;
			if(uBytesToAlloc / sizeof(Element) != uElementsToAlloc){
				throw std::bad_array_new_length();
			}

			const auto pNewStorage = ::operator new[](uBytesToAlloc);
			const auto pOldStorage = x_pStorage;
			const auto pNewBegin = static_cast<Element *>(pNewStorage);
			const auto pOldBegin = static_cast<Element *>(pOldStorage);
			auto pWrite = pNewBegin;
			try {
				for(std::size_t i = 0; i < x_uSize; ++i){
					if(MoveCaster::kEnabled){
						Construct(pWrite, MoveCaster()(pOldBegin[i]));
					} else if(!std::is_copy_constructible<Element>::value){
						Construct(pWrite, std::move(pOldBegin[i]));
					} else {
						Construct(pWrite, pOldBegin[i]);
					}
					++pWrite;
				}
			} catch(...){
				while(pWrite != pNewBegin){
					--pWrite;
					Destruct(pWrite);
				}
				::operator delete[](pNewStorage);
				throw;
			}
			for(std::size_t i = x_uSize; i > 0; --i){
				Destruct(pOldBegin + i - 1);
			}
			::operator delete[](pOldStorage);

			x_pStorage  = pNewStorage;
			x_uCapacity = uElementsToAlloc;
		}
		void ReserveMore(std::size_t uDeltaCapacity){
			const auto uOldSize = x_uSize;
			const auto uNewCapacity = uOldSize + uDeltaCapacity;
			if(uNewCapacity < uOldSize){
				throw std::bad_array_new_length();
			}
			Reserve(uNewCapacity);
		}

		template<typename ...ParamsT>
		Element *Emplace(const Element *pPos, ParamsT &&...vParams){
			if(!pPos || (pPos == GetEnd())){
				X_Push(std::forward<ParamsT>(vParams)...);
				return nullptr;
			}
			ASSERT((GetBegin() <= pPos) && (pPos <= GetEnd()));

			const auto uOffset = static_cast<std::size_t>(pPos - GetBegin());

			if(MoveCaster::kEnabled){
				ReserveMore(1);
				const auto pBegin = GetBegin();
				for(std::size_t i = x_uSize; i > uOffset; --i){
					const auto pRead = pBegin + i - 1;
					Construct(pRead + 1, MoveCaster()(pRead[0]));
					Destruct(pRead);
				}
				try {
					Construct(pBegin + uOffset, std::forward<ParamsT>(vParams)...);
				} catch(...){
					const auto pBegin = GetBegin();
					for(std::size_t i = uOffset; i < x_uSize; ++i){
						const auto pWrite = pBegin + i;
						Construct(pWrite, MoveCaster()(pWrite[1]));
						Destruct(pWrite + 1);
					}
					throw;
				}
				++x_uSize;
			} else {
				auto uNewCapacity = x_uSize + 1;
				if(uNewCapacity < x_uSize){
					throw std::bad_array_new_length();
				}
				if(uNewCapacity < x_uCapacity){
					uNewCapacity = x_uCapacity;
				}
				FlatContainer vTemp;
				vTemp.Reserve(uNewCapacity);
				for(auto pCur = GetBegin(); pCur != pPos; ++pCur){
					vTemp.X_UncheckedPush(*pCur);
				}
				vTemp.X_UncheckedPush(std::forward<ParamsT>(vParams)...);
				for(auto pCur = pPos; pCur != GetEnd(); ++pCur){
					vTemp.X_UncheckedPush(*pCur);
				}
				*this = std::move(vTemp);
			}

			return GetBegin() + uOffset;
		}
		Element *Erase(const Element *pBegin, const Element *pEnd) noexcept(MoveCaster::kEnabled) {
			if(pBegin == pEnd){
				if(pEnd == GetEnd()){
					return nullptr;
				}
				return const_cast<Element *>(pEnd);
			}
			ASSERT(pBegin);

			if(!pEnd || (pEnd == GetEnd())){
				const auto uDeltaSize = static_cast<std::size_t>(GetEnd() - pBegin);

				X_Pop(uDeltaSize);
				return nullptr;
			}
			ASSERT((GetBegin() <= pBegin) && (pBegin <= pEnd) && (pEnd <= GetEnd()));

			const auto uOffset = static_cast<std::size_t>(pBegin - GetBegin());

			if(MoveCaster::kEnabled){
				const auto uDeltaSize = static_cast<std::size_t>(pEnd - pBegin);

				auto pWrite = const_cast<Element *>(pBegin);
				for(auto pCur = pWrite; pCur != pEnd; ++pCur){
					Destruct(pCur);
				}
				for(auto pCur = const_cast<Element *>(pEnd); pCur != GetEnd(); ++pCur){
					Construct(pWrite, MoveCaster()(*pCur));
					Destruct(pCur);
					++pWrite;
				}
				x_uSize -= uDeltaSize;
			} else {
				FlatContainer vTemp;
				vTemp.Reserve(x_uCapacity);
				for(auto pCur = GetBegin(); pCur != pBegin; ++pCur){
					vTemp.X_UncheckedPush(*pCur);
				}
				for(auto pCur = pEnd; pCur != GetEnd(); ++pCur){
					vTemp.X_UncheckedPush(*pCur);
				}
				*this = std::move(vTemp);
			}

			return GetBegin() + uOffset;
		}
		Element *Erase(const Element *pPos) noexcept(noexcept(DeclVal<FlatContainer &>().Erase(pPos, pPos))) {
			ASSERT(pPos);

			return Erase(pPos, pPos + 1);
		}

		friend void swap(FlatContainer &lhs, FlatContainer &rhs) noexcept {
			lhs.Swap(rhs);
		}
	};
}

}

#endif
