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

		static EncodedType Encode(Plain vVal) noexcept {
			auto vTemp = static_cast<Encoded>(vVal);
			vTemp <<= 1;
			vTemp ^= static_cast<Encoded>(vVal >> std::numeric_limits<Plain>::digits);
			return vTemp;
		}
		static Plain Decode(EncodedType vVal) noexcept {
			auto vTemp = vVal;
			vTemp >>= 1;
			vTemp ^= static_cast<Encoded>(-(vVal & 1));
			return static_cast<Plain>(vTemp);
		}
	};

	template<typename Plain>
	struct ZigZagger<Plain, Plain> {
		typedef Plain EncodedType;

		static EncodedType Encode(Plain vVal) noexcept {
			return vVal;
		}
		static Plain Decode(EncodedType vVal) noexcept {
			return vVal;
		}
	};
}

template<typename Underlying, Underlying ORIGIN = 0>
class VarIntEx {
	static_assert(std::is_arithmetic<Underlying>::value, "Underlying must be an arithmetic type.");

	static_assert(std::numeric_limits<unsigned char>::digits == 8, "Not supported.");
	static_assert(sizeof(std::uintmax_t) <= 8, "Not supported.");

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
	void Serialize(OutputIterator &itWrite) const
		noexcept(noexcept(*itWrite = std::declval<unsigned char>(), ++itWrite))
	{
		auto uEncoded = Impl::ZigZagger<Underlying>::Encode(xm_vValue - ORIGIN);
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
	bool Deserialize(InputIterator &itRead, std::size_t uCount){
		std::uint64_t u64Encoded = 0;
		for(std::size_t i = 0; i < 4; ++i){
			if(uCount == 0){
				return false;
			}
			const unsigned char by = *itRead;
			++itRead;
			--uCount;
			u64Encoded |= (std::uint32_t)(by & 0x7F) << (i * 7);
			if(!(by & 0x80)){
				goto jDone;
			}
		}
		for(std::size_t i = 4; i < 8; ++i){
			if(uCount == 0){
				return false;
			}
			const unsigned char by = *itRead;
			++itRead;
			--uCount;
			u64Encoded |= (std::uint64_t)(by & 0x7F) << (i * 7);
			if(!(by & 0x80)){
				goto jDone;
			}
		}
		if(uCount == 0){
			return false;
		}
		u64Encoded |= (std::uint64_t)*itRead << 56;
		++itRead;

	jDone:
		static constexpr auto MY_MAX = std::numeric_limits<Underlying>::max();
		static constexpr auto MY_MIN = std::numeric_limits<Underlying>::min();

		const auto vDecoded = Impl::ZigZagger<
			typename std::conditional<
				std::is_signed<Underlying>::value,
				std::int64_t, std::uint64_t
				>::type
			>::Decode(u64Encoded) + ORIGIN;
		if((vDecoded < MY_MIN) || (MY_MAX < vDecoded)){
			MCF_THROW(ERROR_ARITHMETIC_OVERFLOW, L"整型溢出。"_wso);
		}
		xm_vValue = vDecoded;
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
