// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef MCF_SERDES_HPP_
#define MCF_SERDES_HPP_

#include "_SerdesTraits.hpp"
#include "../Core/StreamBuffer.hpp"
#include <cstddef>

namespace MCF {

template<typename Object_t>
inline void Serialize(StreamBuffer &sbufStream, const Object_t &vObject){
	Impl::SerdesTrait<Object_t>()(sbufStream, vObject);
}
template<typename Object_t>
inline void Serialize(StreamBuffer &sbufStream, const Object_t *pObjects, std::size_t uCount){
	Impl::SerdesTrait<Object_t[]>()(sbufStream, pObjects, uCount);
}

template<typename Object_t>
inline void Deserialize(Object_t &vObject, StreamBuffer &sbufStream){
	Impl::SerdesTrait<Object_t>()(vObject, sbufStream);
}
template<typename Object_t>
inline void Deserialize(Object_t *pObjects, std::size_t uCount, StreamBuffer &sbufStream){
	Impl::SerdesTrait<Object_t[]>()(pObjects, uCount, sbufStream);
}

// 运算符重载。
template<typename Object_t>
inline StreamBuffer &operator<<(StreamBuffer &sbufStream, const Object_t &vObject){
	Serialize(sbufStream, vObject);
	return sbufStream;
}
template<typename Object_t, std::size_t COUNT>
inline StreamBuffer &operator<<(StreamBuffer &sbufStream, const Object_t (&aObjects)[COUNT]){
	Serialize(sbufStream, aObjects, COUNT);
	return sbufStream;
}

template<typename Object_t>
inline StreamBuffer &operator>>(StreamBuffer &sbufStream, Object_t &vObject){
	Deserialize(vObject, sbufStream);
	return sbufStream;
}
template<typename Object_t, std::size_t COUNT>
inline StreamBuffer &operator>>(StreamBuffer &sbufStream, Object_t (&aObjects)[COUNT]){
	Deserialize(aObjects, COUNT, sbufStream);
	return sbufStream;
}

}

#endif
