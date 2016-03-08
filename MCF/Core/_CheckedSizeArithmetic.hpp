// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_CHECKED_SIZE_ARITHMETIC_HPP_
#define MCF_CORE_CHECKED_SIZE_ARITHMETIC_HPP_

#include <new>
#include <cstddef>

namespace MCF {

namespace Impl_CheckedSizeArithmetic {
	inline std::size_t Add(std::size_t uLhs, std::size_t uRhs){
		const auto uRet = uLhs + uRhs;
		if(uRet < uLhs){
			throw std::bad_array_new_length();
		}
		return uRet;
	}
	inline std::size_t Mul(std::size_t uLhs, std::size_t uRhs){
		const auto uRet = uLhs * uRhs;
		if(uRet / uLhs != uRhs){
			throw std::bad_array_new_length();
		}
		return uRet;
	}
}

}

#endif
