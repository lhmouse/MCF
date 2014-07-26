// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014 LH_Mouse. All wrongs reserved.

#ifndef MCF_VAR_INT_EX_HPP_
#define MCF_VAR_INT_EX_HPP_

#include <type_traits>
#include <cstddef>
#include <cstdint>

namespace MCF {

namespace Impl {
	template<typename Plain_t, typename Encoded_t = typename std::make_unsigned<Plain_t>::type>
	struct ZigZagger {
		typedef Encoded_t EncodedType;

		EncodedType Encode(Plain_t vVal) const noexcept {
			return ((EncodedType)vVal << 1) ^ (EncodedType)(vVal >> (sizeof(vVal) * __CHAR_BIT__ - 1));
		}
		Plain_t Decode(EncodedType vVal) const noexcept {
			return (Plain_t)((vVal >> 1) ^ -(vVal & 1));
		}
	};

	template<typename Plain_t>
	struct ZigZagger<Plain_t, Plain_t> {
		typedef Plain_t EncodedType;

		EncodedType Encode(Plain_t vVal) const noexcept {
			return vVal;
		}
		Plain_t Decode(EncodedType vVal) const noexcept {
			return vVal;
		}
	};
}

template<typename Underlying_t, Underlying_t ORIGIN = 0>
class VarIntEx {
	static_assert(std::is_arithmetic<Underlying_t>::value, "Underlying_t must be an arithmetic type.");

	static_assert(__CHAR_BIT__ == 8, "Not supported.");
	static_assert(sizeof(std::uintmax_t) * __CHAR_BIT__ <= 64, "Not supported.");

public:
	enum : std::size_t {
		MAX_SERIALIZED_SIZE = sizeof(Underlying_t) + 1
	};

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
		auto uEncoded = Impl::ZigZagger<Underlying_t>().Encode(xm_vValue - ORIGIN);
		for(std::size_t i = 0; i < sizeof(uEncoded); ++i){
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
	bool Deserialize(InputIterator_t &itRead, const typename std::common_type<InputIterator_t>::type &itEnd){
		typename Impl::ZigZagger<Underlying_t>::EncodedType uEncoded = 0;
		for(std::size_t i = 0; i < 4; ++i){
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
		for(std::size_t i = 4; i < 8; ++i){
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
		xm_vValue = Impl::ZigZagger<Underlying_t>().Decode(uEncoded) + ORIGIN;
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
