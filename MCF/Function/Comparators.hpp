// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_FUNCTION_COMPARATORS_HPP_
#define MCF_FUNCTION_COMPARATORS_HPP_

#include <cstdint>

namespace MCF {

#define DEFINE_COMPARATOR_(name_, op_)	\
	struct name_ {	\
		template<typename Tx, typename Ty>	\
		constexpr bool operator()(const Tx &x, const Ty &y) const {	\
			return x op_ y;	\
		}	\
		template<typename Tx, typename Ty>	\
		bool operator()(Tx *const &px, Ty *const &py) const {	\
			return reinterpret_cast<std::uintptr_t>(static_cast<const volatile void *>(px)) op_ reinterpret_cast<std::uintptr_t>(static_cast<const volatile void *>(py));	\
		}	\
	};

DEFINE_COMPARATOR_(Equal,        ==)
DEFINE_COMPARATOR_(Unequal,      !=)
DEFINE_COMPARATOR_(Less,         < )
DEFINE_COMPARATOR_(Greater,      > )
DEFINE_COMPARATOR_(LessEqual,    <=)
DEFINE_COMPARATOR_(GreaterEqual, >=)

#undef DEFINE_COMPARATOR_

}

#endif
