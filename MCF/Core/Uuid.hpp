// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_UUID_HPP_
#define MCF_CORE_UUID_HPP_

#include "Array.hpp"
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
	} x_unData;

public:
	// 填零。
	constexpr Uuid() noexcept
		: x_unData()
	{
	}
	// 填指定字节。
	explicit Uuid(const Array<unsigned char, 16> &abyData) noexcept {
		BCopy(x_unData, abyData);
	}
	// 根据字符串创建。字符串无效抛出异常。
	explicit Uuid(const Array<char, 36> &achHex);

public:
	const unsigned char *GetData() const noexcept {
		return x_unData.aby;
	}
	unsigned char *GetData() noexcept {
		return x_unData.aby;
	}
	static constexpr std::size_t GetSize() noexcept {
		return 16;
	}

	void Swap(Uuid &rhs) noexcept {
		using std::swap;
		swap(x_unData, rhs.x_unData);
	}

	void Print(Array<char, 36> &achHex, bool bUpperCase = true) const noexcept;
	bool Scan(const Array<char, 36> &achHex) noexcept;

public:
	const unsigned char &operator[](std::size_t uIndex) const noexcept {
		ASSERT(uIndex < GetSize());

		return GetData()[uIndex];
	}
	unsigned char &operator[](std::size_t uIndex) noexcept {
		ASSERT(uIndex < GetSize());

		return GetData()[uIndex];
	}
};

inline bool operator==(const Uuid &lhs, const Uuid &rhs) noexcept {
	return BComp(lhs.GetData(), rhs.GetData()) == 0;
}
inline bool operator!=(const Uuid &lhs, const Uuid &rhs) noexcept {
	return BComp(lhs.GetData(), rhs.GetData()) != 0;
}
inline bool operator<(const Uuid &lhs, const Uuid &rhs) noexcept {
	return BComp(lhs.GetData(), rhs.GetData()) < 0;
}
inline bool operator>(const Uuid &lhs, const Uuid &rhs) noexcept {
	return BComp(lhs.GetData(), rhs.GetData()) > 0;
}
inline bool operator<=(const Uuid &lhs, const Uuid &rhs) noexcept {
	return BComp(lhs.GetData(), rhs.GetData()) <= 0;
}
inline bool operator>=(const Uuid &lhs, const Uuid &rhs) noexcept {
	return BComp(lhs.GetData(), rhs.GetData()) >= 0;
}

inline void swap(Uuid &lhs, Uuid &rhs) noexcept {
	lhs.Swap(rhs);
}

}

#endif
