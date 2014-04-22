// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef MCF_CRT_OFFSET_OF_HPP_
#define MCF_CRT_OFFSET_OF_HPP_

#include <cstddef>

namespace MCF {

template<class Parent_t, class Member_t>
constexpr std::size_t OffsetOf(Member_t Parent_t::*mpMember) noexcept {
	return (std::uintptr_t)&(((const Parent_t *)(std::uintptr_t)1)->*mpMember) - (std::uintptr_t)1;
}

template<class Parent_t, class Member_t>
constexpr Parent_t *DownCast(Member_t Parent_t::*mpMember, Member_t *pParent) noexcept {
	return (Parent_t *)((std::uintptr_t)pParent - OffsetOf(mpMember));
}

}

#endif
