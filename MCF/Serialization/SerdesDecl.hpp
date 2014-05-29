// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef MCF_SERDES_DECL_HPP_
#define MCF_SERDES_DECL_HPP_

namespace MCF {

namespace Impl {
	template<typename Object_t, typename = void>
	struct SerdesTrait;
}

}

#define SERDES_FRIEND_DECL(...)	\
	friend class ::MCF::Impl::SerdesTrait<__VA_ARGS__>;

#endif
