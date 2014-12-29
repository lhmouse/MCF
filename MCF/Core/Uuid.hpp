// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_UUID_HPP_
#define MCF_CORE_UUID_HPP_

#include "../Utilities/Assert.hpp"
#include "../Utilities/BinaryOperations.hpp"
#include <cstddef>
#include <cstdint>

namespace MCF {

class Uuid {
public:
	static Uuid Generate();

private:
	union {
		unsigned char aby[16];
		std::uint16_t au16[8];
		std::uint32_t au32[4];
		std::uint64_t au64[2];
	} xm_unData;

public:
	// 不初始化。
	explicit Uuid(std::nullptr_t) noexcept {
	}
	// 填零。
	constexpr Uuid() noexcept
		: xm_unData()
	{
	}
	// 填指定字节。
	explicit Uuid(unsigned char (&abyData)[16]) noexcept {
		BCopy(xm_unData, abyData);
	}
	// 根据字符串创建。字符串无效抛出异常。
	explicit Uuid(const char (&pszString)[37]);

public:
	const unsigned char *GetBegin() const noexcept {
		return xm_unData.aby;
	}
	unsigned char *GetBegin() noexcept {
		return xm_unData.aby;
	}
	const unsigned char *GetEnd() const noexcept {
		return xm_unData.aby + 16;
	}
	unsigned char *GetEnd() noexcept {
		return xm_unData.aby + 16;
	}
	constexpr std::size_t GetSize() const noexcept {
		return sizeof(xm_unData);
	}

	auto GetBytes() const noexcept -> const unsigned char (&)[16] {
		return xm_unData.aby;
	}
	auto GetBytes() noexcept -> unsigned char (&)[16] {
		return xm_unData.aby;
	}

	void Swap(Uuid &rhs) noexcept {
		BSwap(xm_unData, rhs.xm_unData);
	}

	void Print(char (&pszString)[37], bool bUpperCase = true) const noexcept;
	bool Scan(const char (&pszString)[37]) noexcept;

public:
	const unsigned char &operator[](std::size_t uIndex) const noexcept {
		ASSERT(uIndex <= GetSize());
		return GetBytes()[uIndex];
	}
	unsigned char &operator[](std::size_t uIndex) noexcept {
		ASSERT(uIndex <= GetSize());
		return GetBytes()[uIndex];
	}
};

inline bool operator==(const Uuid &lhs, const Uuid &rhs) noexcept {
	return BComp(lhs.GetBytes(), rhs.GetBytes()) == 0;
}
inline bool operator!=(const Uuid &lhs, const Uuid &rhs) noexcept {
	return BComp(lhs.GetBytes(), rhs.GetBytes()) != 0;
}
inline bool operator<(const Uuid &lhs, const Uuid &rhs) noexcept {
	return BComp(lhs.GetBytes(), rhs.GetBytes()) < 0;
}
inline bool operator>(const Uuid &lhs, const Uuid &rhs) noexcept {
	return BComp(lhs.GetBytes(), rhs.GetBytes()) > 0;
}
inline bool operator<=(const Uuid &lhs, const Uuid &rhs) noexcept {
	return BComp(lhs.GetBytes(), rhs.GetBytes()) <= 0;
}
inline bool operator>=(const Uuid &lhs, const Uuid &rhs) noexcept {
	return BComp(lhs.GetBytes(), rhs.GetBytes()) >= 0;
}

inline void swap(Uuid &lhs, Uuid &rhs) noexcept {
	lhs.Swap(rhs);
}

}

#endif
