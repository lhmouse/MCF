// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_VAR_INT_EX_HPP_
#define MCF_VAR_INT_EX_HPP_

#include <type_traits>
#include <limits>
#include <cstddef>
#include <cstdint>

namespace MCF {

namespace Impl {
	template<typename Plain, typename Encoded = typename std::make_unsigned<Plain>::type>
	struct ZigZagger {
		typedef Encoded EncodedType;

		EncodedType Encode(Plain vVal) const noexcept {
			const auto vTemp = static_cast<EncodedType>(vVal);
			return (vTemp << 1) ^ (vTemp >> std::numeric_limits<Plain>::digits);
		}
		Plain Decode(EncodedType vVal) const noexcept {
			return (Plain)((vVal >> 1) ^ -(vVal & 1));
		}
	};

	template<typename Plain>
	struct ZigZagger<Plain, Plain> {
		typedef Plain EncodedType;

		EncodedType Encode(Plain vVal) const noexcept {
			return vVal;
		}
		Plain Decode(EncodedType vVal) const noexcept {
			return vVal;
		}
	};
}

template<typename Underlying, Underlying ORIGIN = 0>
class VarIntEx {
	static_assert(std::is_arithmetic<Underlying>::value, "Underlying must be an arithmetic type.");

	static_assert(std::numeric_limits<unsigned char>::digits == 8, "Not supported.");
	static_assert(sizeof(std::uintmax_t) <= 8, "Not supported.");

public:
	enum : std::size_t {
		MAX_SERIALIZED_SIZE = sizeof(Underlying) + 1
	};

private:
	Underlying xm_vValue;

public:
	VarIntEx(Underlying vValue = Underlying()) noexcept
		: xm_vValue(vValue)
	{
	}
	VarIntEx &operator=(Underlying vValue) noexcept {
		xm_vValue = vValue;
		return *this;
	}

public:
	Underlying Get() const noexcept {
		return xm_vValue;
	}
	void Set(Underlying vNewValue) noexcept {
		xm_vValue = vNewValue;
	}

	// 最多输出 9 个字节。
	template<typename OutputIterator>
	void Serialize(OutputIterator &itWrite) const {
		auto uEncoded = Impl::ZigZagger<Underlying>().Encode(xm_vValue - ORIGIN);
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
	template<typename InputIterator>
	bool Deserialize(InputIterator &itRead, const typename std::common_type<InputIterator>::type &itEnd){
		typename Impl::ZigZagger<Underlying>::EncodedType uEncoded = 0;
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
		xm_vValue = Impl::ZigZagger<Underlying>().Decode(uEncoded) + ORIGIN;
		return true;
	}

public:
	operator const Underlying &() const noexcept {
		return Get();
	}
	operator Underlying &() noexcept {
		return Get();
	}
};

}

#endif
