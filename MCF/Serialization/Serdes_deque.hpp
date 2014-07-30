// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_SERDES_DEQUE_HPP_
#define MCF_SERDES_DEQUE_HPP_

#include "Serdes.hpp"
#include <deque>
#include <iterator>

namespace MCF {

template<typename Element, class Allocator>
void Serialize(StreamBuffer &sbufSink, const std::deque<Element, Allocator> &vSource){
	const auto uSize = vSource.size();
	SerializeSize(sbufSink, uSize);
	Serialize<Element>(sbufSink, vSource.begin(), uSize);
}
template<typename Element, class Allocator>
void Deserialize(std::deque<Element, Allocator> &vSink, StreamBuffer &sbufSource){
	vSink.clear();
	const auto uSize = DeserializeSize(sbufSource);
	Deserialize<Element>(std::back_inserter(vSink), uSize, sbufSource);
}

}

#endif
