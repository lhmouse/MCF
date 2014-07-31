// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_SERDES_UNORDERED_MAP_HPP_
#define MCF_SERDES_UNORDERED_MAP_HPP_

#include "Serdes.hpp"
#include <unordered_map>
#include <iterator>

namespace MCF {

template<class Key, class Value, class Hash, class Equal, class Allocator>
void operator>>=(const std::unordered_map<Key, Value, Hash, Equal, Allocator> &vSource, StreamBuffer &sbufSink){
	const auto uSize = vSource.size();
	uSize >>= sbufSink;
	MakeSeqInserter<std::pair<Key, Value>>(vSource.begin(), uSize) >>= sbufSink;
}
template<class Key, class Value, class Hash, class Equal, class Allocator>
void operator<<=(std::unordered_map<Key, Value, Hash, Equal, Allocator> &vSink, StreamBuffer &sbufSource){
	std::size_t uSize;
	uSize <<= sbufSource;
	vSink.clear();
	MakeSeqExtractor<std::pair<Key, Value>>(std::inserter(vSink, vSink.end()), uSize) <<= sbufSource;
}

template<class Key, class Value, class Hash, class Equal, class Allocator>
void operator>>=(const std::unordered_multimap<Key, Value, Hash, Equal, Allocator> &vSource, StreamBuffer &sbufSink){
	const auto uSize = vSource.size();
	uSize >>= sbufSink;
	MakeSeqInserter<std::pair<Key, Value>>(vSource.begin(), uSize) >>= sbufSink;
}
template<class Key, class Value, class Hash, class Equal, class Allocator>
void operator<<=(std::unordered_multimap<Key, Value, Hash, Equal, Allocator> &vSink, StreamBuffer &sbufSource){
	std::size_t uSize;
	uSize <<= sbufSource;
	vSink.clear();
	MakeSeqExtractor<std::pair<Key, Value>>(std::inserter(vSink, vSink.end()), uSize) <<= sbufSource;
}

}

#endif
