// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_STRING_HPP_
#define MCF_CORE_STRING_HPP_

#include "StringView.hpp"
#include "../Containers/_EnumeratorTemplate.hpp"
#include "../Utilities/Assert.hpp"
#include "../Utilities/CountOf.hpp"
#include "../Utilities/CountLeadingTrailingZeroes.hpp"
#include "../Utilities/CopyMoveFill.hpp"
#include <type_traits>
#include <new>
#include <cstring>
#include <cstddef>
#include <cstdint>

// 借用了 https://github.com/elliotgoodrich/SSO-23 的一些想法。然而我们可以优化更多……
// 我们支持的最大 SSO 长度是 31 个“字符”，而不是 23 个“字节”。

namespace MCF {

template<StringType kTypeT>
class String;

template<StringType kTypeT>
class String {
public:
	using View = StringView<kTypeT>;

	using Char = typename View::Char;

	enum : std::size_t {
		kNpos = View::kNpos
	};

public:
	static const String kEmpty;

public:
	static StringView<StringType::kUtf16> UnifyAssign(String<StringType::kUtf16> &u16sTemp, const View &vSrc);
	static void DeunifyAppend(String &strDst, const StringView<StringType::kUtf16> &u16svSrc);

	static StringView<StringType::kUtf32> UnifyAssign(String<StringType::kUtf32> &u32sTemp, const View &vSrc);
	static void DeunifyAppend(String &strDst, const StringView<StringType::kUtf32> &u32svSrc);

private:
	union X_Storage {
		struct {
			Char achData[31];
			std::make_signed_t<Char> schComplLength;
		} vSmall;

		struct {
			Char *pchBegin;
			std::size_t uLength;
			std::size_t uSizeAllocated;
		} vLarge;
	} x_vStorage;

private:
	std::size_t X_GetSmallLength() const noexcept {
		return CountOf(x_vStorage.vSmall.achData) - static_cast<std::make_unsigned_t<Char>>(x_vStorage.vSmall.schComplLength);
	}
	void X_SetSmallLength(std::size_t uLength) noexcept {
		x_vStorage.vSmall.schComplLength = static_cast<std::make_signed_t<Char>>(CountOf(x_vStorage.vSmall.achData) - uLength);
	}

public:
	String() noexcept {
#ifndef NDEBUG
		std::memset(x_vStorage.vSmall.achData, 0xCC, sizeof(x_vStorage.vSmall.achData));
#endif
		X_SetSmallLength(0);
	}
	explicit String(Char ch, std::size_t uCount = 1)
		: String()
	{
		Append(ch, uCount);
	}
	explicit String(const Char *pszBegin)
		: String()
	{
		Append(pszBegin);
	}
	String(const Char *pchBegin, const Char *pchEnd)
		: String()
	{
		Append(pchBegin, pchEnd);
	}
	String(const Char *pchBegin, std::size_t uLen)
		: String()
	{
		Append(pchBegin, uLen);
	}
	explicit String(const View &rhs)
		: String()
	{
		Append(rhs);
	}
	String(std::initializer_list<Char> rhs)
		: String()
	{
		Append(rhs);
	}
	template<StringType kOtherTypeT>
	explicit String(const StringView<kOtherTypeT> &rhs)
		: String()
	{
		Append(rhs);
	}
	template<StringType kOtherTypeT>
	explicit String(const String<kOtherTypeT> &rhs)
		: String()
	{
		Append(rhs);
	}
	String(const String &rhs)
		: String()
	{
		Append(rhs);
	}
	String(String &&rhs) noexcept
		: String()
	{
		Swap(rhs);
	}
	String &operator=(Char ch){
		Assign(ch, 1);
		return *this;
	}
	String &operator=(const Char *pszBegin){
		Assign(pszBegin);
		return *this;
	}
	String &operator=(const View &rhs){
		Assign(rhs);
		return *this;
	}
	template<StringType kOtherTypeT>
	String &operator=(const StringView<kOtherTypeT> &rhs){
		Assign(rhs);
		return *this;
	}
	template<StringType kOtherTypeT>
	String &operator=(const String<kOtherTypeT> &rhs){
		Assign(rhs);
		return *this;
	}
	String &operator=(const String &rhs){
		Assign(rhs);
		return *this;
	}
	String &operator=(String &&rhs) noexcept {
		Assign(std::move(rhs));
		return *this;
	}
	~String() noexcept {
		if(x_vStorage.vSmall.schComplLength < 0){
			::operator delete[](x_vStorage.vLarge.pchBegin);
		}
#ifndef NDEBUG
		std::memset(&x_vStorage, 0xDD, sizeof(x_vStorage));
#endif
	}

private:
	Char *X_ChopAndSplice(std::size_t uRemovedBegin, std::size_t uRemovedEnd, std::size_t uFirstOffset, std::size_t uThirdOffset){
		const auto pchOldBuffer = GetBegin();
		const auto uOldLength = GetLength();
		auto pchNewBuffer = pchOldBuffer;
		const auto uNewLength = uThirdOffset + (uOldLength - uRemovedEnd);
		auto uCharsToAlloc = uNewLength + 1;

		ASSERT(uRemovedBegin <= uOldLength);
		ASSERT(uRemovedEnd <= uOldLength);
		ASSERT(uRemovedBegin <= uRemovedEnd);
		ASSERT(uFirstOffset + uRemovedBegin <= uThirdOffset);

		if(GetCapacity() < uNewLength){
			uCharsToAlloc += (uCharsToAlloc >> 1);
			uCharsToAlloc = (uCharsToAlloc + 0x0F) & (std::size_t)-0x10;
			if(uCharsToAlloc < uNewLength + 1){
				uCharsToAlloc = uNewLength + 1;
			}
			const std::size_t uBytesToAlloc = sizeof(Char) * uCharsToAlloc;
			if(uBytesToAlloc / sizeof(Char) != uCharsToAlloc){
				throw std::bad_array_new_length();
			}

			pchNewBuffer = (Char *)::operator new[](uBytesToAlloc);
		}

		if((pchNewBuffer + uFirstOffset != pchOldBuffer) && (uRemovedBegin != 0)){
			std::memmove(pchNewBuffer + uFirstOffset, pchOldBuffer, uRemovedBegin * sizeof(Char));
		}
		if((pchNewBuffer + uThirdOffset != pchOldBuffer + uRemovedEnd) && (uOldLength != uRemovedEnd)){
			std::memmove(pchNewBuffer + uThirdOffset, pchOldBuffer + uRemovedEnd, (uOldLength - uRemovedEnd) * sizeof(Char));
		}

		if(pchNewBuffer != pchOldBuffer){
			if(x_vStorage.vSmall.schComplLength >= 0){
				x_vStorage.vSmall.schComplLength = -1;
			} else {
				::operator delete[](pchOldBuffer);
			}

			x_vStorage.vLarge.pchBegin = pchNewBuffer;
			x_vStorage.vLarge.uLength = uOldLength;
			x_vStorage.vLarge.uSizeAllocated = uCharsToAlloc;
		}

		return pchNewBuffer + uFirstOffset + uRemovedBegin;
	}
	void X_SetSize(std::size_t uNewSize) noexcept {
		ASSERT(uNewSize <= GetCapacity());

		if(x_vStorage.vSmall.schComplLength >= 0){
			X_SetSmallLength(uNewSize);
		} else {
			x_vStorage.vLarge.uLength = uNewSize;
		}
	}

public:
	// 容器需求。
	using Element         = Char;
	using ConstEnumerator = Impl_EnumeratorTemplate::ConstEnumerator <String>;
	using Enumerator      = Impl_EnumeratorTemplate::Enumerator      <String>;

	bool IsEmpty() const noexcept {
		return GetBegin() == GetEnd();
	}
	void Clear() noexcept {
		X_SetSize(0);
	}

	const Element *GetFirst() const noexcept {
		if(IsEmpty()){
			return nullptr;
		}
		return GetBegin();
	}
	Element *GetFirst() noexcept {
		if(IsEmpty()){
			return nullptr;
		}
		return GetBegin();
	}
	const Element *GetConstFirst() const noexcept {
		return GetFirst();
	}
	const Element *GetLast() const noexcept {
		if(IsEmpty()){
			return nullptr;
		}
		return GetEnd() - 1;
	}
	Element *GetLast() noexcept {
		if(IsEmpty()){
			return nullptr;
		}
		return GetEnd() - 1;
	}
	const Element *GetConstLast() const noexcept {
		return GetLast();
	}

	const Element *GetPrev(const Element *pPos) const noexcept {
		const auto pBegin = GetBegin();
		const auto uOffset = static_cast<std::size_t>(pPos - pBegin);
		if(uOffset == 0){
			return nullptr;
		}
		return pBegin + uOffset - 1;
	}
	Element *GetPrev(const Element *pPos) noexcept {
		const auto pBegin = GetBegin();
		const auto uOffset = static_cast<std::size_t>(pPos - pBegin);
		if(uOffset == 0){
			return nullptr;
		}
		return pBegin + uOffset - 1;
	}
	const Element *GetNext(const Element *pPos) const noexcept {
		const auto pBegin = GetBegin();
		const auto uOffset = static_cast<std::size_t>(pPos - pBegin);
		if(uOffset + 1 == GetSize()){
			return nullptr;
		}
		return pBegin + uOffset + 1;
	}
	Element *GetNext(const Element *pPos) noexcept {
		const auto pBegin = GetBegin();
		const auto uOffset = static_cast<std::size_t>(pPos - pBegin);
		if(uOffset + 1 == GetSize()){
			return nullptr;
		}
		return pBegin + uOffset + 1;
	}

	ConstEnumerator EnumerateFirst() const noexcept {
		return ConstEnumerator(*this, GetFirst());
	}
	Enumerator EnumerateFirst() noexcept {
		return Enumerator(*this, GetFirst());
	}
	ConstEnumerator EnumerateConstFirst() const noexcept {
		return EnumerateFirst();
	}
	ConstEnumerator EnumerateLast() const noexcept {
		return ConstEnumerator(*this, GetLast());
	}
	Enumerator EnumerateLast() noexcept {
		return Enumerator(*this, GetLast());
	}
	ConstEnumerator EnumerateConstLast() const noexcept {
		return EnumerateLast();
	}
	constexpr ConstEnumerator EnumerateSingular() const noexcept {
		return ConstEnumerator(*this, nullptr);
	}
	Enumerator EnumerateSingular() noexcept {
		return Enumerator(*this, nullptr);
	}
	constexpr ConstEnumerator EnumerateConstSingular() const noexcept {
		return EnumerateSingular();
	}

	void Swap(String &rhs) noexcept {
		using std::swap;
		swap(x_vStorage, rhs.x_vStorage);
	}

	// String 需求。
	const Char *GetBegin() const noexcept {
		if(x_vStorage.vSmall.schComplLength >= 0){
			return x_vStorage.vSmall.achData;
		} else {
			return x_vStorage.vLarge.pchBegin;
		}
	}
	Char *GetBegin() noexcept {
		if(x_vStorage.vSmall.schComplLength >= 0){
			return x_vStorage.vSmall.achData;
		} else {
			return x_vStorage.vLarge.pchBegin;
		}
	}

	const Char *GetEnd() const noexcept {
		if(x_vStorage.vSmall.schComplLength >= 0){
			return x_vStorage.vSmall.achData + X_GetSmallLength();
		} else {
			return x_vStorage.vLarge.pchBegin + x_vStorage.vLarge.uLength;
		}
	}
	Char *GetEnd() noexcept {
		if(x_vStorage.vSmall.schComplLength >= 0){
			return x_vStorage.vSmall.achData + X_GetSmallLength();
		} else {
			return x_vStorage.vLarge.pchBegin + x_vStorage.vLarge.uLength;
		}
	}
	std::size_t GetSize() const noexcept {
		if(x_vStorage.vSmall.schComplLength >= 0){
			return X_GetSmallLength();
		} else {
			return x_vStorage.vLarge.uLength;
		}
	}

	const Char *GetData() const noexcept {
		return GetBegin();
	}
	Char *GetData() noexcept {
		return GetBegin();
	}
	const Char *GetConstData() const noexcept {
		return GetData();
	}
	const Char *GetStr() const noexcept {
		if(x_vStorage.vSmall.schComplLength >= 0){
			const_cast<Char &>(x_vStorage.vSmall.achData[X_GetSmallLength()]) = Char();
			return x_vStorage.vSmall.achData;
		} else {
			x_vStorage.vLarge.pchBegin[x_vStorage.vLarge.uLength] = Char();
			return x_vStorage.vLarge.pchBegin;
		}
	}
	Char *GetStr() noexcept {
		if(x_vStorage.vSmall.schComplLength >= 0){
			x_vStorage.vSmall.achData[X_GetSmallLength()] = Char();
			return x_vStorage.vSmall.achData;
		} else {
			x_vStorage.vLarge.pchBegin[x_vStorage.vLarge.uLength] = Char();
			return x_vStorage.vLarge.pchBegin;
		}
	}
	const Char *GetConstStr() const noexcept {
		return GetStr();
	}
	std::size_t GetLength() const noexcept {
		return GetSize();
	}

	const Char &Get(std::size_t uIndex) const {
		if(uIndex >= GetSize()){
			DEBUG_THROW(Exception, ERROR_INVALID_PARAMETER, RefCountingNtmbs::View(__PRETTY_FUNCTION__));
		}
		return UncheckedGet(uIndex);
	}
	Char &Get(std::size_t uIndex){
		if(uIndex >= GetSize()){
			DEBUG_THROW(Exception, ERROR_INVALID_PARAMETER, RefCountingNtmbs::View(__PRETTY_FUNCTION__));
		}
		return UncheckedGet(uIndex);
	}
	const Char &UncheckedGet(std::size_t uIndex) const noexcept {
		ASSERT(uIndex < GetSize());

		return GetBegin()[uIndex];
	}
	Char &UncheckedGet(std::size_t uIndex) noexcept {
		ASSERT(uIndex < GetSize());

		return GetBegin()[uIndex];
	}

	View GetView() const noexcept {
		if(x_vStorage.vSmall.schComplLength >= 0){
			return View(x_vStorage.vSmall.achData, X_GetSmallLength());
		} else {
			return View(x_vStorage.vLarge.pchBegin, x_vStorage.vLarge.uLength);
		}
	}

	std::size_t GetCapacity() const noexcept {
		if(x_vStorage.vSmall.schComplLength >= 0){
			return CountOf(x_vStorage.vSmall.achData);
		} else {
			return x_vStorage.vLarge.uSizeAllocated - 1;
		}
	}
	void Reserve(std::size_t uNewCapacity){
		if(uNewCapacity > GetCapacity()){
			const auto uOldSize = GetSize();
			X_ChopAndSplice(uOldSize, uOldSize, 0, uNewCapacity);
		}
	}
	void ReserveMore(std::size_t uDeltaCapacity){
		const auto uOldSize = GetSize();
		const auto uNewCapacity = uOldSize + uDeltaCapacity;
		if(uNewCapacity < uOldSize){
			throw std::bad_array_new_length();
		}
		Reserve(uNewCapacity);
	}

	void Resize(std::size_t uNewSize){
		const std::size_t uOldSize = GetSize();
		if(uNewSize > uOldSize){
			Reserve(uNewSize);
			X_SetSize(uNewSize);
		} else if(uNewSize < uOldSize){
			Pop(uOldSize - uNewSize);
		}
	}
	Char *ResizeMore(std::size_t uDeltaSize){
		const auto uOldSize = GetSize();
		const auto uNewSize = uOldSize + uDeltaSize;
		if(uNewSize < uOldSize){
			throw std::bad_array_new_length();
		}
		X_ChopAndSplice(uOldSize, uOldSize, 0, uNewSize);
		X_SetSize(uNewSize);
		return GetData() + uOldSize;
	}
	std::pair<Char *, std::size_t> ResizeToCapacity() noexcept {
		const auto uOldSize = GetSize();
		const auto uNewSize = GetCapacity();
		X_SetSize(uNewSize);
		return std::make_pair(GetData() + uOldSize, uNewSize - uOldSize);
	}
	void Shrink() noexcept {
		const auto uSzLen = View(GetStr()).GetLength();
		ASSERT(uSzLen <= GetSize());
		X_SetSize(uSzLen);
	}

	int Compare(const View &rhs) const noexcept {
		return GetView().Compare(rhs);
	}
	int Compare(const String &rhs) const noexcept {
		return GetView().Compare(rhs.GetView());
	}

	void Assign(Char ch, std::size_t uCount = 1){
		Resize(uCount);
		FillN(GetStr(), uCount, ch);
	}
	void Assign(const Char *pszBegin){
		Assign(View(pszBegin));
	}
	void Assign(const Char *pchBegin, const Char *pchEnd){
		Assign(View(pchBegin, pchEnd));
	}
	void Assign(const Char *pchBegin, std::size_t uCount){
		Assign(View(pchBegin, uCount));
	}
	void Assign(const View &rhs){
		Resize(rhs.GetSize());
		Copy(GetStr(), rhs.GetBegin(), rhs.GetEnd());
	}
	void Assign(std::initializer_list<Char> rhs){
		Assign(View(rhs));
	}
	template<StringType kOtherTypeT>
	void Assign(const StringView<kOtherTypeT> &rhs){
		String strTemp;
		strTemp.Append(rhs);
		Assign(std::move(strTemp));
	}
	template<StringType kOtherTypeT>
	void Assign(const String<kOtherTypeT> &rhs){
		Assign(StringView<kOtherTypeT>(rhs));
	}
	void Assign(const String &rhs){
		if(&rhs != this){
			Assign(View(rhs));
		}
	}
	void Assign(String &&rhs) noexcept {
		ASSERT(this != &rhs);

		if(x_vStorage.vSmall.schComplLength < 0){
			::operator delete[](x_vStorage.vLarge.pchBegin);
		}
		x_vStorage = rhs.x_vStorage;
#ifndef NDEBUG
		std::memset(rhs.x_vStorage.vSmall.achData, 0xCD, sizeof(rhs.x_vStorage.vSmall.achData));
#endif
		rhs.x_vStorage.vSmall.schComplLength = 0;
	}

	void Push(Char ch){
		Append(ch, 1);
	}
	void UncheckedPush(Char ch) noexcept {
		ASSERT(GetLength() < GetCapacity());

		if(x_vStorage.vSmall.schComplLength >= 0){
			x_vStorage.vSmall.achData[X_GetSmallLength()] = ch;
			--x_vStorage.vSmall.schComplLength;
		} else {
			x_vStorage.vLarge.pchBegin[x_vStorage.vLarge.uLength] = ch;
			++x_vStorage.vLarge.uLength;
		}
	}
	void Pop(std::size_t uCount = 1) noexcept {
		const auto uOldSize = GetSize();
		ASSERT(uOldSize >= uCount);
		X_SetSize(uOldSize - uCount);
	}

	void Append(Char ch, std::size_t uCount = 1){
		FillN(ResizeMore(uCount), uCount, ch);
	}
	void Append(const Char *pszBegin){
		Append(View(pszBegin));
	}
	void Append(const Char *pchBegin, const Char *pchEnd){
		Append(View(pchBegin, pchEnd));
	}
	void Append(const Char *pchBegin, std::size_t uCount){
		Append(View(pchBegin, uCount));
	}
	void Append(const View &rhs){
		const auto pWrite = ResizeMore(rhs.GetSize());
		Copy(pWrite, rhs.GetBegin(), rhs.GetEnd());
	}
	void Append(std::initializer_list<Char> rhs){
		Append(View(rhs));
	}
	void Append(const String &rhs){
		const auto pWrite = ResizeMore(rhs.GetSize());
		Copy(pWrite, rhs.GetBegin(), rhs.GetEnd()); // 这是正确的即使对于 &rhs == this 的情况。
	}
	template<StringType kOtherTypeT>
	void Append(const StringView<kOtherTypeT> &rhs){
		constexpr int kConvertViaUtf16Weight = StringEncodingTrait<kTypeT>::kPrefersConversionViaUtf16 + StringEncodingTrait<kOtherTypeT>::kPrefersConversionViaUtf16;
		constexpr int kConvertViaUtf32Weight = StringEncodingTrait<kTypeT>::kPrefersConversionViaUtf32 + StringEncodingTrait<kOtherTypeT>::kPrefersConversionViaUtf32;
		using UnifiedString = String<(kConvertViaUtf16Weight > kConvertViaUtf32Weight) ? StringType::kUtf16 : StringType::kUtf32>;

		UnifiedString usTemp;
		const auto usvResult = String<kOtherTypeT>::UnifyAssign(usTemp, rhs);
		DeunifyAppend(*this, usvResult);
	}
	template<StringType kOtherTypeT>
	void Append(const String<kOtherTypeT> &rhs){
		Append(rhs.GetView());
	}

	View Slice(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd = -1) const noexcept {
		return GetView().Slice(nBegin, nEnd);
	}
	String SliceStr(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd = -1) const {
		return String(Slice(nBegin, nEnd));
	}

	void Reverse() noexcept {
		auto pchBegin = GetBegin();
		auto pchEnd = GetEnd();
		if(pchBegin != pchEnd){
			--pchEnd;
			while(pchBegin < pchEnd){
				std::iter_swap(pchBegin++, pchEnd--);
			}
		}
	}

	std::size_t Find(const View &vToFind, std::ptrdiff_t nBegin = 0) const noexcept {
		return GetView().Find(vToFind, nBegin);
	}
	std::size_t FindBackward(const View &vToFind, std::ptrdiff_t nEnd = -1) const noexcept {
		return GetView().FindBackward(vToFind, nEnd);
	}
	std::size_t FindRep(Char chToFind, std::size_t uRepCount, std::ptrdiff_t nBegin = 0) const noexcept {
		return GetView().FindRep(chToFind, uRepCount, nBegin);
	}
	std::size_t FindRepBackward(Char chToFind, std::size_t uRepCount, std::ptrdiff_t nEnd = -1) const noexcept {
		return GetView().FindRepBackward(chToFind, uRepCount, nEnd);
	}
	std::size_t Find(Char chToFind, std::ptrdiff_t nBegin = 0) const noexcept {
		return GetView().Find(chToFind, nBegin);
	}
	std::size_t FindBackward(Char chToFind, std::ptrdiff_t nEnd = -1) const noexcept {
		return GetView().FindBackward(chToFind, nEnd);
	}

	void Replace(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd, Char chRep, std::size_t uRepSize = 1){
		const auto vCurrent = GetView();
		const auto uOldLength = vCurrent.GetLength();

		const auto vRemoved = vCurrent.Slice(nBegin, nEnd);
		const auto uRemovedBegin = (std::size_t)(vRemoved.GetBegin() - vCurrent.GetBegin());
		const auto uRemovedEnd = (std::size_t)(vRemoved.GetEnd() - vCurrent.GetBegin());
		const auto uLengthAfterRemoved = uOldLength - (uRemovedEnd - uRemovedBegin);
		const auto uNewLength = uLengthAfterRemoved + uRepSize;
		if(uNewLength < uLengthAfterRemoved){
			throw std::bad_array_new_length();
		}

		const auto pchWrite = X_ChopAndSplice(uRemovedBegin, uRemovedEnd, 0, uRemovedBegin + uRepSize);
		FillN(pchWrite, uRepSize, chRep);
		X_SetSize(uNewLength);
	}
	void Replace(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd, const Char *pchRepBegin){
		Replace(nBegin, nEnd, View(pchRepBegin));
	}
	void Replace(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd, const Char *pchRepBegin, const Char *pchRepEnd){
		Replace(nBegin, nEnd, View(pchRepBegin, pchRepEnd));
	}
	void Replace(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd, const Char *pchRepBegin, std::size_t uLen){
		Replace(nBegin, nEnd, View(pchRepBegin, uLen));
	}
	void Replace(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd, const View &vRep){
		const auto uRepSize = vRep.GetLength();

		const auto vCurrent = GetView();
		const auto uOldLength = vCurrent.GetLength();

		const auto vRemoved = vCurrent.Slice(nBegin, nEnd);
		const auto uRemovedBegin = (std::size_t)(vRemoved.GetBegin() - vCurrent.GetBegin());
		const auto uRemovedEnd = (std::size_t)(vRemoved.GetEnd() - vCurrent.GetBegin());
		const auto uLengthAfterRemoved = uOldLength - (uRemovedEnd - uRemovedBegin);
		const auto uNewLength = uLengthAfterRemoved + uRepSize;
		if(uNewLength < uLengthAfterRemoved){
			throw std::bad_array_new_length();
		}

		if(vCurrent.DoesOverlapWith(vRep)){
			String strTemp;
			strTemp.Resize(uNewLength);
			auto pchWrite = strTemp.GetStr();
			pchWrite = Copy(pchWrite, vCurrent.GetBegin(), vCurrent.GetBegin() + uRemovedBegin);
			pchWrite = Copy(pchWrite, vRep.GetBegin(), vRep.GetEnd());
			pchWrite = Copy(pchWrite, vCurrent.GetBegin() + uRemovedEnd, vCurrent.GetEnd());
			Assign(std::move(strTemp));
		} else {
			const auto pchWrite = X_ChopAndSplice(uRemovedBegin, uRemovedEnd, 0, uRemovedBegin + vRep.GetSize());
			CopyN(pchWrite, vRep.GetBegin(), vRep.GetSize());
			X_SetSize(uNewLength);
		}
	}

public:
	operator View() const noexcept {
		return GetView();
	}

	explicit operator bool() const noexcept {
		return !IsEmpty();
	}
	explicit operator const Char *() const noexcept {
		return GetStr();
	}
	explicit operator Char *() noexcept {
		return GetStr();
	}
	const Char &operator[](std::size_t uIndex) const noexcept {
		return UncheckedGet(uIndex);
	}
	Char &operator[](std::size_t uIndex) noexcept {
		return UncheckedGet(uIndex);
	}
};

template<StringType kTypeT>
const String<kTypeT> String<kTypeT>::kEmpty;

template<StringType kTypeT, StringType kOtherTypeT>
String<kTypeT> &operator+=(String<kTypeT> &lhs, const String<kOtherTypeT> &rhs){
	lhs.Append(rhs);
	return lhs;
}
template<StringType kTypeT, StringType kOtherTypeT>
String<kTypeT> &operator+=(String<kTypeT> &lhs, const StringView<kOtherTypeT> &rhs){
	lhs.Append(rhs);
	return lhs;
}
template<StringType kTypeT>
String<kTypeT> &operator+=(String<kTypeT> &lhs, typename String<kTypeT>::Char rhs){
	lhs.Append(rhs);
	return lhs;
}
template<StringType kTypeT>
String<kTypeT> &operator+=(String<kTypeT> &lhs, const typename String<kTypeT>::Char *rhs){
	lhs.Append(rhs);
	return lhs;
}
template<StringType kTypeT, StringType kOtherTypeT>
String<kTypeT> &&operator+=(String<kTypeT> &&lhs, const String<kOtherTypeT> &rhs){
	lhs.Append(rhs);
	return std::move(lhs);
}
template<StringType kTypeT, StringType kOtherTypeT>
String<kTypeT> &&operator+=(String<kTypeT> &&lhs, const StringView<kOtherTypeT> &rhs){
	lhs.Append(rhs);
	return std::move(lhs);
}
template<StringType kTypeT>
String<kTypeT> &&operator+=(String<kTypeT> &&lhs, typename String<kTypeT>::Char rhs){
	lhs.Append(rhs);
	return std::move(lhs);
}
template<StringType kTypeT>
String<kTypeT> &&operator+=(String<kTypeT> &&lhs, const typename String<kTypeT>::Char *rhs){
	lhs.Append(rhs);
	return std::move(lhs);
}
template<StringType kTypeT>
String<kTypeT> &&operator+=(String<kTypeT> &&lhs, String<kTypeT> &&rhs){
	lhs.Append(std::move(rhs));
	return std::move(lhs);
}

template<StringType kTypeT, StringType kOtherTypeT>
String<kTypeT> operator+(const String<kTypeT> &lhs, const String<kOtherTypeT> &rhs){
	String<kTypeT> strRet(lhs);
	strRet += rhs;
	return strRet;
}
template<StringType kTypeT, StringType kOtherTypeT>
String<kTypeT> operator+(const String<kTypeT> &lhs, const StringView<kOtherTypeT> &rhs){
	String<kTypeT> strRet(lhs);
	strRet += rhs;
	return strRet;
}
template<StringType kTypeT>
String<kTypeT> operator+(const String<kTypeT> &lhs, typename String<kTypeT>::Char rhs){
	String<kTypeT> strRet(lhs);
	strRet += rhs;
	return strRet;
}
template<StringType kTypeT>
String<kTypeT> operator+(const String<kTypeT> &lhs, const typename String<kTypeT>::Char *rhs){
	String<kTypeT> strRet(lhs);
	strRet += rhs;
	return strRet;
}
template<StringType kTypeT, StringType kOtherTypeT>
String<kTypeT> &&operator+(String<kTypeT> &&lhs, const String<kOtherTypeT> &rhs){
	return std::move(lhs += rhs);
}
template<StringType kTypeT, StringType kOtherTypeT>
String<kTypeT> &&operator+(String<kTypeT> &&lhs, const StringView<kOtherTypeT> &rhs){
	return std::move(lhs += rhs);
}
template<StringType kTypeT>
String<kTypeT> &&operator+(String<kTypeT> &&lhs, typename String<kTypeT>::Char rhs){
	return std::move(lhs += rhs);
}
template<StringType kTypeT>
String<kTypeT> &&operator+(String<kTypeT> &&lhs, const typename String<kTypeT>::Char *rhs){
	return std::move(lhs += rhs);
}
template<StringType kTypeT>
String<kTypeT> &&operator+(String<kTypeT> &&lhs, String<kTypeT> &&rhs){
	return std::move(lhs += std::move(rhs));
}

template<StringType kTypeT>
bool operator==(const String<kTypeT> &lhs, const String<kTypeT> &rhs) noexcept {
	return lhs.GetView() == rhs.GetView();
}
template<StringType kTypeT>
bool operator==(const String<kTypeT> &lhs, const StringView<kTypeT> &rhs) noexcept {
	return lhs.GetView() == rhs;
}
template<StringType kTypeT>
bool operator==(const StringView<kTypeT> &lhs, const String<kTypeT> &rhs) noexcept {
	return lhs == rhs.GetView();
}

template<StringType kTypeT>
bool operator!=(const String<kTypeT> &lhs, const String<kTypeT> &rhs) noexcept {
	return lhs.GetView() != rhs.GetView();
}
template<StringType kTypeT>
bool operator!=(const String<kTypeT> &lhs, const StringView<kTypeT> &rhs) noexcept {
	return lhs.GetView() != rhs;
}
template<StringType kTypeT>
bool operator!=(const StringView<kTypeT> &lhs, const String<kTypeT> &rhs) noexcept {
	return lhs != rhs.GetView();
}

template<StringType kTypeT>
bool operator<(const String<kTypeT> &lhs, const String<kTypeT> &rhs) noexcept {
	return lhs.GetView() < rhs.GetView();
}
template<StringType kTypeT>
bool operator<(const String<kTypeT> &lhs, const StringView<kTypeT> &rhs) noexcept {
	return lhs.GetView() < rhs;
}
template<StringType kTypeT>
bool operator<(const StringView<kTypeT> &lhs, const String<kTypeT> &rhs) noexcept {
	return lhs < rhs.GetView();
}

template<StringType kTypeT>
bool operator>(const String<kTypeT> &lhs, const String<kTypeT> &rhs) noexcept {
	return lhs.GetView() > rhs.GetView();
}
template<StringType   kTypeT>
bool operator>(const String<kTypeT> &lhs, const StringView<kTypeT> &rhs) noexcept {
	return lhs.GetView() > rhs;
}
template<StringType kTypeT>
bool operator>(const StringView<kTypeT> &lhs, const String<kTypeT> &rhs) noexcept {
	return lhs > rhs.GetView();
}

template<StringType kTypeT>
bool operator<=(const String<kTypeT> &lhs, const String<kTypeT> &rhs) noexcept {
	return lhs.GetView() <= rhs.GetView();
}
template<StringType kTypeT>
bool operator<=(const String<kTypeT> &lhs, const StringView<kTypeT> &rhs) noexcept {
	return lhs.GetView() <= rhs;
}
template<StringType kTypeT>
bool operator<=(const StringView<kTypeT> &lhs, const String<kTypeT> &rhs) noexcept {
	return lhs <= rhs.GetView();
}

template<StringType kTypeT>
bool operator>=(const String<kTypeT> &lhs, const String<kTypeT> &rhs) noexcept {
	return lhs.GetView() >= rhs.GetView();
}
template<StringType kTypeT>
bool operator>=(const String<kTypeT> &lhs, const StringView<kTypeT> &rhs) noexcept {
	return lhs.GetView() >= rhs;
}
template<StringType kTypeT>
bool operator>=(const StringView<kTypeT> &lhs, const String<kTypeT> &rhs) noexcept {
	return lhs >= rhs.GetView();
}

template<StringType kTypeT>
void swap(String<kTypeT> &lhs, String<kTypeT> &rhs) noexcept {
	lhs.Swap(rhs);
}

template<StringType kTypeT>
decltype(auto) begin(const String<kTypeT> &rhs) noexcept {
	return rhs.GetBegin();
}
template<StringType kTypeT>
decltype(auto) begin(String<kTypeT> &rhs) noexcept {
	return rhs.GetBegin();
}
template<StringType kTypeT>
decltype(auto) cbegin(const String<kTypeT> &rhs) noexcept {
	return begin(rhs);
}
template<StringType kTypeT>
decltype(auto) end(const String<kTypeT> &rhs) noexcept {
	return rhs.GetEnd();
}
template<StringType kTypeT>
decltype(auto) end(String<kTypeT> &rhs) noexcept {
	return rhs.GetEnd();
}
template<StringType kTypeT>
decltype(auto) cend(const String<kTypeT> &rhs) noexcept {
	return end(rhs);
}

extern template class String<StringType::kNarrow>;
extern template class String<StringType::kWide>;
extern template class String<StringType::kUtf8>;
extern template class String<StringType::kUtf16>;
extern template class String<StringType::kUtf32>;
extern template class String<StringType::kCesu8>;
extern template class String<StringType::kAnsi>;

using NarrowString = String<StringType::kNarrow>;
using WideString   = String<StringType::kWide>;
using Utf8String   = String<StringType::kUtf8>;
using Utf16String  = String<StringType::kUtf16>;
using Utf32String  = String<StringType::kUtf32>;
using Cesu8String  = String<StringType::kCesu8>;
using AnsiString   = String<StringType::kAnsi>;

// 字面量运算符。
template<typename CharT, CharT ...kCharsT>
extern inline const auto &operator""_ns(){
	static const NarrowString s_nsRet{ kCharsT... };
	return s_nsRet;
}
template<typename CharT, CharT ...kCharsT>
extern inline const auto &operator""_ws(){
	static const WideString s_wsRet{ kCharsT... };
	return s_wsRet;
}
template<typename CharT, CharT ...kCharsT>
extern inline const auto &operator""_u8s(){
	static const Utf8String s_u8sRet{ kCharsT... };
	return s_u8sRet;
}
template<typename CharT, CharT ...kCharsT>
extern inline const auto &operator""_u16s(){
	static const Utf16String s_u16sRet{ kCharsT... };
	return s_u16sRet;
}
template<typename CharT, CharT ...kCharsT>
extern inline const auto &operator""_u32s(){
	static const Utf32String s_u32sRet{ kCharsT... };
	return s_u32sRet;
}

}

#endif
