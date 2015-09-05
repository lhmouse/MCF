// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_UTILITIES_UUID_HPP_
#define MCF_UTILITIES_UUID_HPP_

#include "Assert.hpp"
#include "BinaryOperations.hpp"
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
	} $unData;

public:
	// 填零。
	constexpr Uuid(std::nullptr_t = nullptr) noexcept
		: $unData()
	{
	}
	// 填指定字节。
	explicit Uuid(unsigned char (&abyData)[16]) noexcept {
		BCopy($unData, abyData);
	}
	// 根据字符串创建。字符串无效抛出异常。
	explicit Uuid(const char (&pchString)[36]);

public:
	const unsigned char *GetBegin() const noexcept {
		return $unData.aby;
	}
	unsigned char *GetBegin() noexcept {
		return $unData.aby;
	}
	const unsigned char *GetEnd() const noexcept {
		return $unData.aby + 16;
	}
	unsigned char *GetEnd() noexcept {
		return $unData.aby + 16;
	}
	constexpr std::size_t GetSize() const noexcept {
		return sizeof($unData);
	}

	auto GetBytes() const noexcept -> const unsigned char (&)[16] {
		return $unData.aby;
	}
	auto GetBytes() noexcept -> unsigned char (&)[16] {
		return $unData.aby;
	}

	void Swap(Uuid &rhs) noexcept {
		BSwap($unData, rhs.$unData);
	}

	void Print(char (&pchString)[36], bool bUpperCase = true) const noexcept;
	bool Scan(const char (&pchString)[36]) noexcept;

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
