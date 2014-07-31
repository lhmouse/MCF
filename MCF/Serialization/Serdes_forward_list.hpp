// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_SERDES_FORWARD_LIST_HPP_
#define MCF_SERDES_FORWARD_LIST_HPP_

#include "Serdes.hpp"
#include <forward_list>
#include <iterator>

namespace MCF {

template<class Element, class Allocator>
void operator>>=(const std::forward_list<Element, Allocator> &vSource, StreamBuffer &sbufSink){
	const auto uSize = (std::size_t)std::distance(vSource.begin(), vSource.end());
	uSize >>= sbufSink;
	MakeSeqInserter<Element>(vSource.begin(), uSize) >>= sbufSink;
}
template<class Element, class Allocator>
void operator<<=(std::forward_list<Element, Allocator> &vSink, StreamBuffer &sbufSource){
	std::size_t uSize;
	uSize <<= sbufSource;
	vSink.clear();
	MakeSeqExtractor<Element>(std::front_inserter(vSink), uSize) <<= sbufSource;
	vSink.reverse();
}

}

#endif
