// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_SERDES_FORWARD_LIST_HPP_
#define MCF_SERDES_FORWARD_LIST_HPP_

#include "Serdes.hpp"
#include <forward_list>
#include <iterator>

namespace MCF {

template<typename Element, class Allocator>
void Serialize(StreamBuffer &sbufSink, const std::forward_list<Element, Allocator> &vSource){
	const auto uSize = (std::size_t)std::distance(vSource.begin(), vSource.end());
	SerializeSize(sbufSink, uSize);
	Serialize<Element>(sbufSink, vSource.begin(), uSize);
}
template<typename Element, class Allocator>
void Deserialize(std::forward_list<Element, Allocator> &vSink, StreamBuffer &sbufSource){
	vSink.clear();
	const auto uSize = DeserializeSize(sbufSource);
	Deserialize<Element>(std::front_inserter(vSink), uSize, sbufSource);
	vSink.reverse();
}

}

#endif
