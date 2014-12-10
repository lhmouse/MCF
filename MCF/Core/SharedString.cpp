// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "SharedString.hpp"
#include "../Utilities/ConstructDestruct.hpp"
#include "../Utilities/Algorithms.hpp"
using namespace MCF;

namespace {

struct Empty {
};

template<typename T>
struct IncrementalAlloc {
public:
	using value_type = T;

	template<typename U>
	using rebind = IncrementalAlloc<U>;

	void *&m_pIncData;
	const std::size_t m_uIncSize;

	IncrementalAlloc(void *&pIncData, std::size_t uIncSize) noexcept
		: m_pIncData(pIncData), m_uIncSize(uIncSize)
	{
	}
	template<typename U>
	IncrementalAlloc(const IncrementalAlloc<U> &rhs) noexcept
		: m_pIncData(rhs.m_pIncData), m_uIncSize(rhs.m_uIncSize)
	{
	}

	T *allocate(std::size_t n, const void * = nullptr){
		const auto uTotal = n * sizeof(T) + m_uIncSize;
		if((uTotal / sizeof(T) != n + m_uIncSize / sizeof(T)) || (uTotal < n * sizeof(T))){
			throw std::bad_alloc();
		}
		const auto p = static_cast<char *>(::operator new(uTotal));
		m_pIncData = p + uTotal - m_uIncSize;
		return reinterpret_cast<T *>(p);
	}
	void deallocate(T *p, std::size_t) noexcept {
		::operator delete(p);
	}
	constexpr std::size_t max_size() const noexcept {
		return static_cast<std::size_t>(-1);
	}

	template<typename U>
	bool operator==(const IncrementalAlloc<U> &) const noexcept {
		return true;
	}
	template<typename U>
	bool operator!=(const IncrementalAlloc<U> &) const noexcept {
		return false;
	}

	template<typename ...ArgsT>
	void construct(T *p, ArgsT &&...args){
		Construct(p, std::forward<ArgsT>(args)...);
	}
	void destroy(T *p) noexcept {
		Destruct(p);
	}
};

}

namespace MCF {

template<typename CharT>
std::shared_ptr<CharT> CreateSharedString(const CharT *pchSrc, std::size_t uLen, std::size_t uAdditional){
	const std::size_t uIncrement = sizeof(std::size_t) * 2 + sizeof(CharT) * (uLen + 1 + uAdditional);
	if(uIncrement / sizeof(CharT) != sizeof(std::size_t) * 2 / sizeof(CharT) + (uLen + 1 + uAdditional)){
		throw std::bad_alloc();
	}
	void *pIncData;
	auto pBuffer = std::allocate_shared<Empty>(IncrementalAlloc<Empty>(pIncData, uIncrement));
	const auto pchWrite = reinterpret_cast<CharT *>(static_cast<std::size_t *>(pIncData) + 2);
	reinterpret_cast<std::size_t *>(pchWrite)[-2] = uLen + uAdditional;
	reinterpret_cast<std::size_t *>(pchWrite)[-1] = uLen;
	CopyN(pchWrite, pchSrc, uLen).first[0] = CharT();
	return std::shared_ptr<CharT>(std::move(pBuffer), pchWrite);
}

template std::shared_ptr<char> CreateSharedString<char>(const char *, std::size_t, std::size_t);
template std::shared_ptr<wchar_t> CreateSharedString<wchar_t>(const wchar_t *, std::size_t, std::size_t);
template std::shared_ptr<char16_t> CreateSharedString<char16_t>(const char16_t *, std::size_t, std::size_t);
template std::shared_ptr<char32_t> CreateSharedString<char32_t>(const char32_t *, std::size_t, std::size_t);

}
