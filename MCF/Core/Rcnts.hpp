// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_RCNTS_HPP_
#define MCF_CORE_RCNTS_HPP_

#include "_CheckedSizeArithmetic.hpp"
#include "../Thread/Atomic.hpp"
#include "../Utilities/ConstructDestruct.hpp"
#include <type_traits>
#include <utility>
#include <cstring>
#include <cstddef>

namespace MCF {

namespace Impl_Rcnts {
	template<typename CharT>
	std::size_t StrLen(const CharT *s) noexcept {
		const auto b = s;
		for(;;){
			if(*s == CharT()){
				return static_cast<std::size_t>(s - b);
			}
			++s;
		}
	}

	template<typename CharT>
	int StrCmp(const CharT *s1, const CharT *s2) noexcept {
		for(;;){
			const auto c1 = static_cast<std::int_fast32_t>(static_cast<std::make_unsigned_t<CharT>>(*s1));
			const auto c2 = static_cast<std::int_fast32_t>(static_cast<std::make_unsigned_t<CharT>>(*s2));
			const auto d = c1 - c2;
			if(d != 0){
				return (d >> (sizeof(d) * __CHAR_BIT__ - 1)) | 1;
			}
			if(c1 == 0){
				return 0;
			}
			++s1;
			++s2;
		}
	}
}

template<typename CharT>
class Rcnts {
public:
	using Char = CharT;

private:
	static const CharT xs_kEmpty[1];

public:
	static Rcnts Copy(const Char *pszSrc){
		return Copy(pszSrc, Impl_Rcnts::StrLen(pszSrc));
	}
	static Rcnts Copy(const Char *pchSrc, std::size_t uCount){
		const auto uSizeToCopy = Impl_CheckedSizeArithmetic::Mul(sizeof(Char), uCount);
		const auto uSizeToAlloc = Impl_CheckedSizeArithmetic::Add(sizeof(Atomic<std::size_t>) + sizeof(Char), uSizeToCopy);
		const auto puRef = static_cast<Atomic<std::size_t> *>(::operator new[](uSizeToAlloc));
		Construct(puRef, 1u);
		const auto pszStr = static_cast<Char *>(std::memcpy(puRef + 1, pchSrc, uSizeToCopy));
		pszStr[uCount] = Char();
		return Rcnts(puRef, pszStr);
	}
	static Rcnts View(const Char *pszSrc) noexcept {
		return Rcnts(nullptr, pszSrc);
	}

private:
	Atomic<std::size_t> *x_puRef;
	const Char *x_pszStr;

private:
	Atomic<std::size_t> *X_ForkRef() const noexcept {
		const auto puRef = x_puRef;
		if(puRef){
			puRef->Increment(kAtomicRelaxed);
		}
		return puRef;
	}
	void X_Dispose() noexcept {
		const auto puRef = x_puRef;
		if(puRef){
			if(puRef->Decrement(kAtomicRelaxed) == 0){
				Destruct(puRef);
				::operator delete[](puRef);
			}
		}
#ifndef NDEBUG
		x_puRef = (Atomic<std::size_t> *)(std::uintptr_t)0xDEADBEEFDEADBEEF;
#endif
	}

private:
	constexpr Rcnts(Atomic<std::size_t> *puRef, const Char *pszStr) noexcept
		: x_puRef(puRef), x_pszStr(pszStr)
	{
	}

public:
	constexpr Rcnts() noexcept
		: Rcnts(nullptr, xs_kEmpty)
	{
	}
	Rcnts(const Rcnts &rhs) noexcept
		: x_puRef(rhs.X_ForkRef()), x_pszStr(rhs.x_pszStr)
	{
	}
	Rcnts(Rcnts &&rhs) noexcept
		: x_puRef(std::exchange(rhs.x_puRef, nullptr)), x_pszStr(std::exchange(rhs.x_pszStr, xs_kEmpty))
	{
	}
	Rcnts &operator=(const Rcnts &rhs) noexcept {
		Rcnts(rhs).Swap(*this);
		return *this;
	}
	Rcnts &operator=(Rcnts &&rhs) noexcept {
		rhs.Swap(*this);
		return *this;
	}
	~Rcnts(){
		X_Dispose();
	}

public:
	bool IsEmpty() const noexcept {
		return x_pszStr[0] == 0;
	}
	const Char *GetStr() const noexcept {
		return x_pszStr;
	}
	void Clear() noexcept {
		Rcnts().Swap(*this);
	}

	void AssignCopy(const Char *pszSrc){
		*this = Copy(pszSrc);
	}
	void AssignCopy(const Char *pchSrc, std::size_t uCount){
		*this = Copy(pchSrc, uCount);
	}
	void AssignView(const Char *pszSrc) noexcept {
		*this = View(pszSrc);
	}

	int Compare(const Char *rhs) const noexcept {
		return Impl_Rcnts::StrCmp(GetStr(), rhs);
	}
	int Compare(const Rcnts &rhs) const noexcept {
		return Compare(rhs.GetStr());
	}

	void Swap(Rcnts &rhs) noexcept {
		using std::swap;
		swap(x_puRef,  rhs.x_puRef);
		swap(x_pszStr, rhs.x_pszStr);
	}

public:
	explicit operator bool() const noexcept {
		return !IsEmpty();
	}
	operator const Char *() const noexcept {
		return GetStr();
	}

	bool operator==(const Char *rhs) const noexcept {
		return Compare(rhs) == 0;
	}
	bool operator==(const Rcnts &rhs) const noexcept {
		return Compare(rhs) == 0;
	}
	friend bool operator==(const Char *lhs, const Rcnts &rhs) noexcept {
		return -rhs.Compare(rhs) == 0;
	}

	bool operator!=(const Char *rhs) const noexcept {
		return Compare(rhs) != 0;
	}
	bool operator!=(const Rcnts &rhs) const noexcept {
		return Compare(rhs) != 0;
	}
	friend bool operator!=(const Char *lhs, const Rcnts &rhs) noexcept {
		return -rhs.Compare(rhs) != 0;
	}

	bool operator<(const Char *rhs) const noexcept {
		return Compare(rhs) < 0;
	}
	bool operator<(const Rcnts &rhs) const noexcept {
		return Compare(rhs) < 0;
	}
	friend bool operator<(const Char *lhs, const Rcnts &rhs) noexcept {
		return -rhs.Compare(rhs) < 0;
	}

	bool operator>(const Char *rhs) const noexcept {
		return Compare(rhs) > 0;
	}
	bool operator>(const Rcnts &rhs) const noexcept {
		return Compare(rhs) > 0;
	}
	friend bool operator>(const Char *lhs, const Rcnts &rhs) noexcept {
		return -rhs.Compare(rhs) > 0;
	}

	bool operator<=(const Char *rhs) const noexcept {
		return Compare(rhs) <= 0;
	}
	bool operator<=(const Rcnts &rhs) const noexcept {
		return Compare(rhs) <= 0;
	}
	friend bool operator<=(const Char *lhs, const Rcnts &rhs) noexcept {
		return -rhs.Compare(rhs) <= 0;
	}

	bool operator>=(const Char *rhs) const noexcept {
		return Compare(rhs) >= 0;
	}
	bool operator>=(const Rcnts &rhs) const noexcept {
		return Compare(rhs) >= 0;
	}
	friend bool operator>=(const Char *lhs, const Rcnts &rhs) noexcept {
		return -rhs.Compare(rhs) >= 0;
	}

	friend void swap(Rcnts &lhs, Rcnts &rhs) noexcept {
		lhs.Swap(rhs);
	}
};

template<typename CharT>
const CharT Rcnts<CharT>::xs_kEmpty[1] = { };

extern template class Rcnts<char>;
extern template class Rcnts<wchar_t>;
extern template class Rcnts<char16_t>;
extern template class Rcnts<char32_t>;

using Rcntns   = Rcnts<char>;
using Rcntws   = Rcnts<wchar_t>;
using Rcntu8s  = Rcnts<char>;
using Rcntu16s = Rcnts<char16_t>;
using Rcntu32s = Rcnts<char32_t>;

}

#endif
