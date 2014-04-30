// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef MCF_VAR_INT_EX_HPP_
#define MCF_VAR_INT_EX_HPP_

#include <type_traits>
#include <cstddef>
#include <cstdint>
#include <climits>

namespace MCF {

template<typename Underlying_t, Underlying_t ORIGIN = 0>
class VarIntEx {
	static_assert(std::is_arithmetic<Underlying_t>::value, "Underlying_t must be an arithmetic type.");

	static_assert(CHAR_BIT == 8, "Not supported.");
	static_assert(sizeof(std::uintmax_t) * CHAR_BIT <= 64, "Not supported.");

private:
	typedef typename std::make_unsigned<Underlying_t>::type xUnsigned;

private:
	template<typename Test_t = Underlying_t>
	static xUnsigned xZigZagEncode(typename std::enable_if<std::is_signed<Underlying_t>::value, Test_t>::type nValue) noexcept {
		return ((xUnsigned)nValue << 1) ^ (xUnsigned)(nValue >> (sizeof(nValue) * CHAR_BIT - 1));
	}
	template<typename Test_t = xUnsigned>
	static Underlying_t xZigZagDecode(typename std::enable_if<std::is_signed<Underlying_t>::value, Test_t>::type uEncoded) noexcept {
		return (Underlying_t)((uEncoded >> 1) ^ -(uEncoded & 1));
	}

	template<typename Test_t = Underlying_t>
	static xUnsigned xZigZagEncode(typename std::enable_if<!std::is_signed<Underlying_t>::value, Test_t>::type uValue) noexcept {
		return uValue;
	}
	template<typename Test_t = xUnsigned>
	static Underlying_t xZigZagDecode(typename std::enable_if<!std::is_signed<Underlying_t>::value, Test_t>::type uEncoded) noexcept {
		return uEncoded;
	}

private:
	Underlying_t xm_vValue;

public:
	VarIntEx(Underlying_t vValue = Underlying_t()) noexcept
		: xm_vValue(vValue)
	{
	}
	VarIntEx &operator=(Underlying_t vValue) noexcept {
		xm_vValue = vValue;
		return *this;
	}

public:
	const Underlying_t &Get() const noexcept {
		return xm_vValue;
	}
	Underlying_t &Get() noexcept {
		return xm_vValue;
	}

	unsigned char *Serialize(unsigned char (&abyBuffer)[9]) const noexcept {
		unsigned char *pbyWrite = abyBuffer;
		auto uEncoded = xZigZagEncode(xm_vValue - ORIGIN);
		for(auto i = Min(8u, sizeof(uEncoded)); i; --i){
			unsigned char by = uEncoded & 0x7F;
			uEncoded >>= 7;
			if(uEncoded != 0){
				by |= 0x80;
			}
			*(pbyWrite++) = by;
			if(uEncoded == 0){
				goto jDone;
			}
		}
		*(pbyWrite++) = uEncoded & 0xFF;

	jDone:
		return pbyWrite;
	}
	const unsigned char *Unserialize(const unsigned char *pbyBegin, const unsigned char *pbyEnd) noexcept {
		auto pbyRead = pbyBegin;
		xUnsigned uEncoded = 0;
		for(std::size_t i = 0; i < Min(4, sizeof(uEncoded)); ++i){
			if(pbyRead == pbyEnd){
				return nullptr;
			}
			const unsigned char by = *(pbyRead++);
			uEncoded |= (std::uint32_t)(by & 0x7F) << (i * 7);
			if(!(by & 0x80)){
				goto jDone;
			}
		}
		for(auto i = Min(4u, sizeof(uEncoded)); i; --i){
			if(pbyRead == pbyEnd){
				return nullptr;
			}
			const unsigned char by = *(pbyRead++);
			uEncoded |= (std::uint64_t)(by & 0x7F) << (i * 7);
			if(!(by & 0x80)){
				goto jDone;
			}
		}
		if(pbyRead == pbyEnd){
			return nullptr;
		}
		uEncoded |= (std::uint64_t)*(pbyRead++) << 56;

	jDone:
		xm_vValue = xZigZagDecode(uEncoded) + ORIGIN;
		return pbyRead;
	}

public:
	operator const Underlying_t &() const noexcept {
		return Get();
	}
	operator Underlying_t &() noexcept {
		return Get();
	}
};

}

#endif
