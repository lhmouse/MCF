// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_ASSERT_HPP_
#define MCF_CORE_ASSERT_HPP_

#include "../Config.hpp"

#define MCF_ASSERT(expr_)                  _MCFCRT_ASSERT(expr_)
#define MCF_ASSERT_MSG(expr_, msg_)        _MCFCRT_ASSERT_MSG(expr_, msg_)

#define MCF_DEBUG_CHECK(expr_)             _MCFCRT_DEBUG_CHECK(expr_)
#define MCF_DEBUG_CHECK_MSG(expr_, msg_)   _MCFCRT_DEBUG_CHECK_MSG(expr_, msg_)

#endif

#include <MCFCRT/ext/assert.h>
