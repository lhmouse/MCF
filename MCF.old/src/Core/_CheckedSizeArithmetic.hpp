// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2018, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_CHECKED_SIZE_ARITHMETIC_HPP_
#define MCF_CORE_CHECKED_SIZE_ARITHMETIC_HPP_

#include <new>
#include <cstddef>

namespace MCF {

namespace Impl_CheckedSizeArithmetic {
	constexpr std::size_t Add(std::size_t uOpOne, std::size_t uOpTwo){
		if(uOpOne == 0){
			return uOpTwo;
		}
		if(uOpTwo > static_cast<std::size_t>(-1) - uOpOne){
			throw std::bad_array_new_length();
		}
		return uOpOne + uOpTwo;
	}
	constexpr std::size_t Mul(std::size_t uOpOne, std::size_t uOpTwo){
		if(uOpOne == 0){
			return 0;
		}
		if(uOpTwo > static_cast<std::size_t>(-1) / uOpOne){
			throw std::bad_array_new_length();
		}
		return uOpOne * uOpTwo;
	}
}

}

#endif
