// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_SERDES_ARRAY_HPP_
#define MCF_SERDES_ARRAY_HPP_

#include "Serdes.hpp"
#include <array>

namespace MCF {

template<class Element, std::size_t SIZE>
void operator>>=(const std::array<Element, SIZE> &vSource, StreamBuffer &sbufSink){
	MakeSeqInserter<Element>(vSource.begin(), SIZE) >>= sbufSink;
}
template<class Element, std::size_t SIZE>
void operator<<=(std::array<Element, SIZE> &vSink, StreamBuffer &sbufSource){
	MakeSeqExtractor<Element>(vSink.begin(), SIZE) <<= sbufSource;
}

}

#endif
