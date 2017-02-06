// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef MCF_FUNCTION_CONVERTERS_HPP_
#define MCF_FUNCTION_CONVERTERS_HPP_

#include <utility>

namespace MCF {

template<typename DstT>
struct ImplicitConverter {
	template<typename SrcT>
	constexpr DstT operator()(SrcT &&src) const noexcept {
		return std::forward<SrcT>(src);
	}
};

template<typename DstT>
struct StaticCaster {
	template<typename SrcT>
	constexpr DstT operator()(SrcT &&src) const noexcept {
		return static_cast<DstT>(std::forward<SrcT>(src));
	}
};

template<typename DstT>
struct DynamicCaster {
	template<typename SrcT>
	DstT operator()(SrcT &&src) const {
		return dynamic_cast<DstT>(std::forward<SrcT>(src));
	}
};

template<typename DstT>
struct ConstCaster {
	template<typename SrcT>
	constexpr DstT operator()(SrcT &&src) const noexcept {
		return const_cast<DstT>(std::forward<SrcT>(src));
	}
};

template<typename DstT>
struct ReinterpretCaster {
	template<typename SrcT>
	DstT operator()(SrcT &&src) const noexcept {
		return reinterpret_cast<DstT>(std::forward<SrcT>(src));
	}
};

template<typename DstT>
struct CCaster {
	template<typename SrcT>
	constexpr DstT operator()(SrcT &&src) const noexcept {
		return DstT(std::forward<SrcT>(src));
	}
};

}

#endif
