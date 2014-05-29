// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef MCF_SERDES_HPP_
#define MCF_SERDES_HPP_

#include "Serializer.hpp"
#include "Deserializer.hpp"

namespace MCF {

class SerdesBase : public SerializerBase, public DeserializerBase {
};

template<typename Object_t>
class Serdes : public Serializer<Object_t>, public Deserializer<Object_t> {
protected:
	virtual void xDoSerialize(StreamBuffer &sbufStream, const void *pObject) const override {
		Serializer<Object_t>()(sbufStream, *(const Object_t *)pObject);
	}
	virtual void xDoSerialize(StreamBuffer &sbufStream, const void *pObject, std::size_t uSize) const override {
		Serializer<Object_t>()(sbufStream, (const Object_t *)pObject, uSize);
	}

	virtual void xDoDeserialize(void *pObject, StreamBuffer &sbufStream) const override {
		Deserializer<Object_t>()(*(Object_t *)pObject, sbufStream);
	}
	virtual void xDoDeserialize(void *pObject, std::size_t uSize, StreamBuffer &sbufStream) const override {
		Deserializer<Object_t>()((Object_t *)pObject, uSize, sbufStream);
	}
};

template class Serdes<bool>;

template class Serdes<char>;
template class Serdes<wchar_t>;
template class Serdes<char16_t>;
template class Serdes<char32_t>;

template class Serdes<signed char>;
template class Serdes<unsigned char>;
template class Serdes<short>;
template class Serdes<unsigned short>;
template class Serdes<int>;
template class Serdes<unsigned int>;
template class Serdes<long>;
template class Serdes<unsigned long>;
template class Serdes<long long>;
template class Serdes<unsigned long long>;

template class Serdes<float>;
template class Serdes<double>;
template class Serdes<long double>;

template class Serdes<std::nullptr_t>;

}

#endif
