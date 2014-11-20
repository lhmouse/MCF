// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_UNIQUE_PTR_HPP_
#define MCF_CORE_UNIQUE_PTR_HPP_

#include "UniqueHandle.hpp"
#include "DefaultDeleter.hpp"
#include <type_traits>

namespace MCF {

template<class T>
using UniquePtr = UniqueHandle<DefaultDeleter<std::remove_cv_t<T>>>;

}

#endif
