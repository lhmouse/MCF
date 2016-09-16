// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_CONVERTERS_HPP_
#define MCF_CORE_CONVERTERS_HPP_

#include "../Config.hpp"
#include <utility>

namespace MCF {

template<typename DstT>
struct ImplicitConverter {
	template<typename SrcT>
	constexpr DstT operator()(SrcT &&vSrc) const noexcept {
		return std::forward<SrcT>(vSrc);
	}
};

template<typename DstT>
struct StaticCaster {
	template<typename SrcT>
	constexpr DstT operator()(SrcT &&vSrc) const noexcept {
		return static_cast<DstT>(std::forward<SrcT>(vSrc));
	}
};

template<typename DstT>
struct DynamicCaster {
	template<typename SrcT>
	DstT operator()(SrcT &&vSrc) const {
		return dynamic_cast<DstT>(std::forward<SrcT>(vSrc));
	}
};

template<typename DstT>
struct ConstCaster {
	template<typename SrcT>
	constexpr DstT operator()(SrcT &&vSrc) const noexcept {
		return const_cast<DstT>(std::forward<SrcT>(vSrc));
	}
};

template<typename DstT>
struct ReinterpretCaster {
	template<typename SrcT>
	DstT operator()(SrcT &&vSrc) const noexcept {
		return reinterpret_cast<DstT>(std::forward<SrcT>(vSrc));
	}
};

template<typename DstT>
struct CCaster {
	template<typename SrcT>
	constexpr DstT operator()(SrcT &&vSrc) const noexcept {
		return DstT(std::forward<SrcT>(vSrc));
	}
};

}

#endif
