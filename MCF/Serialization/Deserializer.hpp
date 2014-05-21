// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef MCF_DESERIALIZER_HPP_
#define MCF_DESERIALIZER_HPP_

#include "DataBuffer.hpp"
#include "_SerdesTraits.hpp"
#include <cstddef>

namespace MCF {

class DeserializerBase : ABSTRACT {
protected:
	virtual void xDoDeserialize(void *pObject, DataBuffer &dbufStream) const = 0;
};

template<typename Object_t>
class Deserializer : CONCRETE(DeserializerBase) {
public:
	typedef Object_t ObjectType;

public:
	static void Deserialize(Object_t &vObject, DataBuffer &dbufStream){
		Impl::SerdesTrait<Object_t>()(vObject, dbufStream);
	}

protected:
	virtual void xDoDeserialize(void *pObject, DataBuffer &dbufStream) const override {
		Deserialize(*(Object_t *)pObject, dbufStream);
	}

public:
	void operator()(Object_t &vObject, DataBuffer &dbufStream) const {
		Deserialize(vObject, dbufStream);
	}
};

template class Deserializer<bool>;

template class Deserializer<char>;
template class Deserializer<wchar_t>;
template class Deserializer<char16_t>;
template class Deserializer<char32_t>;

template class Deserializer<signed char>;
template class Deserializer<unsigned char>;
template class Deserializer<short>;
template class Deserializer<unsigned short>;
template class Deserializer<int>;
template class Deserializer<unsigned int>;
template class Deserializer<long>;
template class Deserializer<unsigned long>;
template class Deserializer<long long>;
template class Deserializer<unsigned long long>;

template class Deserializer<float>;
template class Deserializer<double>;
template class Deserializer<long double>;

template class Deserializer<std::nullptr_t>;

// 小工具。
template<typename Object_t>
inline void Deserialize(Object_t &vObject, DataBuffer &dbufStream){
	Deserializer<Object_t>()(vObject, dbufStream);
}

}

#endif
