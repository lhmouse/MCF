// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_SERDES_UNORDERED_SET_HPP_
#define MCF_SERDES_UNORDERED_SET_HPP_

#include "Serdes.hpp"
#include <unordered_set>
#include <iterator>

namespace MCF {

template<typename Key, class Compare, class Allocator>
void Serialize(StreamBuffer &sbufSink, const std::unordered_set<Key, Compare, Allocator> &vSource){
	const auto uSize = vSource.size();
	SerializeSize(sbufSink, uSize);
	Serialize<Key>(sbufSink, vSource.begin(), uSize);
}
template<typename Key, class Compare, class Allocator>
void Deserialize(std::unordered_set<Key, Compare, Allocator> &vSink, StreamBuffer &sbufSource){
	vSink.clear();
	const auto uSize = DeserializeSize(sbufSource);
	Deserialize<Key>(std::inserter(vSink, vSink.end()), uSize, sbufSource);
}

template<typename Key, class Compare, class Allocator>
void Serialize(StreamBuffer &sbufSink, const std::multiunordered_set<Key, Compare, Allocator> &vSource){
	const auto uSize = vSource.size();
	SerializeSize(sbufSink, uSize);
	Serialize<Key>(sbufSink, vSource.begin(), uSize);
}
template<typename Key, class Compare, class Allocator>
void Deserialize(std::multiunordered_set<Key, Compare, Allocator> &vSink, StreamBuffer &sbufSource){
	vSink.clear();
	const auto uSize = DeserializeSize(sbufSource);
	Deserialize<Key>(std::inserter(vSink, vSink.end()), uSize, sbufSource);
}

}

#endif
