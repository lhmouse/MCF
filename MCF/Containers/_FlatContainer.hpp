// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_CONTAINERS_FLAT_CONTAINER_HPP_
#define MCF_CONTAINERS_FLAT_CONTAINER_HPP_

#include "../Utilities/Assert.hpp"
#include "../Utilities/ConstructDestruct.hpp"
#include "../Utilities/DeclVal.hpp"
#include "../Core/Exception.hpp"
#include "../Core/_CheckedSizeArithmetic.hpp"
#include <utility>
#include <type_traits>
#include <cstddef>

namespace MCF {

namespace Impl_FlatContainer {
	template<typename ElementT, class MoveCasterT, class AllocatorT>
	class FlatContainer {
	public:
		// 容器需求。
		using Element         = ElementT;
		using MoveCaster      = MoveCasterT;
		using Allocator       = AllocatorT;

	private:
		Element *x_pStorage;
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
			Reserve(rhs.x_uSize);
			for(std::size_t i = 0; i < rhs.x_uSize; ++i){
				X_UncheckedPush(rhs.x_pStorage[i]);
			}
		}
		FlatContainer(FlatContainer &&rhs) noexcept
			: FlatContainer()
		{
			rhs.Swap(*this);
		}
		FlatContainer &operator=(const FlatContainer &rhs){
			if(std::is_nothrow_copy_constructible<Element>::value || IsEmpty()){
				Reserve(rhs.x_uSize);
				try {
					for(std::size_t i = 0; i < rhs.x_uSize; ++i){
						X_UncheckedPush(rhs.x_pStorage[i]);
					}
				} catch(...){
					Clear();
					throw;
				}
			} else {
				FlatContainer(rhs).Swap(*this);
			}
			return *this;
		}
		FlatContainer &operator=(FlatContainer &&rhs) noexcept {
			rhs.Swap(*this);
			return *this;
		}
		~FlatContainer(){
			Clear();
			Allocator()(x_pStorage);
		}

	private:
		template<typename ...ParamsT>
		Element &X_UncheckedPush(ParamsT &&...vParams){
			MCF_ASSERT(x_uCapacity - x_uSize > 0);

			const auto pData = x_pStorage;
			const auto pElement = pData + x_uSize;
			DefaultConstruct(pElement, std::forward<ParamsT>(vParams)...);
			++x_uSize;

			return *pElement;
		}
		void X_Pop(std::size_t uCount = 1) noexcept {
			MCF_ASSERT(uCount <= x_uSize);

			const auto pData = x_pStorage;
			for(std::size_t i = 0; i < uCount; ++i){
				Destruct(pData + x_uSize - 1 - i);
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
				for(std::size_t i = 0; i < x_uSize; ++i){
					*itOutput = MoveCaster()(x_pStorage[i]);
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
			return x_pStorage;
		}
		Element *GetData() noexcept {
			return x_pStorage;
		}
		std::size_t GetSize() const noexcept {
			return x_uSize;
		}
		std::size_t GetCapacity() const noexcept {
			return x_uCapacity;
		}

		const Element *GetBegin() const noexcept {
			return x_pStorage;
		}
		Element *GetBegin() noexcept {
			return x_pStorage;
		}
		const Element *GetEnd() const noexcept {
			return x_pStorage + x_uSize;
		}
		Element *GetEnd() noexcept {
			return x_pStorage + x_uSize;
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
			const auto uBytesToAlloc = Impl_CheckedSizeArithmetic::Mul(sizeof(Element), uElementsToAlloc);
			const auto pNewStorage = static_cast<Element *>(Allocator()(uBytesToAlloc));
			const auto pOldStorage = x_pStorage;
			auto pWrite = pNewStorage;
			try {
				for(std::size_t i = 0; i < x_uSize; ++i){
					if(MoveCaster::kEnabled){
						Construct(pWrite, MoveCaster()(pOldStorage[i]));
					} else {
						Construct(pWrite, std::move_if_noexcept(pOldStorage[i]));
					}
					++pWrite;
				}
			} catch(...){
				while(pWrite != pNewStorage){
					--pWrite;
					Destruct(pWrite);
				}
				Allocator()(pNewStorage);
				throw;
			}
			for(std::size_t i = x_uSize; i > 0; --i){
				Destruct(pOldStorage + i - 1);
			}
			Allocator()(pOldStorage);

			x_pStorage  = pNewStorage;
			x_uCapacity = uElementsToAlloc;
		}
		void ReserveMore(std::size_t uDeltaCapacity){
			const auto uNewCapacity = Impl_CheckedSizeArithmetic::Add(uDeltaCapacity, x_uSize);
			Reserve(uNewCapacity);
		}

		template<typename ...ParamsT>
		Element *Emplace(const Element *pPos, ParamsT &&...vParams){
			std::size_t uOffset;
			if(pPos){
				uOffset = static_cast<std::size_t>(pPos - x_pStorage);
			} else {
				uOffset = x_uSize;
			}

			if(MoveCaster::kEnabled){
				ReserveMore(1);
				for(std::size_t i = x_uSize; i > uOffset; --i){
					Construct(x_pStorage + i, MoveCaster()(x_pStorage[i - 1]));
					Destruct(x_pStorage + i - 1);
				}
				auto uWrite = uOffset;
				try {
					DefaultConstruct(x_pStorage + uWrite, std::forward<ParamsT>(vParams)...);
				} catch(...){
					for(std::size_t i = uOffset; i < x_uSize; ++i){
						Construct(x_pStorage + i, MoveCaster()(x_pStorage[i + 1]));
						Destruct(x_pStorage + i + 1);
					}
					throw;
				}
				x_uSize += 1;
			} else {
				auto uNewCapacity = Impl_CheckedSizeArithmetic::Add(1, x_uSize);
				const auto uCapacity = x_uCapacity;
				if(uNewCapacity < uCapacity){
					uNewCapacity = uCapacity;
				}
				FlatContainer vTemp;
				vTemp.Reserve(uNewCapacity);
				for(std::size_t i = 0; i < uOffset; ++i){
					vTemp.X_UncheckedPush(x_pStorage[i]);
				}
				vTemp.X_UncheckedPush(std::forward<ParamsT>(vParams)...);
				for(std::size_t i = uOffset; i < x_uSize; ++i){
					vTemp.X_UncheckedPush(x_pStorage[i]);
				}
				*this = std::move(vTemp);
			}

			return x_pStorage + uOffset;
		}

		Element *Erase(const Element *pBegin, const Element *pEnd) noexcept(MoveCaster::kEnabled) {
			std::size_t uOffsetBegin, uOffsetEnd;
			if(pBegin){
				uOffsetBegin = static_cast<std::size_t>(pBegin - x_pStorage);
			} else {
				uOffsetBegin = x_uSize;
			}
			if(pEnd){
				uOffsetEnd = static_cast<std::size_t>(pEnd - x_pStorage);
			} else {
				uOffsetEnd = x_uSize;
			}

			if(uOffsetBegin != uOffsetEnd){
				if(MoveCaster::kEnabled){
					const auto uDeltaSize = uOffsetEnd - uOffsetBegin;
					for(std::size_t i = uOffsetBegin; i < uOffsetEnd; ++i){
						Destruct(x_pStorage + i);
					}
					for(std::size_t i = uOffsetEnd; i < x_uSize; ++i){
						Construct(x_pStorage + i - uDeltaSize, MoveCaster()(x_pStorage[i]));
						Destruct(x_pStorage + i);
					}
					x_uSize -= uDeltaSize;
				} else {
					FlatContainer vTemp;
					const auto uCapacity = x_uCapacity;
					vTemp.Reserve(uCapacity);
					for(std::size_t i = 0; i < uOffsetBegin; ++i){
						vTemp.X_UncheckedPush(x_pStorage[i]);
					}
					for(std::size_t i = uOffsetEnd; i < x_uSize; ++i){
						vTemp.X_UncheckedPush(x_pStorage[i]);
					}
					*this = std::move(vTemp);
				}
			}

			return x_pStorage + uOffsetBegin;
		}
		Element *Erase(const Element *pPos) noexcept(noexcept(DeclVal<FlatContainer &>().Erase(pPos, pPos))) {
			MCF_ASSERT(pPos);

			return Erase(pPos, pPos + 1);
		}

	public:
		friend void swap(FlatContainer &lhs, FlatContainer &rhs) noexcept {
			lhs.Swap(rhs);
		}
	};
}

}

#endif
