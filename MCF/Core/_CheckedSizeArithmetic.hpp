// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_CHECKED_SIZE_ARITHMETIC_HPP_
#define MCF_CORE_CHECKED_SIZE_ARITHMETIC_HPP_

#include <new>
#include <cstddef>

namespace MCF {

namespace Impl_CheckedSizeArithmetic {
	inline std::size_t Add(std::size_t lhs, std::size_t rhs){
		if(lhs == 0){
			return rhs;
		}
		if(rhs > static_cast<std::size_t>(-1) - lhs){
			throw std::bad_array_new_length();
		}
		return lhs + rhs;
	}
	inline std::size_t Mul(std::size_t lhs, std::size_t rhs){
		if(lhs == 0){
			return 0;
		}
		if(rhs > static_cast<std::size_t>(-1) / lhs){
			throw std::bad_array_new_length();
		}
		return lhs * rhs;
	}
}

}

#endif
