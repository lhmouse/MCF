// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_ADDRESS_OF_HPP_
#define MCF_CORE_ADDRESS_OF_HPP_

#include "../Config.hpp"

namespace MCF {

template<typename T>
inline T *AddressOf(T &v) noexcept {
	return reinterpret_cast<T *>(const_cast<char *>(reinterpret_cast<const volatile char (&)[1]>(v)));
}

}

#endif
