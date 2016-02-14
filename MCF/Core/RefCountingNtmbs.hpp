// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_REF_COUNTING_NTMBS_HPP_
#define MCF_CORE_REF_COUNTING_NTMBS_HPP_

#include "../Thread/Atomic.hpp"
#include "../Utilities/ConstructDestruct.hpp"
#include <utility>
#include <new>
#include <cstring>
#include <cstddef>

namespace MCF {

class RefCountingNtmbs {
public:
	static RefCountingNtmbs Copy(const char *pszSrc){
		if(!pszSrc){
			return RefCountingNtmbs();
		}
		return Copy(pszSrc, std::strlen(pszSrc));
	}
	static RefCountingNtmbs Copy(const char *pszSrc, std::size_t uLen){
		if(uLen == 0){
			return RefCountingNtmbs();
		}
		const auto uSizeToAlloc = sizeof(Atomic<std::size_t>) + uLen + 1;
		if(uSizeToAlloc < uLen){
			throw std::bad_array_new_length();
		}
		const auto puRef = static_cast<Atomic<std::size_t> *>(::operator new[](uSizeToAlloc));
		Construct(puRef, 1u);
		const auto pszStr = static_cast<char *>(std::memcpy(puRef + 1, pszSrc, uLen));
		pszStr[uLen] = 0;
		return RefCountingNtmbs(1, puRef, pszStr);
	}
	static RefCountingNtmbs View(const char *pszSrc) noexcept {
		if(!pszSrc){
			return RefCountingNtmbs();
		}
		return RefCountingNtmbs(1, nullptr, pszSrc);
	}

private:
	Atomic<std::size_t> *x_puRef;
	const char *x_pszStr;

private:
	constexpr RefCountingNtmbs(int, Atomic<std::size_t> *puRef, const char *pszStr) noexcept
		: x_puRef(puRef), x_pszStr(pszStr)
	{
	}

	Atomic<std::size_t> *X_Fork() const noexcept {
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

public:
	constexpr RefCountingNtmbs(std::nullptr_t = nullptr) noexcept
		: x_puRef(nullptr), x_pszStr("")
	{
	}
	RefCountingNtmbs(const RefCountingNtmbs &rhs) noexcept
		: x_puRef(rhs.X_Fork()), x_pszStr(rhs.x_pszStr)
	{
	}
	RefCountingNtmbs(RefCountingNtmbs &&rhs) noexcept
		: x_puRef(std::exchange(rhs.x_puRef, nullptr)), x_pszStr(std::exchange(rhs.x_pszStr, ""))
	{
	}
	RefCountingNtmbs &operator=(const RefCountingNtmbs &rhs) noexcept {
		RefCountingNtmbs(rhs).Swap(*this);
		return *this;
	}
	RefCountingNtmbs &operator=(RefCountingNtmbs &&rhs) noexcept {
		rhs.Swap(*this);
		return *this;
	}
	~RefCountingNtmbs(){
		X_Dispose();
	}

public:
	bool IsEmpty() const noexcept {
		return x_pszStr[0] == 0;
	}
	const char *GetStr() const noexcept {
		return x_pszStr;
	}
	void Clear() noexcept {
		RefCountingNtmbs().Swap(*this);
	}

	void AssignCopy(const char *pszSrc){
		Copy(pszSrc).Swap(*this);
	}
	void AssignCopy(const char *pszSrc, std::size_t uLen){
		Copy(pszSrc, uLen).Swap(*this);
	}
	void AssignView(const char *pszSrc) noexcept {
		View(pszSrc).Swap(*this);
	}

	int Compare(const char *rhs) const noexcept {
		return std::strcmp(GetStr(), rhs);
	}

	void Swap(RefCountingNtmbs &rhs) noexcept {
		using std::swap;
		swap(x_puRef,  rhs.x_puRef);
		swap(x_pszStr, rhs.x_pszStr);
	}

public:
	explicit operator bool() const noexcept {
		return !IsEmpty();
	}
	operator const char *() const noexcept {
		return GetStr();
	}

	bool operator==(const RefCountingNtmbs &rhs) const noexcept {
		return std::strcmp(GetStr(), rhs.GetStr()) == 0;
	}
	bool operator==(const char *rhs) const noexcept {
		return std::strcmp(GetStr(), rhs) == 0;
	}
	friend bool operator==(const char *lhs, const RefCountingNtmbs &rhs) noexcept {
		return std::strcmp(lhs, rhs.GetStr()) == 0;
	}

	bool operator!=(const RefCountingNtmbs &rhs) const noexcept {
		return std::strcmp(GetStr(), rhs.GetStr()) != 0;
	}
	bool operator!=(const char *rhs) const noexcept {
		return std::strcmp(GetStr(), rhs) != 0;
	}
	friend bool operator!=(const char *lhs, const RefCountingNtmbs &rhs) noexcept {
		return std::strcmp(lhs, rhs.GetStr()) != 0;
	}

	bool operator<(const RefCountingNtmbs &rhs) const noexcept {
		return std::strcmp(GetStr(), rhs.GetStr()) < 0;
	}
	bool operator<(const char *rhs) const noexcept {
		return std::strcmp(GetStr(), rhs) < 0;
	}
	friend bool operator<(const char *lhs, const RefCountingNtmbs &rhs) noexcept {
		return std::strcmp(lhs, rhs.GetStr()) < 0;
	}

	bool operator>(const RefCountingNtmbs &rhs) const noexcept {
		return std::strcmp(GetStr(), rhs.GetStr()) > 0;
	}
	bool operator>(const char *rhs) const noexcept {
		return std::strcmp(GetStr(), rhs) > 0;
	}
	friend bool operator>(const char *lhs, const RefCountingNtmbs &rhs) noexcept {
		return std::strcmp(lhs, rhs.GetStr()) > 0;
	}

	bool operator<=(const RefCountingNtmbs &rhs) const noexcept {
		return std::strcmp(GetStr(), rhs.GetStr()) <= 0;
	}
	bool operator<=(const char *rhs) const noexcept {
		return std::strcmp(GetStr(), rhs) <= 0;
	}
	friend bool operator<=(const char *lhs, const RefCountingNtmbs &rhs) noexcept {
		return std::strcmp(lhs, rhs.GetStr()) <= 0;
	}

	bool operator>=(const RefCountingNtmbs &rhs) const noexcept {
		return std::strcmp(GetStr(), rhs.GetStr()) >= 0;
	}
	bool operator>=(const char *rhs) const noexcept {
		return std::strcmp(GetStr(), rhs) >= 0;
	}
	friend bool operator>=(const char *lhs, const RefCountingNtmbs &rhs) noexcept {
		return std::strcmp(lhs, rhs.GetStr()) >= 0;
	}

	friend void swap(RefCountingNtmbs &lhs, RefCountingNtmbs &rhs) noexcept {
		lhs.Swap(rhs);
	}
};

inline RefCountingNtmbs operator""_rcs(const char *pszStr, std::size_t /* uLen */) noexcept {
	return RefCountingNtmbs::View(pszStr);
}

}

#endif
