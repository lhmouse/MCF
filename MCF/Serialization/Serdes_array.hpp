// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_SERDES_ARRAY_HPP_
#define MCF_SERDES_ARRAY_HPP_

#include "Serdes.hpp"
#include <array>

namespace MCF {

template<typename Element, std::size_t ARRAY_SIZE>
void Serialize(StreamBuffer &sbufSink, const std::array<Element, ARRAY_SIZE> &vSource){
	Serialize<Element>(sbufSink, vSource.begin(), ARRAY_SIZE);
}
template<typename Element, std::size_t ARRAY_SIZE>
void Deserialize(std::array<Element, ARRAY_SIZE> &vSink, StreamBuffer &sbufSource){
	Deserialize<Element>(vSink.begin(), ARRAY_SIZE, sbufSource);
}

}

#endif
