// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_RCNTS_HPP_
#define MCF_CORE_RCNTS_HPP_

#include "_CheckedSizeArithmetic.hpp"
#include "Atomic.hpp"
#include "ConstructDestruct.hpp"
#include <cstring>
#include <cstddef>
#include <cstdint>

namespace MCF {

template<typename CharT>
class Rcnts {
private:
	struct X_AdoptionTag { };

public:
	using Char = CharT;

private:
	static const CharT *X_GetNullTerminator() noexcept {
		static constexpr CharT kNull[1] = { };
		return kNull;
	}

private:
	static std::size_t X_Len(const Char *s) noexcept {
		std::size_t i = 0;
		while(s[i] != Char()){
			++i;
		}
		return i;
	}
	static int X_Cmp(const Char *s1, const Char *s2) noexcept {
		std::size_t i = 0;
		for(;;){
			const auto c1 = static_cast<std::int32_t>(static_cast<std::make_unsigned_t<Char>>(s1[i]));
			const auto c2 = static_cast<std::int32_t>(static_cast<std::make_unsigned_t<Char>>(s2[i]));
			const auto d = c1 - c2;
			if(d != 0){
				return static_cast<int>(d >> 31) | 1;
			}
			if(c1 == 0){
				return 0;
			}
			++i;
		}
	}

public:
	static Rcnts Copy(const Char *pszBegin){
		return Copy(pszBegin, X_Len(pszBegin));
	}
	static Rcnts Copy(const Char *pchBegin, std::size_t uLength){
		const auto uSizeToCopy = Impl_CheckedSizeArithmetic::Mul(sizeof(Char), uLength);
		const auto uSizeToAlloc = sizeof(Atomic<std::size_t>) + uSizeToCopy + sizeof(Char);
		const auto puRef = static_cast<Atomic<std::size_t> *>(::operator new[](uSizeToAlloc));
		Construct(puRef, 1u);
		const auto pszStr = static_cast<Char *>(std::memcpy(puRef + 1, pchBegin, uSizeToCopy));
		pszStr[uLength] = Char();
		return Rcnts(X_AdoptionTag(), puRef, pszStr);
	}
	static Rcnts View(const Char *pszBegin) noexcept {
		return Rcnts(X_AdoptionTag(), nullptr, pszBegin);
	}

private:
	Atomic<std::size_t> *x_puRef;
	const Char *x_pszStr;

private:
	Atomic<std::size_t> *X_Fork() const noexcept {
		const auto puRef = x_puRef;
		if(puRef){
			puRef->Increment(kAtomicRelaxed);
		}
		return puRef;
	}

private:
	constexpr Rcnts(const X_AdoptionTag &, Atomic<std::size_t> *puRef, const Char *pszStr) noexcept
		: x_puRef(puRef), x_pszStr(pszStr)
	{
	}

public:
	constexpr Rcnts() noexcept
		: Rcnts(X_AdoptionTag(), nullptr, X_GetNullTerminator())
	{
	}
	Rcnts(const Rcnts &rhs) noexcept
		: Rcnts(X_AdoptionTag(), rhs.X_Fork(), rhs.x_pszStr)
	{
	}
	Rcnts(Rcnts &&rhs) noexcept
		: Rcnts(X_AdoptionTag(), std::exchange(rhs.x_puRef, nullptr), std::exchange(rhs.x_pszStr, X_GetNullTerminator()))
	{
	}
	Rcnts &operator=(const Rcnts &rhs) noexcept {
		Rcnts(rhs).Swap(*this);
		return *this;
	}
	Rcnts &operator=(Rcnts &&rhs) noexcept {
		Rcnts(std::move(rhs)).Swap(*this);
		return *this;
	}
	~Rcnts(){
		const auto puRef = x_puRef;
#ifndef NDEBUG
		__builtin_memset(&x_puRef,  0xAA, sizeof(x_puRef));
		__builtin_memset(&x_pszStr, 0xBB, sizeof(x_pszStr));
#endif
		if(puRef){
			if(puRef->Decrement(kAtomicRelaxed) == 0){
				Destruct(puRef);
				::operator delete[](puRef);
			}
		}
	}

public:
	bool IsEmpty() const noexcept {
		return x_pszStr[0] == Char();
	}
	const Char *GetStr() const noexcept {
		return x_pszStr;
	}
	void Clear() noexcept {
		Rcnts().Swap(*this);
	}

	Rcnts &AssignCopy(const Char *pszBegin){
		Copy(pszBegin).Swap(*this);
		return *this;
	}
	Rcnts &AssignCopy(const Char *pchBegin, std::size_t uLength){
		Copy(pchBegin, uLength).Swap(*this);
		return *this;
	}
	Rcnts &AssignView(const Char *pszBegin) noexcept {
		View(pszBegin).Swap(*this);
		return *this;
	}

	int Compare(const Char *rhs) const noexcept {
		return X_Cmp(GetStr(), rhs);
	}
	int Compare(const Rcnts &rhs) const noexcept {
		return X_Cmp(GetStr(), rhs.GetStr());
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

	bool operator==(const Rcnts &rhs) const noexcept {
		return X_Cmp(GetStr(), rhs.GetStr()) == 0;
	}
	bool operator==(const Char *rhs) const noexcept {
		return X_Cmp(GetStr(), rhs) == 0;
	}
	friend bool operator==(const Char *lhs, const Rcnts &rhs) noexcept {
		return X_Cmp(lhs, rhs.GetStr()) == 0;
	}

	bool operator!=(const Rcnts &rhs) const noexcept {
		return X_Cmp(GetStr(), rhs.GetStr()) != 0;
	}
	bool operator!=(const Char *rhs) const noexcept {
		return X_Cmp(GetStr(), rhs) != 0;
	}
	friend bool operator!=(const Char *lhs, const Rcnts &rhs) noexcept {
		return X_Cmp(lhs, rhs.GetStr()) != 0;
	}

	bool operator<(const Rcnts &rhs) const noexcept {
		return X_Cmp(GetStr(), rhs.GetStr()) < 0;
	}
	bool operator<(const Char *rhs) const noexcept {
		return X_Cmp(GetStr(), rhs) < 0;
	}
	friend bool operator<(const Char *lhs, const Rcnts &rhs) noexcept {
		return X_Cmp(lhs, rhs.GetStr()) < 0;
	}

	bool operator>(const Rcnts &rhs) const noexcept {
		return X_Cmp(GetStr(), rhs.GetStr()) > 0;
	}
	bool operator>(const Char *rhs) const noexcept {
		return X_Cmp(GetStr(), rhs) > 0;
	}
	friend bool operator>(const Char *lhs, const Rcnts &rhs) noexcept {
		return X_Cmp(lhs, rhs.GetStr()) > 0;
	}

	bool operator<=(const Rcnts &rhs) const noexcept {
		return X_Cmp(GetStr(), rhs.GetStr()) <= 0;
	}
	bool operator<=(const Char *rhs) const noexcept {
		return X_Cmp(GetStr(), rhs) <= 0;
	}
	friend bool operator<=(const Char *lhs, const Rcnts &rhs) noexcept {
		return X_Cmp(lhs, rhs.GetStr()) <= 0;
	}

	bool operator>=(const Rcnts &rhs) const noexcept {
		return X_Cmp(GetStr(), rhs.GetStr()) >= 0;
	}
	bool operator>=(const Char *rhs) const noexcept {
		return X_Cmp(GetStr(), rhs) >= 0;
	}
	friend bool operator>=(const Char *lhs, const Rcnts &rhs) noexcept {
		return X_Cmp(lhs, rhs.GetStr()) >= 0;
	}

	friend void swap(Rcnts &lhs, Rcnts &rhs) noexcept {
		lhs.Swap(rhs);
	}
};

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
