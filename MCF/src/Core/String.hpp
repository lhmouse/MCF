// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_STRING_HPP_
#define MCF_CORE_STRING_HPP_

#include "StringView.hpp"
#include "_CheckedSizeArithmetic.hpp"
#include "_Enumerator.hpp"
#include "Assert.hpp"
#include "CountOf.hpp"
#include "CopyMoveFill.hpp"
#include <initializer_list>
#include <type_traits>
#include <cstring>
#include <cstddef>
#include <cstdint>

// 借用了 https://github.com/elliotgoodrich/SSO-23 的一些想法。然而我们可以优化更多……
// 我们支持的最大 SSO 长度是 31 个“字符”，而不是 23 个“字节”。

namespace MCF {

template<Impl_StringTraits::Type kTypeT>
class String;

namespace Impl_String {
	template<Impl_StringTraits::Type kSrcTypeT>
	struct Transcoder;
}

template<Impl_StringTraits::Type kTypeT>
class String {
public:
	enum : std::size_t { kNpos = StringView<kTypeT>::kNpos };

	using View  = StringView<kTypeT>;
	using Char  = typename View::Char;

	// 容器需求。
	using Element         = Char;
	using ConstEnumerator = Impl_Enumerator::ConstEnumerator <String>;
	using Enumerator      = Impl_Enumerator::Enumerator      <String>;

public:
	static void UnifyAppend(String<Impl_StringTraits::Type::kUtf16> &u16sDst, const View &svSrc);
	static void DeunifyAppend(String &strDst, const StringView<Impl_StringTraits::Type::kUtf16> &u16svSrc);

	static void UnifyAppend(String<Impl_StringTraits::Type::kUtf32> &u32sDst, const View &svSrc);
	static void DeunifyAppend(String &strDst, const StringView<Impl_StringTraits::Type::kUtf32> &u32svSrc);

private:
	__extension__ union {
		struct {
			Char x_achData[31];
			std::make_signed_t<Char> x_schComplLength;
		};
		struct {
			Char *x_pchData;
			std::size_t x_uSize;
			std::size_t x_uCapacity;
		};
	};

public:
	String() noexcept {
#ifndef NDEBUG
		std::memset(x_achData, 0xCA, sizeof(x_achData));
#endif
		x_schComplLength = static_cast<std::make_signed_t<Char>>(CountOf(x_achData));
	}
	explicit String(Char chFill, std::size_t uCount = 1)
		: String()
	{
		Append(chFill, uCount);
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
	explicit String(const View &svOther)
		: String()
	{
		Append(svOther);
	}
	String(std::initializer_list<Char> ilElements)
		: String()
	{
		Append(ilElements);
	}
	template<Impl_StringTraits::Type kOtherTypeT>
	explicit String(const StringView<kOtherTypeT> &svOther)
		: String()
	{
		Append(svOther);
	}
	template<Impl_StringTraits::Type kOtherTypeT>
	explicit String(const String<kOtherTypeT> &strOther)
		: String()
	{
		Append(strOther);
	}
	String(const String &svOther)
		: String()
	{
		Append(svOther);
	}
	String(String &&svOther) noexcept
		: String()
	{
		Swap(svOther);
	}
	String &operator=(Char chFill){
		Assign(chFill, 1);
		return *this;
	}
	String &operator=(const Char *pszOther){
		Assign(pszOther);
		return *this;
	}
	String &operator=(const View &svOther){
		Assign(svOther);
		return *this;
	}
	template<Impl_StringTraits::Type kOtherTypeT>
	String &operator=(const StringView<kOtherTypeT> &svOther){
		Assign(svOther);
		return *this;
	}
	template<Impl_StringTraits::Type kOtherTypeT>
	String &operator=(const String<kOtherTypeT> &svOther){
		Assign(svOther);
		return *this;
	}
	String &operator=(const String &svOther){
		Assign(svOther);
		return *this;
	}
	String &operator=(String &&svOther) noexcept {
		Assign(std::move(svOther));
		return *this;
	}
	~String() noexcept {
		if(x_schComplLength < 0){
			::operator delete[](x_pchData);
		}
#ifndef NDEBUG
		std::memset(this, 0xDD, sizeof(*this));
#endif
	}

private:
	Char *X_ChopAndSplice(std::size_t uRemovedBegin, std::size_t uRemovedEnd, std::size_t uFirstOffset, std::size_t uThirdOffset){
		const auto pchOldBuffer = GetBegin();
		const auto uOldSize = GetSize();
		auto pchNewBuffer = pchOldBuffer;
		const auto uNewSize = uThirdOffset + (uOldSize - uRemovedEnd);
		auto uCharsToAlloc = uNewSize + 1;

		MCF_DEBUG_CHECK(uRemovedBegin <= uOldSize);
		MCF_DEBUG_CHECK(uRemovedEnd <= uOldSize);
		MCF_DEBUG_CHECK(uRemovedBegin <= uRemovedEnd);
		MCF_DEBUG_CHECK(uFirstOffset + uRemovedBegin <= uThirdOffset);

		const auto uOldCapacity = GetCapacity();
		if(uOldCapacity < uNewSize){
			uCharsToAlloc += (uCharsToAlloc >> 1);
			uCharsToAlloc = (uCharsToAlloc + 0x0F) & static_cast<std::size_t>(-0x10);
			if(uCharsToAlloc < uNewSize + 1){
				uCharsToAlloc = uNewSize + 1;
			}
			const auto uBytesToAlloc = Impl_CheckedSizeArithmetic::Mul(sizeof(Char), uCharsToAlloc);
			pchNewBuffer = (Char *)::operator new[](uBytesToAlloc);
		}

		if((pchNewBuffer + uFirstOffset != pchOldBuffer) && (uRemovedBegin != 0)){
			std::memmove(pchNewBuffer + uFirstOffset, pchOldBuffer, uRemovedBegin * sizeof(Char));
		}
		if((pchNewBuffer + uThirdOffset != pchOldBuffer + uRemovedEnd) && (uOldSize != uRemovedEnd)){
			std::memmove(pchNewBuffer + uThirdOffset, pchOldBuffer + uRemovedEnd, (uOldSize - uRemovedEnd) * sizeof(Char));
		}

		if(pchNewBuffer != pchOldBuffer){
			if(x_schComplLength >= 0){
				x_schComplLength = -1;
			} else {
				::operator delete[](pchOldBuffer);
			}

			x_pchData = pchNewBuffer;
			x_uSize = uOldSize;
			x_uCapacity = uCharsToAlloc - 1;
		}

		return pchNewBuffer + uFirstOffset + uRemovedBegin;
	}
	void X_GetStorage(Char **ppchData, std::size_t *puSize, std::size_t *puCapacity) const noexcept {
		if(x_schComplLength >= 0){
			if(ppchData){
				*ppchData = const_cast<Char *>(x_achData);
			}
			if(puSize){
				*puSize = CountOf(x_achData) - static_cast<std::make_unsigned_t<Char>>(x_schComplLength);
			}
			if(puCapacity){
				*puCapacity = CountOf(x_achData);
			}
		} else {
			if(ppchData){
				*ppchData = x_pchData;
			}
			if(puSize){
				*puSize = x_uSize;
			}
			if(puCapacity){
				*puCapacity = x_uCapacity;
			}
		}
	}
	void X_SetSize(std::size_t uNewSize) noexcept {
		MCF_DEBUG_CHECK(uNewSize <= GetCapacity());

		if(x_schComplLength >= 0){
			x_schComplLength = static_cast<std::make_signed_t<Char>>(CountOf(x_achData) - uNewSize);
		} else {
			x_uSize = uNewSize;
		}
	}

public:
	// 容器需求。
	bool IsEmpty() const noexcept {
		return GetSize() == 0;
	}
	void Clear() noexcept {
		X_SetSize(0);
	}
	template<typename OutputIteratorT>
	OutputIteratorT Extract(OutputIteratorT itOutput){
		try {
			const auto pBegin = GetBegin();
			const auto pEnd = GetEnd();
			for(auto p = pBegin; p != pEnd; ++p){
				*itOutput = *p;
				++itOutput;
			}
		} catch(...){
			Clear();
			throw;
		}
		Clear();
		return itOutput;
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

	void Swap(String &strOther) noexcept {
		unsigned char abyTemp[sizeof(*this)];
		std::memcpy(abyTemp, this, sizeof(*this));
		std::memcpy(this, &strOther, sizeof(*this));
		std::memcpy(&strOther, abyTemp, sizeof(*this));
	}

	// String 需求。
	const Char *GetBegin() const noexcept {
		Char *pchData;
		X_GetStorage(&pchData, nullptr, nullptr);
		return pchData;
	}
	Char *GetBegin() noexcept {
		Char *pchData;
		X_GetStorage(&pchData, nullptr, nullptr);
		return pchData;
	}
	const Char *GetConstBegin() const noexcept {
		return GetBegin();
	}

	const Char *GetEnd() const noexcept {
		Char *pchData;
		std::size_t uSize;
		X_GetStorage(&pchData, &uSize, nullptr);
		return pchData + uSize;
	}
	Char *GetEnd() noexcept {
		Char *pchData;
		std::size_t uSize;
		X_GetStorage(&pchData, &uSize, nullptr);
		return pchData + uSize;
	}
	const Char *GetConstEnd() const noexcept {
		return GetEnd();
	}

	const Char *GetData() const noexcept {
		Char *pchData;
		X_GetStorage(&pchData, nullptr, nullptr);
		return pchData;
	}
	Char *GetData() noexcept {
		Char *pchData;
		X_GetStorage(&pchData, nullptr, nullptr);
		return pchData;
	}
	const Char *GetConstData() const noexcept {
		return GetData();
	}
	std::size_t GetSize() const noexcept {
		std::size_t uSize;
		X_GetStorage(nullptr, &uSize, nullptr);
		return uSize;
	}
	const Char *GetStr() const noexcept {
		Char *pchData;
		std::size_t uSize;
		X_GetStorage(&pchData, &uSize, nullptr);
		if(pchData[uSize] != Char()){
			pchData[uSize] = Char();
		}
		return  pchData;
	}
	Char *GetStr() noexcept {
		Char *pchData;
		std::size_t uSize;
		X_GetStorage(&pchData, &uSize, nullptr);
		if(pchData[uSize] != Char()){
			pchData[uSize] = Char();
		}
		return  pchData;
	}
	const Char *GetConstStr() const noexcept {
		return GetStr();
	}
	std::size_t GetLength() const noexcept {
		std::size_t uSize;
		X_GetStorage(nullptr, &uSize, nullptr);
		return uSize;
	}

	View GetView() const noexcept {
		Char *pchData;
		std::size_t uSize;
		X_GetStorage(&pchData, &uSize, nullptr);
		return View(pchData, uSize);
	}

	const Char &Get(std::size_t uIndex) const {
		if(uIndex > GetSize()){
			MCF_THROW(Exception, ERROR_ACCESS_DENIED, Rcntws::View(L"String: 下标越界。"));
		}
		return UncheckedGet(uIndex);
	}
	Char &Get(std::size_t uIndex){
		if(uIndex > GetSize()){
			MCF_THROW(Exception, ERROR_ACCESS_DENIED, Rcntws::View(L"String: 下标越界。"));
		}
		return UncheckedGet(uIndex);
	}
	const Char &UncheckedGet(std::size_t uIndex) const noexcept {
		MCF_DEBUG_CHECK(uIndex <= GetSize());

		return GetStr()[uIndex];
	}
	Char &UncheckedGet(std::size_t uIndex) noexcept {
		MCF_DEBUG_CHECK(uIndex <= GetSize());

		return GetStr()[uIndex];
	}

	std::size_t GetCapacity() const noexcept {
		std::size_t uCapacity;
		X_GetStorage(nullptr, nullptr, &uCapacity);
		return uCapacity;
	}
	std::size_t GetCapacityRemaining() const noexcept {
		std::size_t uSize;
		std::size_t uCapacity;
		X_GetStorage(nullptr, &uSize, &uCapacity);
		return uCapacity - uSize;
	}
	void Reserve(std::size_t uNewCapacity){
		const auto uOldCapacity = GetCapacity();
		if(uNewCapacity <= uOldCapacity){
			return;
		}
		const auto uOldSize = GetSize();
		X_ChopAndSplice(uOldSize, uOldSize, 0, uNewCapacity);
	}
	void ReserveMore(std::size_t uDeltaCapacity){
		const auto uOldSize = GetSize();
		const auto uNewCapacity = Impl_CheckedSizeArithmetic::Add(uDeltaCapacity, uOldSize);
		Reserve(uNewCapacity);
	}

	Char *Resize(std::size_t uNewSize){
		const auto uOldSize = GetSize();
		if(uNewSize > uOldSize){
			Reserve(uNewSize);
			X_SetSize(uNewSize);
		} else if(uNewSize < uOldSize){
			Pop(uOldSize - uNewSize);
		}
		return GetData();
	}
	Char *ResizeMore(std::size_t uDeltaSize){
		const auto uOldSize = GetSize();
		const auto uNewSize = Impl_CheckedSizeArithmetic::Add(uDeltaSize, uOldSize);
		X_ChopAndSplice(uOldSize, uOldSize, 0, uNewSize);
		X_SetSize(uNewSize);
		return GetData() + uOldSize;
	}
	std::pair<Char *, std::size_t> ResizeToCapacity() noexcept {
		const auto uOldSize = GetSize();
		const auto uNewSize = GetCapacity();
		const auto uDeltaSize = uNewSize - uOldSize;
		X_SetSize(uNewSize);
		return std::make_pair(GetData() + uOldSize, uDeltaSize);
	}
	void ShrinkAsZeroTerminated() noexcept {
		const auto uSzLen = View(GetStr()).GetSize();
		MCF_DEBUG_CHECK(uSzLen <= GetSize());
		X_SetSize(uSzLen);
	}

	int Compare(const View &svOther) const noexcept {
		return GetView().Compare(svOther);
	}
	int Compare(const String &svOther) const noexcept {
		return GetView().Compare(svOther.GetView());
	}

	void Assign(Char chFill, std::size_t uCount = 1){
		Resize(uCount);
		FillN(GetData(), uCount, chFill);
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
	void Assign(const View &svOther){
		Resize(svOther.GetSize());
		Copy(GetData(), svOther.GetBegin(), svOther.GetEnd());
	}
	void Assign(std::initializer_list<Char> ilElements){
		Assign(View(ilElements.begin(), ilElements.end()));
	}
	template<Impl_StringTraits::Type kOtherTypeT>
	void Assign(const StringView<kOtherTypeT> &svOther){
		const auto uOldSize = GetSize();
		Append(svOther);
		if(uOldSize != 0){
			const auto pchData = GetData();
			const auto uNewSize = GetSize();
			Copy(pchData, pchData + uOldSize, pchData + uNewSize);
			Pop(uOldSize);
		}
	}
	template<Impl_StringTraits::Type kOtherTypeT>
	void Assign(const String<kOtherTypeT> &strOther){
		Assign(StringView<kOtherTypeT>(strOther));
	}
	void Assign(const String &strOther){
		if(&strOther != this){
			Assign(View(strOther));
		}
	}
	void Assign(String &&strOther) noexcept {
		MCF_DEBUG_CHECK(this != &strOther);

		if(x_schComplLength < 0){
			::operator delete[](x_pchData);
		}
		std::memcpy(this, &strOther, sizeof(*this));
#ifndef NDEBUG
		std::memset(&strOther, 0xCD, sizeof(*this));
#endif
		strOther.x_schComplLength = 0;
	}

	void Push(Char chFill){
		Append(chFill, 1);
	}
	void UncheckedPush(Char chFill) noexcept {
		MCF_DEBUG_CHECK(GetSize() < GetCapacity());

		if(x_schComplLength >= 0){
			x_achData[CountOf(x_achData) - static_cast<std::make_unsigned_t<Char>>(x_schComplLength)] = chFill;
			--x_schComplLength;
		} else {
			x_pchData[x_uSize] = chFill;
			++x_uSize;
		}
	}
	void Pop(std::size_t uCount = 1) noexcept {
		const auto uOldSize = GetSize();
		MCF_DEBUG_CHECK(uOldSize >= uCount);
		X_SetSize(uOldSize - uCount);
	}

	void Append(Char chFill, std::size_t uCount = 1){
		FillN(ResizeMore(uCount), uCount, chFill);
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
	void Append(const View &svOther){
		const auto pWrite = ResizeMore(svOther.GetSize());
		Copy(pWrite, svOther.GetBegin(), svOther.GetEnd());
	}
	void Append(std::initializer_list<Char> ilElements){
		Append(View(ilElements.begin(), ilElements.end()));
	}
	void Append(const String &strOther){
		const auto pWrite = ResizeMore(strOther.GetSize());
		Copy(pWrite, strOther.GetBegin(), strOther.GetEnd()); // 这是正确的即使对于 &strOther == this 的情况。
	}
	template<Impl_StringTraits::Type kOtherTypeT>
	void Append(const StringView<kOtherTypeT> &svOther){
		Impl_String::Transcoder<kOtherTypeT>()(*this, svOther);
	}
	template<Impl_StringTraits::Type kOtherTypeT>
	void Append(const String<kOtherTypeT> &strOther){
		Append(strOther.GetView());
	}

	View Slice(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd = -1) const noexcept {
		return GetView().Slice(nBegin, nEnd);
	}
	String SliceStr(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd = -1) const {
		return String(Slice(nBegin, nEnd));
	}

	void Reverse() noexcept {
		Char *pchData;
		std::size_t uSize;
		X_GetStorage(&pchData, &uSize, nullptr);
		for(std::size_t i = 0, j = uSize; i < j; ++i, --j){
			using std::swap;
			swap(pchData[i], pchData[j - 1]);
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

	void Replace(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd, Char chChar, std::size_t uCount = 1){
		const auto vCurrent = GetView();
		const auto uOldSize = vCurrent.GetSize();

		const auto vRemoved = vCurrent.Slice(nBegin, nEnd);
		const auto uRemovedBegin = static_cast<std::size_t>(vRemoved.GetBegin() - vCurrent.GetBegin());
		const auto uRemovedEnd = static_cast<std::size_t>(vRemoved.GetEnd() - vCurrent.GetBegin());
		const auto uLengthAfterRemoved = uOldSize - (uRemovedEnd - uRemovedBegin);
		const auto uNewSize = Impl_CheckedSizeArithmetic::Add(uCount, uLengthAfterRemoved);

		const auto pchWrite = X_ChopAndSplice(uRemovedBegin, uRemovedEnd, 0, uRemovedBegin + uCount);
		FillN(pchWrite, uCount, chChar);
		X_SetSize(uNewSize);
	}
	void Replace(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd, const Char *pchReplacementBegin){
		Replace(nBegin, nEnd, View(pchReplacementBegin));
	}
	void Replace(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd, const Char *pchReplacementBegin, const Char *pchReplacementEnd){
		Replace(nBegin, nEnd, View(pchReplacementBegin, pchReplacementEnd));
	}
	void Replace(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd, const Char *pchReplacementBegin, std::size_t uLen){
		Replace(nBegin, nEnd, View(pchReplacementBegin, uLen));
	}
	void Replace(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd, const View &vReplacement){
		const auto uReplacementSize = vReplacement.GetSize();

		const auto vCurrent = GetView();
		const auto uOldSize = vCurrent.GetSize();

		const auto vRemoved = vCurrent.Slice(nBegin, nEnd);
		const auto uRemovedBegin = static_cast<std::size_t>(vRemoved.GetBegin() - vCurrent.GetBegin());
		const auto uRemovedEnd = static_cast<std::size_t>(vRemoved.GetEnd() - vCurrent.GetBegin());
		const auto uLengthAfterRemoved = uOldSize - (uRemovedEnd - uRemovedBegin);
		const auto uNewSize = Impl_CheckedSizeArithmetic::Add(uReplacementSize, uLengthAfterRemoved);

		if(vCurrent.DoesOverlapWith(vReplacement)){
			String strTemp;
			strTemp.Resize(uNewSize);
			auto pchWrite = strTemp.GetData();
			pchWrite = Copy(pchWrite, vCurrent.GetBegin(), vCurrent.GetBegin() + uRemovedBegin);
			pchWrite = Copy(pchWrite, vReplacement.GetBegin(), vReplacement.GetEnd());
			pchWrite = Copy(pchWrite, vCurrent.GetBegin() + uRemovedEnd, vCurrent.GetEnd());
			Assign(std::move(strTemp));
		} else {
			const auto pchWrite = X_ChopAndSplice(uRemovedBegin, uRemovedEnd, 0, uRemovedBegin + vReplacement.GetSize());
			CopyN(pchWrite, vReplacement.GetBegin(), vReplacement.GetSize());
			X_SetSize(uNewSize);
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

	template<Impl_StringTraits::Type kOtherTypeT>
	String &operator+=(const String<kOtherTypeT> &strOther){
		Append(strOther);
		return *this;
	}
	template<Impl_StringTraits::Type kOtherTypeT>
	String &operator+=(const StringView<kOtherTypeT> &svOther){
		Append(svOther);
		return *this;
	}
	String &operator+=(Char chOther){
		Append(chOther);
		return *this;
	}
	String &operator+=(const Char *pszOther){
		Append(pszOther);
		return *this;
	}
	template<Impl_StringTraits::Type kOtherTypeT>
	friend String operator+(const String &strSelf, const StringView<kOtherTypeT> &svOther){
		auto strTemp = strSelf;
		strTemp += svOther;
		return strTemp;
	}
	template<Impl_StringTraits::Type kOtherTypeT>
	friend String operator+(String &&strSelf, const StringView<kOtherTypeT> &svOther){
		strSelf += svOther;
		return std::move(strSelf);
	}
	friend String operator+(const String &strSelf, Char chOther){
		auto strTemp = strSelf;
		strTemp += chOther;
		return strTemp;
	}
	friend String operator+(String &&strSelf, Char chOther){
		strSelf += chOther;
		return std::move(strSelf);
	}
	friend String operator+(const String &strSelf, const Char *pszOther){
		auto strTemp = strSelf;
		strTemp += pszOther;
		return strTemp;
	}
	friend String operator+(String &&strSelf, const Char *pszOther){
		strSelf += pszOther;
		return std::move(strSelf);
	}
	friend String operator+(Char chSelf, const String &strOther){
		auto strTemp = String(chSelf);
		strTemp += strOther;
		return strTemp;
	}
	friend String operator+(Char chSelf, String &&strOther){
		auto strTemp = String(chSelf);
		strTemp += std::move(strOther);
		return strTemp;
	}
	friend String operator+(const Char *pszSelf, const String &strOther){
		auto strTemp = String(pszSelf);
		strTemp += strOther;
		return strTemp;
	}
	friend String operator+(const Char *pszSelf, String &&strOther){
		auto strTemp = String(pszSelf);
		strTemp += std::move(strOther);
		return strTemp;
	}

	bool operator==(const String &strOther) const noexcept {
		return GetView() == strOther.GetView();
	}
	bool operator==(const View &svOther) const noexcept {
		return GetView() == svOther;
	}
	friend bool operator==(const View &svSelf, const String &strOther) noexcept {
		return svSelf == strOther.GetView();
	}

	bool operator!=(const String &strOther) const noexcept {
		return GetView() != strOther.GetView();
	}
	bool operator!=(const View &svOther) const noexcept {
		return GetView() != svOther;
	}
	friend bool operator!=(const View &svSelf, const String &strOther) noexcept {
		return svSelf != strOther.GetView();
	}

	bool operator<(const String &strOther) const noexcept {
		return GetView() < strOther.GetView();
	}
	bool operator<(const View &svOther) const noexcept {
		return GetView() < svOther;
	}
	friend bool operator<(const View &svSelf, const String &strOther) noexcept {
		return svSelf < strOther.GetView();
	}

	bool operator>(const String &strOther) const noexcept {
		return GetView() > strOther.GetView();
	}
	bool operator>(const View &svOther) const noexcept {
		return GetView() > svOther;
	}
	friend bool operator>(const View &svSelf, const String &strOther) noexcept {
		return svSelf > strOther.GetView();
	}

	bool operator<=(const String &strOther) const noexcept {
		return GetView() <= strOther.GetView();
	}
	bool operator<=(const View &svOther) const noexcept {
		return GetView() <= svOther;
	}
	friend bool operator<=(const View &svSelf, const String &strOther) noexcept {
		return svSelf <= strOther.GetView();
	}

	bool operator>=(const String &strOther) const noexcept {
		return GetView() >= strOther.GetView();
	}
	bool operator>=(const View &svOther) const noexcept {
		return GetView() >= svOther;
	}
	friend bool operator>=(const View &svSelf, const String &strOther) noexcept {
		return svSelf >= strOther.GetView();
	}

	friend void swap(String &svSelf, String &strOther) noexcept {
		svSelf.Swap(strOther);
	}

	friend decltype(auto) begin(const String &strOther) noexcept {
		return strOther.GetBegin();
	}
	friend decltype(auto) begin(String &strOther) noexcept {
		return strOther.GetBegin();
	}
	friend decltype(auto) cbegin(const String &strOther) noexcept {
		return begin(strOther);
	}
	friend decltype(auto) end(const String &strOther) noexcept {
		return strOther.GetEnd();
	}
	friend decltype(auto) end(String &strOther) noexcept {
		return strOther.GetEnd();
	}
	friend decltype(auto) cend(const String &strOther) noexcept {
		return end(strOther);
	}
};

namespace Impl_String {
	static_assert(sizeof(wchar_t) == sizeof(char16_t), "wchar_t does not have the same size with char16_t.");
	static_assert(alignof(wchar_t) == alignof(char16_t), "wchar_t does not have the same alignment with char16_t.");

	template<Impl_StringTraits::Type kSrcTypeT>
	struct Transcoder {
		template<Impl_StringTraits::Type kDstTypeT>
		void operator()(String<kDstTypeT> &strDst, const StringView<kSrcTypeT> &svSrc) const {
			constexpr auto kUnifiedStringType = (Impl_StringTraits::Encoding<kDstTypeT>::kConversionPreference + Impl_StringTraits::Encoding<kSrcTypeT>::kConversionPreference < 0) ? Impl_StringTraits::Type::kUtf16 : Impl_StringTraits::Type::kUtf32;
			String<kUnifiedStringType> usTemp;
			String<kSrcTypeT>::UnifyAppend(usTemp, svSrc);
			String<kDstTypeT>::DeunifyAppend(strDst, usTemp);
		}

		void operator()(String<Impl_StringTraits::Type::kWide> &wsDst, const StringView<kSrcTypeT> &svSrc) const {
			String<kSrcTypeT>::UnifyAppend(reinterpret_cast<String<Impl_StringTraits::Type::kUtf16> &>(wsDst), svSrc);
		}
		void operator()(String<Impl_StringTraits::Type::kUtf16> &u16sDst, const StringView<kSrcTypeT> &svSrc) const {
			String<kSrcTypeT>::UnifyAppend(u16sDst, svSrc);
		}
		void operator()(String<Impl_StringTraits::Type::kUtf32> &u32sDst, const StringView<kSrcTypeT> &svSrc) const {
			String<kSrcTypeT>::UnifyAppend(u32sDst, svSrc);
		}
	};

	template<>
	struct Transcoder<Impl_StringTraits::Type::kWide> {
		template<Impl_StringTraits::Type kDstTypeT>
		void operator()(String<kDstTypeT> &strDst, const StringView<Impl_StringTraits::Type::kWide> &wsvSrc) const {
			String<kDstTypeT>::DeunifyAppend(strDst, reinterpret_cast<const StringView<Impl_StringTraits::Type::kUtf16> &>(wsvSrc));
		}
	};
	template<>
	struct Transcoder<Impl_StringTraits::Type::kUtf16> {
		template<Impl_StringTraits::Type kDstTypeT>
		void operator()(String<kDstTypeT> &strDst, const StringView<Impl_StringTraits::Type::kUtf16> &u16svSrc) const {
			String<kDstTypeT>::DeunifyAppend(strDst, u16svSrc);
		}
	};
	template<>
	struct Transcoder<Impl_StringTraits::Type::kUtf32> {
		template<Impl_StringTraits::Type kDstTypeT>
		void operator()(String<kDstTypeT> &strDst, const StringView<Impl_StringTraits::Type::kUtf32> &u32svSrc) const {
			String<kDstTypeT>::DeunifyAppend(strDst, u32svSrc);
		}
	};
}

extern template class String<Impl_StringTraits::Type::kUtf8>;
extern template class String<Impl_StringTraits::Type::kUtf16>;
extern template class String<Impl_StringTraits::Type::kUtf32>;
extern template class String<Impl_StringTraits::Type::kCesu8>;
extern template class String<Impl_StringTraits::Type::kAnsi>;
extern template class String<Impl_StringTraits::Type::kModifiedUtf8>;
extern template class String<Impl_StringTraits::Type::kNarrow>;
extern template class String<Impl_StringTraits::Type::kWide>;

using Utf8String         = String<Impl_StringTraits::Type::kUtf8>;
using Utf16String        = String<Impl_StringTraits::Type::kUtf16>;
using Utf32String        = String<Impl_StringTraits::Type::kUtf32>;
using Cesu8String        = String<Impl_StringTraits::Type::kCesu8>;
using AnsiString         = String<Impl_StringTraits::Type::kAnsi>;
using ModifiedUtf8String = String<Impl_StringTraits::Type::kModifiedUtf8>;
using NarrowString       = String<Impl_StringTraits::Type::kNarrow>;
using WideString         = String<Impl_StringTraits::Type::kWide>;

// 字面量运算符。
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

}

#endif
