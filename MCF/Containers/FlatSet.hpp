// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_CONTAINERS_FLAT_SET_HPP_
#define MCF_CONTAINERS_FLAT_SET_HPP_

#include "_FlatBst.hpp"

namespace MCF {

template<typename KeyT, typename ComparatorT = std::less<void>>
class FlatSet {
private:

};

template<typename KeyT, typename ComparatorT>
void swap(FlatSet<KeyT, ComparatorT> &lhs, FlatSet<KeyT, ComparatorT> &rhs) noexcept {
	lhs.Swap(rhs);
}

template<typename KeyT, typename ComparatorT>
decltype(auto) begin(const FlatSet<KeyT, ComparatorT> &rhs) noexcept {
	return rhs.EnumerateFirst();
}
template<typename KeyT, typename ComparatorT>
decltype(auto) begin(FlatSet<KeyT, ComparatorT> &rhs) noexcept {
	return rhs.EnumerateFirst();
}
template<typename KeyT, typename ComparatorT>
decltype(auto) cbegin(const FlatSet<KeyT, ComparatorT> &rhs) noexcept {
	return begin(rhs);
}
template<typename KeyT, typename ComparatorT>
decltype(auto) end(const FlatSet<KeyT, ComparatorT> &rhs) noexcept {
	return rhs.EnumerateSingular();
}
template<typename KeyT, typename ComparatorT>
decltype(auto) end(FlatSet<KeyT, ComparatorT> &rhs) noexcept {
	return rhs.EnumerateSingular();
}
template<typename KeyT, typename ComparatorT>
decltype(auto) cend(const FlatSet<KeyT, ComparatorT> &rhs) noexcept {
	return end(rhs);
}

}

#endif
