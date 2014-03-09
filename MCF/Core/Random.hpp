// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef __MCF_RANDOM_HPP__
#define __MCF_RANDOM_HPP__

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
	Random(std::uint32_t u32Seed = GenRandSeed()) noexcept;

private:
	std::uint8_t xGetByte() noexcept;

public:
	template<typename T>
	T Get() noexcept {
		typedef typename std::make_unsigned<T>::type U;
		U vTemp = 0;
		for(std::size_t i = 0; i < sizeof(U); ++i){
			vTemp <<= 8;
			vTemp |= xGetByte();
		}
		return (T)vTemp;
	}
};

}

#endif
