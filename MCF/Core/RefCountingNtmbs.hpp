// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

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
		return Copy(pszSrc, std::strlen(pszSrc));
	}
	static RefCountingNtmbs Copy(const char *pszSrc, std::size_t uLen){
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
		return RefCountingNtmbs(1, nullptr, pszSrc ? pszSrc : "");
	}

private:
	Atomic<std::size_t> *x_puRef;
	const char *x_pszStr;

private:
	constexpr RefCountingNtmbs(int, Atomic<std::size_t> *puRef, const char *pszStr) noexcept
		: x_puRef(puRef), x_pszStr(pszStr)
	{
	}

public:
	constexpr RefCountingNtmbs(std::nullptr_t = nullptr) noexcept
		: x_puRef(nullptr), x_pszStr("")
	{
	}
	RefCountingNtmbs(const RefCountingNtmbs &rhs) noexcept
		: x_puRef(rhs.x_puRef), x_pszStr(rhs.x_pszStr)
	{
		if(x_puRef){
			x_puRef->Increment(kAtomicRelaxed);
		}
	}
	RefCountingNtmbs(RefCountingNtmbs &&rhs) noexcept
		: x_puRef(rhs.x_puRef), x_pszStr(rhs.x_pszStr)
	{
		rhs.x_puRef  = nullptr;
		rhs.x_pszStr = "";
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
		if(x_puRef && (x_puRef->Decrement(kAtomicRelaxed) == 0)){
			Destruct(x_puRef);
			::operator delete[](x_puRef);
		}
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

	void Swap(RefCountingNtmbs &rhs) noexcept {
		std::swap(x_puRef, rhs.x_puRef);
		std::swap(x_pszStr,   rhs.x_pszStr);
	}

public:
	explicit operator bool() const noexcept {
		return !IsEmpty();
	}
	operator const char *() const noexcept {
		return GetStr();
	}
};

inline bool operator==(const RefCountingNtmbs &lhs, const RefCountingNtmbs &rhs) noexcept {
	return std::strcmp(lhs.GetStr(), rhs.GetStr()) == 0;
}
inline bool operator==(const RefCountingNtmbs &lhs, const char *rhs) noexcept {
	return std::strcmp(lhs.GetStr(), rhs) == 0;
}
inline bool operator==(const char *lhs, const RefCountingNtmbs &rhs) noexcept {
	return std::strcmp(lhs, rhs.GetStr()) == 0;
}

inline bool operator!=(const RefCountingNtmbs &lhs, const RefCountingNtmbs &rhs) noexcept {
	return std::strcmp(lhs.GetStr(), rhs.GetStr()) != 0;
}
inline bool operator!=(const RefCountingNtmbs &lhs, const char *rhs) noexcept {
	return std::strcmp(lhs.GetStr(), rhs) != 0;
}
inline bool operator!=(const char *lhs, const RefCountingNtmbs &rhs) noexcept {
	return std::strcmp(lhs, rhs.GetStr()) != 0;
}

inline bool operator<(const RefCountingNtmbs &lhs, const RefCountingNtmbs &rhs) noexcept {
	return std::strcmp(lhs.GetStr(), rhs.GetStr()) < 0;
}
inline bool operator<(const RefCountingNtmbs &lhs, const char *rhs) noexcept {
	return std::strcmp(lhs.GetStr(), rhs) < 0;
}
inline bool operator<(const char *lhs, const RefCountingNtmbs &rhs) noexcept {
	return std::strcmp(lhs, rhs.GetStr()) < 0;
}

inline bool operator>(const RefCountingNtmbs &lhs, const RefCountingNtmbs &rhs) noexcept {
	return std::strcmp(lhs.GetStr(), rhs.GetStr()) > 0;
}
inline bool operator>(const RefCountingNtmbs &lhs, const char *rhs) noexcept {
	return std::strcmp(lhs.GetStr(), rhs) > 0;
}
inline bool operator>(const char *lhs, const RefCountingNtmbs &rhs) noexcept {
	return std::strcmp(lhs, rhs.GetStr()) > 0;
}

inline bool operator<=(const RefCountingNtmbs &lhs, const RefCountingNtmbs &rhs) noexcept {
	return std::strcmp(lhs.GetStr(), rhs.GetStr()) <= 0;
}
inline bool operator<=(const RefCountingNtmbs &lhs, const char *rhs) noexcept {
	return std::strcmp(lhs.GetStr(), rhs) <= 0;
}
inline bool operator<=(const char *lhs, const RefCountingNtmbs &rhs) noexcept {
	return std::strcmp(lhs, rhs.GetStr()) <= 0;
}

inline bool operator>=(const RefCountingNtmbs &lhs, const RefCountingNtmbs &rhs) noexcept {
	return std::strcmp(lhs.GetStr(), rhs.GetStr()) >= 0;
}
inline bool operator>=(const RefCountingNtmbs &lhs, const char *rhs) noexcept {
	return std::strcmp(lhs.GetStr(), rhs) >= 0;
}
inline bool operator>=(const char *lhs, const RefCountingNtmbs &rhs) noexcept {
	return std::strcmp(lhs, rhs.GetStr()) >= 0;
}

inline void swap(RefCountingNtmbs &lhs, RefCountingNtmbs &rhs) noexcept {
	lhs.Swap(rhs);
}

inline RefCountingNtmbs operator""_rcs(const char *pszStr, std::size_t /* uLen */) noexcept {
	return RefCountingNtmbs::View(pszStr);
}

}

#endif
