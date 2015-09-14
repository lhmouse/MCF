// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_CONTAINERS_FLAT_BST_HPP_
#define MCF_CONTAINERS_FLAT_BST_HPP_

#include "Vector.hpp"
#include <functional>

namespace MCF {

namespace Impl_FlatBst {
	template<typename ElementT, typename GetKeyFromElementT, typename ComparatorT>
	class FlatBst {
	private:
		Vector<ElementT> x_vecElements;

	public:
		constexpr FlatBst() noexcept
			: x_vecElements()
		{
		}
		template<typename IteratorT, std::enable_if_t<
			sizeof(typename std::iterator_traits<IteratorT>::value_type *),
			int> = 0>
		FlatBst(IteratorT itBegin, std::common_type_t<IteratorT> itEnd)
			: FlatBst()
		{
			Insert(nullptr, itBegin, itEnd);
		}
		FlatBst(std::initializer_list<ElementT> rhs)
			: FlatBst(rhs.begin(), rhs.end())
		{
		}
		FlatBst(const FlatBst &rhs)
			: x_vecElements(rhs.x_vecElements)
		{
		}
		FlatBst(FlatBst &&rhs) noexcept
			: x_vecElements(std::move(rhs.x_vecElements))
		{
		}
		FlatBst &operator=(const FlatBst &rhs){
			FlatBst(rhs).Swap(*this);
			return *this;
		}
		FlatBst &operator=(FlatBst &&rhs) noexcept {
			rhs.Swap(*this);
			return *this;
		}

	public:
		bool IsEmpty() const noexcept {
			return x_vecElements.IsEmpty();
		}
		void Clear() noexcept {
			x_vecElements.Clear();
		}

		const ElementT *GetFirst() const noexcept {
			return x_vecElements.GetFirst();
		}
		ElementT *GetFirst() noexcept {
			return x_vecElements.GetFirst();
		}
		const ElementT *GetConstFirst() const noexcept {
			return x_vecElements.GetConstFirst();
		}
		const ElementT *GetLast() const noexcept {
			return x_vecElements.GetLast();
		}
		ElementT *GetLast() noexcept {
			return x_vecElements.GetLast();
		}
		const ElementT *GetConstLast() const noexcept {
			return x_vecElements.GetConstLast();
		}

		const ElementT *GetPrev(const ElementT *pPos) const noexcept {
			return x_vecElements.GetPrev(pPos);
		}
		ElementT *GetPrev(ElementT *pPos) noexcept {
			return x_vecElements.GetPrev(pPos);
		}
		const ElementT *GetNext(const ElementT *pPos) const noexcept {
			return x_vecElements.GetNext(pPos);
		}
		ElementT *GetNext(ElementT *pPos) noexcept {
			return x_vecElements.GetNext(pPos);
		}

		void Swap(FlatBst &rhs) noexcept {
			x_vecElements.Swap(rhs.x_vecElements);
		}

		const ElementT *GetData() const noexcept {
			return x_vecElements.GetData();
		}
		ElementT *GetData() noexcept {
			return x_vecElements.GetData();
		}
		const ElementT *GetConstData() const noexcept {
			return x_vecElements.GetData();
		}
		std::size_t GetSize() const noexcept {
			return x_vecElements.GetSize();
		}
		std::size_t GetCapacity() noexcept {
			return x_vecElements.GetCapacity();
		}

		const ElementT *GetBegin() const noexcept {
			return x_vecElements.GetBegin();
		}
		ElementT *GetBegin() noexcept {
			return x_vecElements.GetBegin();
		}
		const ElementT *GetConstBegin() const noexcept {
			return x_vecElements.GetConstBegin();
		}
		const ElementT *GetEnd() const noexcept {
			return x_vecElements.GetEnd();
		}
		ElementT *GetEnd() noexcept {
			return x_vecElements.GetEnd();
		}
		const ElementT *GetConstEnd() const noexcept {
			return x_vecElements.GetConstEnd();
		}

		const ElementT &Get(std::size_t uIndex) const {
			return x_vecElements.Get(uIndex);
		}
		ElementT &Get(std::size_t uIndex){
			return x_vecElements.Get(uIndex);
		}
		const ElementT &UncheckedGet(std::size_t uIndex) const noexcept {
			return x_vecElements.UncheckedGet(uIndex);
		}
		ElementT &UncheckedGet(std::size_t uIndex) noexcept {
			return x_vecElements.UncheckedGet(uIndex);
		}

		void Reserve(std::size_t uNewCapacity){
			x_vecElements.Reserve(uNewCapacity);
		}
		void ReserveMore(std::size_t uDeltaCapacity){
			x_vecElements.ReserveMore(uDeltaCapacity);
		}

		template<typename ...ParamT>
		ElementT &Add(ParamT &&vParam){
			const auto pPos = GetUpperBound(vParam);
			const auto pInserted = x_vecElements.Insert(pPos, std::forward<ParamT>(vParam));
			return *pInserted;
		}
		template<typename ...ParamT>
		std::size_t Remove(const ParamT &vParam){
			const auto vRange = GetEqualRange(vParam);
			const auto uElementsRemoved = static_cast<std::size_t>(vRange.second - vRange.first);
			x_vecElements.Erase(vRange.first, vRange.second);
			return uElementsRemoved;
		}
		void Pop(std::size_t uCount = 1) noexcept {
			x_vecElements.Pop(uCount);
		}
/*
		template<typename ...ParamsT>
		ElementT *Emplace(const ElementT *pPos, ParamsT &&...vParams){
			if(!pPos || (pPos == GetEnd())){
				Push(std::forward<ParamsT>(vParams)...);
				return nullptr;
			}
			ASSERT((GetBegin() <= pPos) && (pPos <= GetEnd()));

			const auto uOffset = static_cast<std::size_t>(pPos - GetBegin());

			if(std::is_nothrow_move_constructible<ElementT>::value){
				X_PrepareForInsertion(uOffset, 1);
				const auto pBegin = GetBegin();
				try {
					DefaultConstruct(pBegin + uOffset, std::forward<ParamsT>(vParams)...);
				} catch(...){
					X_UndoPreparation(uOffset, 1);
					throw;
				}
				++x_uSize;
			} else {
				auto uNewCapacity = GetSize() + 1;
				if(uNewCapacity < GetSize()){
					throw std::bad_array_new_length();
				}
				if(uNewCapacity < GetCapacity()){
					uNewCapacity = GetCapacity();
				}
				Vector vecTemp;
				vecTemp.Reserve(uNewCapacity);
				for(auto pCur = GetBegin(); pCur != pPos; ++pCur){
					vecTemp.UncheckedPush(*pCur);
				}
				vecTemp.UncheckedPush(std::forward<ParamsT>(vParams)...);
				for(auto pCur = pPos; pCur != GetEnd(); ++pCur){
					vecTemp.UncheckedPush(*pCur);
				}
				*this = std::move(vecTemp);
			}

			return GetBegin() + uOffset;
		}

		template<typename ...ParamsT>
		ElementT *Insert(const ElementT *pPos, std::size_t uDeltaSize, const ParamsT &...vParams){
			if(!pPos || (pPos == GetEnd())){
				Append(uDeltaSize, vParams...);
				return nullptr;
			}
			ASSERT((GetBegin() <= pPos) && (pPos <= GetEnd()));

			const auto uOffset = static_cast<std::size_t>(pPos - GetBegin());

			if(std::is_nothrow_move_constructible<ElementT>::value){
				X_PrepareForInsertion(uOffset, uDeltaSize);
				const auto pBegin = GetBegin();
				std::size_t uWrite = 0;
				try {
					for(std::size_t i = 0; i < uDeltaSize; ++i){
						DefaultConstruct(pBegin + uOffset + uWrite, vParams...);
						++uWrite;
					}
				} catch(...){
					while(uWrite != 0){
						--uWrite;
						Destruct(pBegin + uWrite);
					}
					X_UndoPreparation(uWrite, uDeltaSize);
					throw;
				}
				x_uSize += uDeltaSize;
			} else {
				auto uNewCapacity = GetSize() + uDeltaSize;
				if(uNewCapacity < GetSize()){
					throw std::bad_array_new_length();
				}
				if(uNewCapacity < GetCapacity()){
					uNewCapacity = GetCapacity();
				}
				Vector vecTemp;
				vecTemp.Reserve(uNewCapacity);
				for(auto pCur = GetBegin(); pCur != pPos; ++pCur){
					vecTemp.UncheckedPush(*pCur);
				}
				for(std::size_t i = 0; i < uDeltaSize; ++i){
					vecTemp.UncheckedPush(vParams...);
				}
				for(auto pCur = pPos; pCur != GetEnd(); ++pCur){
					vecTemp.UncheckedPush(*pCur);
				}
				*this = std::move(vecTemp);
			}

			return GetBegin() + uOffset;
		}
		template<typename IteratorT, std::enable_if_t<
			sizeof(typename std::iterator_traits<IteratorT>::value_type *),
			int> = 0>
		ElementT *Insert(const ElementT *pPos, IteratorT itBegin, std::common_type_t<IteratorT> itEnd){
			if(!pPos || (pPos == GetEnd())){
				Append(itBegin, itEnd);
				return nullptr;
			}
			ASSERT((GetBegin() <= pPos) && (pPos <= GetEnd()));

			constexpr bool kHasDeltaSizeHint = std::is_base_of<std::forward_iterator_tag, typename std::iterator_traits<IteratorT>::iterator_category>::value;

			const auto uOffset = static_cast<std::size_t>(pPos - GetBegin());

			if(kHasDeltaSizeHint && std::is_nothrow_move_constructible<ElementT>::value){
				const auto uDeltaSize = static_cast<std::size_t>(std::distance(itBegin, itEnd));
				X_PrepareForInsertion(uOffset, uDeltaSize);
				const auto pBegin = GetBegin();
				std::size_t uWrite = 0;
				try {
					for(auto it = itBegin; it != itEnd; ++it){
						Construct(pBegin + uOffset + uWrite, *it);
						++uWrite;
					}
				} catch(...){
					while(uWrite != 0){
						--uWrite;
						Destruct(pBegin + uWrite);
					}
					X_UndoPreparation(uWrite, uDeltaSize);
					throw;
				}
				x_uSize += uDeltaSize;
			} else {
				if(kHasDeltaSizeHint){
					const auto uDeltaSize = static_cast<std::size_t>(std::distance(itBegin, itEnd));
					auto uNewCapacity = GetSize() + uDeltaSize;
					if(uNewCapacity < GetSize()){
						throw std::bad_array_new_length();
					}
					if(uNewCapacity < GetCapacity()){
						uNewCapacity = GetCapacity();
					}
					Vector vecTemp;
					vecTemp.Reserve(uNewCapacity);
					for(auto pCur = GetBegin(); pCur != pPos; ++pCur){
						vecTemp.UncheckedPush(*pCur);
					}
					for(auto it = itBegin; it != itEnd; ++it){
						vecTemp.UncheckedPush(*it);
					}
					for(auto pCur = pPos; pCur != GetEnd(); ++pCur){
						vecTemp.UncheckedPush(*pCur);
					}
					*this = std::move(vecTemp);
				} else {
					Vector vecTemp;
					vecTemp.Reserve(GetCapacity());
					for(auto pCur = GetBegin(); pCur != pPos; ++pCur){
						vecTemp.UncheckedPush(*pCur);
					}
					for(auto it = itBegin; it != itEnd; ++it){
						vecTemp.Push(*it);
					}
					for(auto pCur = pPos; pCur != GetEnd(); ++pCur){
						vecTemp.Push(*pCur);
					}
					*this = std::move(vecTemp);
				}
			}

			return GetBegin() + uOffset;
		}
		ElementT *Insert(const ElementT *pPos, std::initializer_list<ElementT> ilElements){
			return Insert(pPos, ilElements.begin(), ilElements.end());
		}
*/
		ElementT *Erase(const ElementT *pBegin, const ElementT *pEnd) noexcept(noexcept(std::declval<Vector<ElementT> *>().Erase(pBegin, pEnd))) {
			return x_vecElements.Erase(pBegin, pEnd);
		}
		ElementT *Erase(const ElementT *pPos) noexcept(noexcept(std::declval<Vector<ElementT> *>().Erase(pPos))) {
			return x_vecElements.Erase(pPos);
		}

		const ElementT &operator[](std::size_t uIndex) const noexcept {
			return x_vecElements[uIndex];
		}
		ElementT &operator[](std::size_t uIndex) noexcept {
			return x_vecElements[uIndex];
		}
	};
}

}

#endif
