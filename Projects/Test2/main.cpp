#include <MCF/StdMCF.hpp>
#include <MCF/Core/StringObserver.hpp>
using namespace MCF;

namespace MCF {

enum class StringEncoding {
	UTF8,
	ANSI,
	UTF16
};

template<typename Char_t, StringEncoding ENCODING>
class String;

typedef String<wchar_t, StringEncoding::UTF16> UnifiedString;

template<typename Char_t, StringEncoding ENCODING>
class String {
	static_assert(std::is_arithmetic<Char_t>::value, "Char_t must be an arithmetic type.");

	template<typename, StringEncoding>
	friend class String;

public:
	typedef StringObserver<Char_t> Observer;

	enum : std::size_t {
		NPOS = Observer::NPOS
	};

private:
	union xStorage {
		struct __attribute__((packed)) {
			Char_t achSmall[32 / sizeof(Char_t) - 2];
			Char_t chNull;
			typename std::make_unsigned<Char_t>::type uchSmallLength;
		};
		struct {
			Char_t *pchLargeBegin;
			Char_t *pchLargeEnd;
			Char_t *pchLargeEndOfStor;
		};
	} xm_vStorage;

public:
	constexpr String() noexcept
		: xm_vStorage{{{Char_t()}, Char_t(), 0}}
	{
	}
/*	explicit String(Char_t ch, std::size_t uCount = 1) : String() {
		Assign(ch, uCount);
	}
	explicit String(const Observer &obs) : String() {
		Assign(obs);
	}
	template<typename OtherChar_t, StringEncoding OTHER_ENCODING>
	explicit String(const String<OtherChar_t, OTHER_ENCODING> &rhs) : String() {
		Assign(rhs);
	}
	template<typename OtherChar_t, StringEncoding OTHER_ENCODING>
	explicit String(String<OtherChar_t, OTHER_ENCODING> &&rhs) : String() {
		Assign(std::move(rhs));
	}
	String(const String &rhs) : String() {
		Assign(rhs);
	}
	String(String &&rhs) noexcept : String() {
		Assign(std::move(rhs));
	}
	String &operator=(Char_t ch) noexcept {
		Assign(ch, 1);
		return *this;
	}
	String &operator=(const Observer &obs){
		Assign(obs);
		return *this;
	}
	template<typename OtherChar_t, StringEncoding OTHER_ENCODING>
	String &operator=(const String<OtherChar_t, OTHER_ENCODING> &rhs){
		Assign(rhs);
		return *this;
	}
	template<typename OtherChar_t, StringEncoding OTHER_ENCODING>
	String &operator=(String<OtherChar_t, OTHER_ENCODING> &&rhs){
		Assign(std::move(rhs));
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
	~String(){
		if(xm_vStorage.chNull != Char_t()){
			delete[] xm_vStorage.pchLargeBegin;
		}
	}
*/
private:

public:
	const Char_t *GetCStr() const noexcept {
		if(xm_vStorage.chNull == Char_t()){
			return &(xm_vStorage.achSmall[0]);
		} else {
			return xm_vStorage.pchLargeBegin;
		}
	}
	Char_t *GetCStr() noexcept {
		if(xm_vStorage.chNull == Char_t()){
			return &(xm_vStorage.achSmall[0]);
		} else {
			return xm_vStorage.pchLargeBegin;
		}
	}
	std::size_t GetLength() const noexcept {
		if(xm_vStorage.chNull == Char_t()){
			return xm_vStorage.uchSmallLength;
		} else {
			return xm_vStorage.uLargeLength;
		}
	}

	Observer GetObserver() const noexcept {
		return Observer(GetBegin(), GetEnd());
	}

public:
/*	void Reverse() noexcept {
		auto pchBegin = GetBegin();
		auto pchEnd = GetEnd();
		if(pchBegin != pchEnd){
			--pchEnd;
			while(pchBegin < pchEnd){
				std::swap(*pchBegin, *pchEnd);
				++pchBegin;
				--pchEnd;
			}
		}
	}
*/
public:
	operator Observer() const noexcept {
		return GetObserver();
	}
/*	explicit operator bool() const noexcept {
		return !IsEmpty();
	}
	explicit operator const Char_t *() const noexcept {
		return GetCStr();
	}
	explicit operator Char_t *() noexcept {
		return GetCStr();
	}
	const Char_t &operator[](std::size_t uIndex) const noexcept {
		ASSERT_MSG(uIndex <= GetLength(), L"索引越界。");

		return GetCStr()[uIndex];
	}
	Char_t &operator[](std::size_t uIndex) noexcept {
		ASSERT_MSG(uIndex <= GetLength(), L"索引越界。");

		return GetCStr()[uIndex];
	}

	String &operator+=(Char_t rhs){
		Append(rhs);
		return *this;
	}
	String &operator+=(const Observer &rhs){
		Append(rhs);
		return *this;
	}
	String &operator+=(const String &rhs){
		Append(rhs);
		return *this;
	}
	String &operator+=(String &&rhs){
		Append(std::move(rhs));
		return *this;
	}
	template<typename OtherChar_t, StringEncoding OTHER_ENCODING>
	String &operator+=(const String<OtherChar_t, OTHER_ENCODING> &rhs){
		Append(rhs);
		return *this;
	}
	template<typename OtherChar_t, StringEncoding OTHER_ENCODING>
	String &operator+=(String<OtherChar_t, OTHER_ENCODING> &&rhs){
		Append(std::move(rhs));
		return *this;
	}*/
};
/*
template<typename Char_t, StringEncoding ENCODING>
String<Char_t, ENCODING> operator+(const String<Char_t, ENCODING> &lhs, const String<Char_t, ENCODING> &rhs){
	String<Char_t, ENCODING> strRet;
	strRet.Reserve(lhs.GetLength() + rhs.GetLength());
	strRet.Assign(lhs);
	strRet.Append(rhs);
	return std::move(strRet);
}
template<typename Char_t, StringEncoding ENCODING>
String<Char_t, ENCODING> operator+(String<Char_t, ENCODING> &&lhs, const String<Char_t, ENCODING> &rhs){
	lhs.Append(rhs);
	return std::move(lhs);
}
template<typename Char_t, StringEncoding ENCODING>
String<Char_t, ENCODING> operator+(const String<Char_t, ENCODING> &lhs, String<Char_t, ENCODING> &&rhs){
	rhs.Unshift(lhs);
	return std::move(rhs);
}
template<typename Char_t, StringEncoding ENCODING>
String<Char_t, ENCODING> operator+(String<Char_t, ENCODING> &&lhs, String<Char_t, ENCODING> &&rhs){
	if(lhs.GetCapacity() >= rhs.GetCapacity()){
		lhs.Append(rhs);
		return std::move(lhs);
	} else {
		rhs.Unshift(lhs);
		return std::move(rhs);
	}
}
template<typename Char_t, StringEncoding ENCODING>
String<Char_t, ENCODING> operator+(const StringObserver<Char_t> &lhs, const String<Char_t, ENCODING> &rhs){
	String<Char_t, ENCODING> strRet;
	strRet.Reserve(lhs.GetLength() + rhs.GetLength());
	strRet.Assign(lhs);
	strRet.Append(rhs);
	return std::move(strRet);
}
template<typename Char_t, StringEncoding ENCODING>
String<Char_t, ENCODING> operator+(const StringObserver<Char_t> &lhs, String<Char_t, ENCODING> &&rhs){
	rhs.Unshift(lhs);
	return std::move(rhs);
}
template<typename Char_t, StringEncoding ENCODING>
String<Char_t, ENCODING> operator+(Char_t lhs, const String<Char_t, ENCODING> &rhs){
	String<Char_t, ENCODING> strRet;
	strRet.Reserve(1 + rhs.GetLength());
	strRet.Assign(lhs);
	strRet.Append(rhs);
	return std::move(strRet);
}
template<typename Char_t, StringEncoding ENCODING>
String<Char_t, ENCODING> operator+(Char_t lhs, String<Char_t, ENCODING> &&rhs){
	rhs.Unshift(lhs);
	return std::move(rhs);
}
template<typename Char_t, StringEncoding ENCODING>
String<Char_t, ENCODING> operator+(const String<Char_t, ENCODING> &lhs, const StringObserver<Char_t> &rhs){
	String<Char_t, ENCODING> strRet;
	strRet.Reserve(lhs.GetLength() + rhs.GetLength());
	strRet.Assign(lhs);
	strRet.Append(rhs);
	return std::move(strRet);
}
template<typename Char_t, StringEncoding ENCODING>
String<Char_t, ENCODING> operator+(String<Char_t, ENCODING> &&lhs, const StringObserver<Char_t> &rhs){
	lhs.Append(rhs);
	return std::move(lhs);
}
template<typename Char_t, StringEncoding ENCODING>
String<Char_t, ENCODING> operator+(const String<Char_t, ENCODING> &lhs, Char_t rhs){
	String<Char_t, ENCODING> strRet;
	strRet.Reserve(lhs.GetLength() + 1);
	strRet.Assign(lhs);
	strRet.Append(rhs);
	return std::move(strRet);
}
template<typename Char_t, StringEncoding ENCODING>
String<Char_t, ENCODING> operator+(Char_t lhs, String<Char_t, ENCODING> &&rhs){
	lhs.Append(rhs);
	return std::move(lhs);
}
*/

template<typename Char_t, StringEncoding ENCODING>
bool operator==(const StringObserver<Char_t> &lhs, const String<Char_t, ENCODING> &rhs) noexcept {
	return rhs == lhs;
}
template<typename Char_t, StringEncoding ENCODING>
bool operator!=(const StringObserver<Char_t> &lhs, const String<Char_t, ENCODING> &rhs) noexcept {
	return rhs != lhs;
}
template<typename Char_t, StringEncoding ENCODING>
bool operator<(const StringObserver<Char_t> &lhs, const String<Char_t, ENCODING> &rhs) noexcept {
	return rhs > lhs;
}
template<typename Char_t, StringEncoding ENCODING>
bool operator<=(const StringObserver<Char_t> &lhs, const String<Char_t, ENCODING> &rhs) noexcept {
	return rhs >= lhs;
}
template<typename Char_t, StringEncoding ENCODING>
bool operator>(const StringObserver<Char_t> &lhs, const String<Char_t, ENCODING> &rhs) noexcept {
	return rhs < lhs;
}
template<typename Char_t, StringEncoding ENCODING>
bool operator>=(const StringObserver<Char_t> &lhs, const String<Char_t, ENCODING> &rhs) noexcept {
	return rhs <= lhs;
}

template<typename Char_t, StringEncoding ENCODING>
const Char_t *begin(const String<Char_t, ENCODING> &str) noexcept {
	return str.GetBegin();
}
template<typename Char_t, StringEncoding ENCODING>
Char_t *begin(String<Char_t, ENCODING> &str) noexcept {
	return str.GetBegin();
}
template<typename Char_t, StringEncoding ENCODING>
const Char_t *cbegin(const String<Char_t, ENCODING> &str) noexcept {
	return str.GetBegin();
}

template<typename Char_t, StringEncoding ENCODING>
const Char_t *end(const String<Char_t, ENCODING> &str) noexcept {
	return str.GetEnd();
}
template<typename Char_t, StringEncoding ENCODING>
Char_t *end(String<Char_t, ENCODING> &str) noexcept {
	return str.GetEnd();
}
template<typename Char_t, StringEncoding ENCODING>
const Char_t *cend(const String<Char_t, ENCODING> &str) noexcept {
	return str.GetEnd();
}

template class String<char,		StringEncoding::UTF8>;
template class String<char,		StringEncoding::ANSI>;
template class String<wchar_t,	StringEncoding::UTF16>;

typedef String<char,	StringEncoding::UTF8>	Utf8String;
typedef String<char,	StringEncoding::ANSI>	AnsiString;
typedef String<wchar_t,	StringEncoding::UTF16>	Utf16String;

}

unsigned int MCFMain(){
	Utf8String s1, s2;

	std::printf("%d\n", s1 < s2);

	return 0;
}
