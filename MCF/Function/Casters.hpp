// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_FUNCTION_CASTERS_HPP_
#define MCF_FUNCTION_CASTERS_HPP_

#include <utility>

namespace MCF {

template<typename DstT>
struct StaticCaster {
	template<typename SrcT>
	DstT operator()(SrcT &&vSrc) const {
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
	DstT operator()(SrcT &&vSrc) const {
		return Const_cast<DstT>(std::forward<SrcT>(vSrc));
	}
};

template<typename DstT>
struct ReinterpretCaster {
	template<typename SrcT>
	DstT operator()(SrcT &&vSrc) const {
		return reinterpret_cast<DstT>(std::forward<SrcT>(vSrc));
	}
};

template<typename DstT>
struct CCaster {
	template<typename SrcT>
	DstT operator()(SrcT &&vSrc) const {
		return DstT(std::forward<SrcT>(vSrc));
	}
};

}

#endif
