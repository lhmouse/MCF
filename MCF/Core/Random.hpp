// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014 LH_Mouse. All wrongs reserved.

#ifndef MCF_RANDOM_HPP_
#define MCF_RANDOM_HPP_

#include <type_traits>
#include <cstdint>
#include <cstddef>

namespace MCF {

extern std::uint32_t GenRandSeed() noexcept;

class Random {
private:
	// http://en.wikipedia.org/wiki/RC4
	unsigned char xm_abyBox[0x100];
	unsigned char xm_byI;
	unsigned char xm_byJ;

public:
	explicit Random(std::uint32_t u32Seed = GenRandSeed()) noexcept;

private:
	std::uint8_t xGetByte() noexcept;

public:
	template<typename T>
	T Get() noexcept {
		static_assert(std::is_integral<T>::value, "T must be an integral type.");

		typedef typename std::make_unsigned<T>::type U;

		U vTemp = 0;
		for(auto i = sizeof(U); i; --i){
			vTemp <<= 8;
			vTemp |= xGetByte();
		}
		return (T)vTemp;
	}
};

}

#endif
