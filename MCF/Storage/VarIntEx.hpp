// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef MCF_VAR_INT_EX_HPP_
#define MCF_VAR_INT_EX_HPP_

#include "../Core/Utilities.hpp"
#include <type_traits>
#include <cstddef>
#include <cstdint>

namespace MCF {

template<typename Underlying_t, Underlying_t ORIGIN = 0>
class VarIntEx {
	static_assert(std::is_arithmetic<Underlying_t>::value, "Underlying_t must be an arithmetic type.");

	static_assert(__CHAR_BIT__ == 8u, "Not supported.");
	static_assert(sizeof(std::uintmax_t) * __CHAR_BIT__ <= 64u, "Not supported.");

public:
	enum : std::size_t {
		MAX_SERIALIZED_SIZE = 9u
	};

private:
	typedef typename std::make_unsigned<Underlying_t>::type xUnsigned;

private:
	template<typename Test_t = Underlying_t>
	static xUnsigned xZigZagEncode(typename std::enable_if<std::is_signed<Underlying_t>::value, Test_t>::type nValue) noexcept {
		return ((xUnsigned)nValue << 1) ^ (xUnsigned)(nValue >> (sizeof(nValue) * __CHAR_BIT__ - 1));
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

	// 最多输出 9 个字节。
	template<typename OutputIterator_t>
	void Serialize(OutputIterator_t &itWrite) const {
		auto uEncoded = xZigZagEncode(xm_vValue - ORIGIN);
		for(auto i = Min(8u, sizeof(uEncoded)); i; --i){
			unsigned char by = uEncoded & 0x7F;
			uEncoded >>= 7;
			if(uEncoded != 0){
				by |= 0x80;
			}
			*itWrite = by;
			++itWrite;
			if(uEncoded == 0){
				return;
			}
		}
		*itWrite = uEncoded & 0xFF;
		++itWrite;
	}
	template<typename InputIterator_t>
	bool Unserialize(InputIterator_t &itRead, const InputIterator_t &itEnd){
		xUnsigned uEncoded = 0;
		for(std::size_t i = 0; i < Min(4u, sizeof(uEncoded)); ++i){
			if(itRead == itEnd){
				return false;
			}
			const unsigned char by = *itRead;
			++itRead;
			uEncoded |= (std::uint32_t)(by & 0x7F) << (i * 7);
			if(!(by & 0x80)){
				goto jDone;
			}
		}
		for(auto i = Min(4u, sizeof(uEncoded)); i; --i){
			if(itRead == itEnd){
				return false;
			}
			const unsigned char by = *itRead;
			++itRead;
			uEncoded |= (std::uint64_t)(by & 0x7F) << (i * 7);
			if(!(by & 0x80)){
				goto jDone;
			}
		}
		if(itRead == itEnd){
			return false;
		}
		uEncoded |= (std::uint64_t)*itRead << 56;
		++itRead;

	jDone:
		xm_vValue = xZigZagDecode(uEncoded) + ORIGIN;
		return true;
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
