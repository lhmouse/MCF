// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_SERDES_LIST_HPP_
#define MCF_SERDES_LIST_HPP_

#include "Serdes.hpp"
#include <list>
#include <iterator>

namespace MCF {

template<class Element, class Allocator>
void operator>>=(const std::list<Element, Allocator> &vSource, StreamBuffer &sbufSink){
	const auto uSize = vSource.size();
	uSize >>= sbufSink;
	MakeSeqInserter<Element>(vSource.begin(), uSize) >>= sbufSink;
}
template<class Element, class Allocator>
void operator<<=(std::list<Element, Allocator> &vSink, StreamBuffer &sbufSource){
	std::size_t uSize;
	uSize <<= sbufSource;
	vSink.clear();
	MakeSeqExtractor<Element>(std::back_inserter(vSink), uSize) <<= sbufSource;
}

}

#endif
