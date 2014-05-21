// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef MCF_SERIALIZER_HPP_
#define MCF_SERIALIZER_HPP_

#include "DataBuffer.hpp"
#include "_SerdesTraits.hpp"
#include <cstddef>

namespace MCF {

class SerializerBase : ABSTRACT {
protected:
	virtual void xDoSerialize(DataBuffer &dbufStream, const void *pObject) const = 0;
};

template<typename Object_t>
class Serializer : CONCRETE(SerializerBase) {
public:
	typedef Object_t ObjectType;

public:
	static void Serialize(DataBuffer &dbufStream, const Object_t &vObject){
		Impl::SerdesTrait<Object_t>()(dbufStream, vObject);
	}

protected:
	virtual void xDoSerialize(DataBuffer &dbufStream, const void *pObject) const override {
		Serialize(dbufStream, *(const Object_t *)pObject);
	}

public:
	void operator()(DataBuffer &dbufStream, const Object_t &vObject){
		Serialize(dbufStream, vObject);
	}
};

template class Serializer<bool>;

template class Serializer<char>;
template class Serializer<wchar_t>;
template class Serializer<char16_t>;
template class Serializer<char32_t>;

template class Serializer<signed char>;
template class Serializer<unsigned char>;
template class Serializer<short>;
template class Serializer<unsigned short>;
template class Serializer<int>;
template class Serializer<unsigned int>;
template class Serializer<long>;
template class Serializer<unsigned long>;
template class Serializer<long long>;
template class Serializer<unsigned long long>;

template class Serializer<float>;
template class Serializer<double>;
template class Serializer<long double>;

template class Serializer<std::nullptr_t>;

// 小工具。
template<typename Object_t>
inline void Serialize(DataBuffer &dbufStream, const Object_t &vObject){
	Serializer<Object_t>()(dbufStream, vObject);
}

}

#endif
