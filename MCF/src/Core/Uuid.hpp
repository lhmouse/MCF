// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_UUID_HPP_
#define MCF_CORE_UUID_HPP_

#include "Array.hpp"
#include "Exception.hpp"
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
		Array<std::uint8_t, 16> aby;
		Array<std::uint16_t, 8> au16;
		Array<std::uint32_t, 4> au32;
		Array<std::uint64_t, 2> au64;
	} x_vData = { };

public:
	// 填零。
	constexpr Uuid() noexcept { }
	// 填指定字节。
	explicit Uuid(const Array<unsigned char, 16> &abyData) noexcept {
		BCopy(x_vData.aby, abyData);
	}
	// 根据字符串创建。字符串无效抛出异常。
	explicit Uuid(const Array<char, 36> &achHex);

public:
	const unsigned char *GetData() const noexcept {
		return x_vData.aby.GetData();
	}
	unsigned char *GetData() noexcept {
		return x_vData.aby.GetData();
	}
	static constexpr std::size_t GetSize() noexcept {
		return 16;
	}

	const unsigned char &Get(std::size_t uIndex) const {
		if(uIndex > GetSize()){
			MCF_THROW(Exception, ERROR_ACCESS_DENIED, Rcntws::View(L"Uuid: 下标越界。"));
		}
		return UncheckedGet(uIndex);
	}
	unsigned char &Get(std::size_t uIndex){
		if(uIndex > GetSize()){
			MCF_THROW(Exception, ERROR_ACCESS_DENIED, Rcntws::View(L"Uuid: 下标越界。"));
		}
		return UncheckedGet(uIndex);
	}
	const unsigned char &UncheckedGet(std::size_t uIndex) const noexcept {
		MCF_DEBUG_CHECK(uIndex <= GetSize());

		return x_vData.aby.UncheckedGet(uIndex);
	}
	unsigned char &UncheckedGet(std::size_t uIndex) noexcept {
		MCF_DEBUG_CHECK(uIndex <= GetSize());

		return x_vData.aby.UncheckedGet(uIndex);
	}

	int Compare(const Uuid &vOther) const noexcept {
		return BComp(GetData(), vOther.GetData());
	}

	void Swap(Uuid &vOther) noexcept {
		BSwap(x_vData.aby, vOther.x_vData.aby);
	}

	void Print(Array<char, 36> &achHex, bool bUpperCase = true) const noexcept;
	bool Scan(const Array<char, 36> &achHex) noexcept;

public:
	const unsigned char &operator[](std::size_t uIndex) const noexcept {
		return UncheckedGet(uIndex);
	}
	unsigned char &operator[](std::size_t uIndex) noexcept {
		return UncheckedGet(uIndex);
	}

	bool operator==(const Uuid &vOther) const noexcept {
		return Compare(vOther) == 0;
	}
	bool operator!=(const Uuid &vOther) const noexcept {
		return Compare(vOther) != 0;
	}
	bool operator<(const Uuid &vOther) const noexcept {
		return Compare(vOther) < 0;
	}
	bool operator>(const Uuid &vOther) const noexcept {
		return Compare(vOther) > 0;
	}
	bool operator<=(const Uuid &vOther) const noexcept {
		return Compare(vOther) <= 0;
	}
	bool operator>=(const Uuid &vOther) const noexcept {
		return Compare(vOther) >= 0;
	}

	friend void swap(Uuid &vSelf, Uuid &vOther) noexcept {
		vSelf.Swap(vOther);
	}
};

}

#endif
