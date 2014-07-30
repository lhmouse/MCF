// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_SERDES_MAP_HPP_
#define MCF_SERDES_MAP_HPP_

#include "Serdes.hpp"
#include <map>
#include <iterator>

namespace MCF {

template<typename Key, typename Value, class Compare, class Allocator>
void Serialize(StreamBuffer &sbufSink, const std::map<Key, Value, Compare, Allocator> &vSource){
	const auto uSize = vSource.size();
	SerializeSize(sbufSink, uSize);
	Serialize<std::pair<Key, Value>>(sbufSink, vSource.begin(), uSize);
}
template<typename Key, typename Value, class Compare, class Allocator>
void Deserialize(std::map<Key, Value, Compare, Allocator> &vSink, StreamBuffer &sbufSource){
	vSink.clear();
	const auto uSize = DeserializeSize(sbufSource);
	Deserialize<std::pair<Key, Value>>(std::inserter(vSink, vSink.end()), uSize, sbufSource);
}

template<typename Key, typename Value, class Compare, class Allocator>
void Serialize(StreamBuffer &sbufSink, const std::multimap<Key, Value, Compare, Allocator> &vSource){
	const auto uSize = vSource.size();
	SerializeSize(sbufSink, uSize);
	Serialize<std::pair<Key, Value>>(sbufSink, vSource.begin(), uSize);
}
template<typename Key, typename Value, class Compare, class Allocator>
void Deserialize(std::multimap<Key, Value, Compare, Allocator> &vSink, StreamBuffer &sbufSource){
	vSink.clear();
	const auto uSize = DeserializeSize(sbufSource);
	Deserialize<std::pair<Key, Value>>(std::inserter(vSink, vSink.end()), uSize, sbufSource);
}

}

#endif
