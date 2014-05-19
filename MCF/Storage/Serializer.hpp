// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef MCF_SERIALIZER_HPP_
#define MCF_SERIALIZER_HPP_

#include "../Core/Utilities.hpp"
#include "VarIntEx.hpp"
#include <utility>
#include <algorithm>
#include <iterator>
#include <type_traits>
#include <tuple>
#include <vector>
#include <cstddef>

namespace MCF {

struct SerializerDefinitions {
	typedef std::vector<unsigned char> DataContainer;
	typedef std::back_insert_iterator<DataContainer> WriteIterator;
	typedef DataContainer::const_iterator ReadIterator;
};

namespace Impl {
	class SerializerBase : ABSTRACT, public SerializerDefinitions {
	protected:
		[[noreturn]] void xBailOnEndOfStream() const;

	public:
		virtual void Serialize(WriteIterator &itOutput, const void *pInput) const = 0;
		virtual void Unserialize(void *pOutput, ReadIterator &itRead, const ReadIterator &itEnd) const = 0;
	};

	// 放置一个声明，免得多事。
	template<typename Object_t, typename = void>
	class Serializer;

	// 针对 bool 的特化。使用最小的存储。注意 bool 本身的大小是实现定义的，有可能当作 int 了。
	template<>
	class Serializer<bool> : CONCRETE(SerializerBase) {
	public:
		virtual void Serialize(WriteIterator &itOutput, const void *pInput) const override {
			*itOutput = *(const bool *)pInput;
			++itOutput;
		}
		virtual void Unserialize(void *pOutput, ReadIterator &itRead, const ReadIterator &itEnd) const override {
			if(itRead == itEnd){
				SerializerBase::xBailOnEndOfStream();
			}
			*(bool *)pOutput = *itRead;
			++itRead;
		}
	};

	// 针对数学类型（bool 除外）的特化。
	template<typename Arithmetic_t>
	class Serializer<Arithmetic_t, typename std::enable_if<std::is_arithmetic<Arithmetic_t>::value>::type> : CONCRETE(SerializerBase) {
	public:
		virtual void Serialize(WriteIterator &itOutput, const void *pInput) const override {
			itOutput = std::
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
				copy
#else
				reverse_copy
#endif
			((const unsigned char *)pInput, (const unsigned char *)pInput + sizeof(Arithmetic_t), itOutput);
		}
		virtual void Unserialize(void *pOutput, ReadIterator &itRead, const ReadIterator &itEnd) const override {
			if((std::size_t)(itEnd - itRead) < sizeof(Arithmetic_t)){
				SerializerBase::xBailOnEndOfStream();
			}
			auto itNextRead = itRead + sizeof(Arithmetic_t);
			std::
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
				copy
#else
				reverse_copy
#endif
			(itRead, itNextRead, (unsigned char *)pOutput);
			itRead = std::move(itNextRead);
		}
	};

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

	// 针对指针的特化。由于指针用于持久存储无任何意义，因此指针不能序列化。
	template<typename Object_t>
	class Serializer<Object_t *> : CONCRETE(SerializerBase) {
		static_assert(((Object_t *)1, false), "Pointers can't be serialized.");
	};

	// 针对指向成员的指针的特化。因为虚基类破事太多干脆整个禁用算了。
	template<typename Parent_t, typename Object_t>
	class Serializer<Object_t Parent_t::*> : CONCRETE(SerializerBase) {
		static_assert(((Object_t *)1, false), "Pointer-to-members can't be serialized.");
	};

	// 针对 nullptr_t 的特化。什么都不做。这个特化貌似只有理论上的意义。
	template<>
	class Serializer<std::nullptr_t> : CONCRETE(SerializerBase) {
	public:
		virtual void Serialize(WriteIterator &, const void *) const override {
		}
		virtual void Unserialize(void *, ReadIterator &, const ReadIterator &) const override {
		}
	};

	// 针对枚举类型的特化。当作整数对待。
	template<typename Enum_t>
	class Serializer<Enum_t, typename std::enable_if<std::is_enum<Enum_t>::value>::type> : CONCRETE(SerializerBase) {
	private:
		typedef typename std::underlying_type<Enum_t>::type xUnderlying;
		typedef Serializer<xUnderlying> xDelegate;

	public:
		virtual void Serialize(WriteIterator &itOutput, const void *pInput) const override {
			xDelegate vSerializer;
			// 虚函数的非虚调用。
			vSerializer.xDelegate::Serialize(itOutput, (const xUnderlying *)pInput);
		}
		virtual void Unserialize(void *pOutput, ReadIterator &itRead, const ReadIterator &itEnd) const override {
			xDelegate vSerializer;
			// 同上。
			vSerializer.xDelegate::Unserialize((xUnderlying *)pOutput, itRead, itEnd);
		}
	};

	// 针对内建数组的特化。从第一个开始迭代处理每一项即可。
	template<typename Object_t, std::size_t COUNT>
	class Serializer<Object_t [COUNT]> : CONCRETE(SerializerBase) {
	private:
		typedef Serializer<Object_t> xDelegate;

	public:
		virtual void Serialize(WriteIterator &itOutput, const void *pInput) const override {
			xDelegate vSerializer;
			for(const auto &vObj : *(const Object_t (*)[COUNT])pInput){
				vSerializer.xDelegate::Serialize(itOutput, &vObj);
			}
		}
		virtual void Unserialize(void *pOutput, ReadIterator &itRead, const ReadIterator &itEnd) const override {
			xDelegate vSerializer;
			for(auto &vObj : *(Object_t (*)[COUNT])pOutput){
				vSerializer.xDelegate::Unserialize(&vObj, itRead, itEnd);
			}
		}
	};

	// 针对 pair 的特化。
	template<typename First_t, typename Second_t>
	class Serializer<std::pair<First_t, Second_t>> : CONCRETE(SerializerBase) {
	private:
		typedef Serializer<First_t> xDelegateFirst;
		typedef Serializer<Second_t> xDelegateSecond;

	public:
		virtual void Serialize(WriteIterator &itOutput, const void *pInput) const override {
			xDelegateFirst vSerializerFirst;
			vSerializerFirst.xDelegateFirst::Serialize(itOutput, &(((const std::pair<First_t, Second_t> *)pInput)->first));
			xDelegateSecond vSerializerSecond;
			vSerializerSecond.xDelegateSecond::Serialize(itOutput, &(((const std::pair<First_t, Second_t> *)pInput)->second));
		}
		virtual void Unserialize(void *pOutput, ReadIterator &itRead, const ReadIterator &itEnd) const override {
			xDelegateFirst vSerializerFirst;
			vSerializerFirst.xDelegateFirst::Unserialize(&(((std::pair<First_t, Second_t> *)pOutput)->first), itRead, itEnd);
			xDelegateSecond vSerializerSecond;
			vSerializerSecond.xDelegateSecond::Unserialize(&(((std::pair<First_t, Second_t> *)pOutput)->second), itRead, itEnd);
		}
	};

	// 针对 tuple 的特化。
	template<typename... Elements_t>
	class Serializer<std::tuple<Elements_t...>> : CONCRETE(SerializerBase) {
	private:
		typedef std::tuple<Serializer<Elements_t>...> xDelegates;

	private:
		template<std::size_t INDEX, typename std::enable_if<(INDEX < sizeof...(Elements_t)), int>::type = 0>
		static void xSerializeAll(const xDelegates &vSerializers, WriteIterator &itOutput, const void *pInput){
			typedef typename std::tuple_element<INDEX, xDelegates>::type Serializer;

			std::get<INDEX>(vSerializers).Serializer::Serialize(
				itOutput, &std::get<INDEX>(*(const std::tuple<Elements_t...> *)pInput)
			);
			xSerializeAll<INDEX + 1>(vSerializers, itOutput, pInput);
		}
		template<std::size_t INDEX, typename std::enable_if<(INDEX == sizeof...(Elements_t)), int>::type = 0>
		static void xSerializeAll(const xDelegates &, WriteIterator &, const void *){
		}

		template<std::size_t INDEX, typename std::enable_if<(INDEX < sizeof...(Elements_t)), int>::type = 0>
		static void xUnserializeAll(const xDelegates &vSerializers, void *pOutput, ReadIterator &itRead, const ReadIterator &itEnd){
			typedef typename std::tuple_element<INDEX, xDelegates>::type Serializer;

			std::get<INDEX>(vSerializers).Serializer::Unserialize(
				&std::get<INDEX>(*(std::tuple<Elements_t...> *)pOutput), itRead, itEnd
			);
			xUnserializeAll<INDEX + 1>(vSerializers, pOutput, itRead, itEnd);
		}
		template<std::size_t INDEX, typename std::enable_if<(INDEX == sizeof...(Elements_t)), int>::type = 0>
		static void xUnserializeAll(const xDelegates &, void *, ReadIterator &, const ReadIterator &){
		}

	public:
		virtual void Serialize(WriteIterator &itOutput, const void *pInput) const override {
			xDelegates vSerializers;
			xSerializeAll<0>(vSerializers, itOutput, pInput);
		}
		virtual void Unserialize(void *pOutput, ReadIterator &itRead, const ReadIterator &itEnd) const override {
			xDelegates vSerializers;
			xUnserializeAll<0>(vSerializers, pOutput, itRead, itEnd);
		}
	};

	// 针对拥有 resize() 的容器的特化。
	template<typename Container_t>
	class Serializer<Container_t, decltype((void)std::declval<Container_t>().resize((std::size_t)0))> : CONCRETE(SerializerBase) {
	private:
		typedef Serializer<typename Container_t::value_type> xDelegate;

	public:
		virtual void Serialize(WriteIterator &itOutput, const void *pInput) const override {
			const auto &vContainer = *(const Container_t *)pInput;

			VarIntEx<std::uint64_t> viSize(vContainer.size());
			viSize.Serialize(itOutput);

			xDelegate vSerializer;
			for(const auto &vObj : vContainer){
				vSerializer.xDelegate::Serialize(itOutput, &vObj);
			}
		}
		virtual void Unserialize(void *pOutput, ReadIterator &itRead, const ReadIterator &itEnd) const override {
			auto &vContainer = *(Container_t *)pOutput;

			VarIntEx<std::uint64_t> viSize;
			if(!viSize.Unserialize(itRead, itEnd)){
				SerializerBase::xBailOnEndOfStream();
			}
			vContainer.resize(viSize.Get());

			xDelegate vSerializer;
			for(auto &vObj : vContainer){
				vSerializer.xDelegate::Unserialize(&vObj, itRead, itEnd);
			}
		}
	};

	// 自定义版本。
	template<typename Object_t>
	class Serializer<Object_t, int> : CONCRETE(SerializerBase) {
	private:
		void (*xm_pfnDoSerialize)(WriteIterator &, const Object_t &);
		void (*xm_pfnDoUnserialize)(Object_t &, ReadIterator &, const ReadIterator &);

	public:
		constexpr Serializer(
			void (*pfnDoSerialize)(WriteIterator &, const Object_t &),
			void (*pfnDoUnserialize)(Object_t &, ReadIterator &, const ReadIterator &)
		)
			: xm_pfnDoSerialize(pfnDoSerialize)
			, xm_pfnDoUnserialize(pfnDoUnserialize)
		{
		}

	public:
		virtual void Serialize(WriteIterator &itOutput, const void *pInput) const override {
			(*xm_pfnDoSerialize)(itOutput, *(const Object_t *)pInput);
		}
		virtual void Unserialize(void *pOutput, ReadIterator &itRead, const ReadIterator &itEnd) const override {
			(*xm_pfnDoUnserialize)(*(Object_t *)pOutput, itRead, itEnd);
		}
	};

	// 使用静态表的版本（缺省）。这个版本需要使用下面的宏定义成员序列化表。
	template<typename Object_t, typename>
	class Serializer : CONCRETE(SerializerBase) {
	private:
		typedef Object_t xObject;

	private:
		static const std::pair<char Object_t::*, std::unique_ptr<SerializerBase>> xSER_TABLE_TABLE[];

	public:
		virtual void Serialize(WriteIterator &itOutput, const void *pInput) const override {
			auto pMemberItem = xSER_TABLE_TABLE;
			for(;;){
				if(pMemberItem->first){
					pMemberItem->second->Serialize(itOutput, &(((const Object_t *)pInput)->*(pMemberItem->first)));
				} else if(pMemberItem->second){
					pMemberItem->second->Serialize(itOutput, pInput);
				} else {
					break;
				}
				++pMemberItem;
			}
		}
		virtual void Unserialize(void *pOutput, ReadIterator &itRead, const ReadIterator &itEnd) const override {
			auto pMemberItem = xSER_TABLE_TABLE;
			for(;;){
				if(pMemberItem->first){
					pMemberItem->second->Unserialize(&(((Object_t *)pOutput)->*(pMemberItem->first)), itRead, itEnd);
				} else if(pMemberItem->second){
					pMemberItem->second->Unserialize(pOutput, itRead, itEnd);
				} else {
					break;
				}
				++pMemberItem;
			}
		}
	};
}

template<typename Object_t>
class Serializer : public SerializerDefinitions {
private:
	typedef Impl::Serializer<Object_t> xDelegate;

public:
	void Serialize(WriteIterator &itOutput, const Object_t &vInput) const {
		xDelegate vSerializer;
		vSerializer.Serialize(itOutput, &vInput);
	}
	void Unserialize(Object_t &vOutput, ReadIterator &itRead, const ReadIterator &itEnd) const {
		xDelegate vSerializer;
		vSerializer.Unserialize(&vOutput, itRead, itEnd);
	}
};

}

#define SER_TABLE_TABLE_BEGIN(...)	\
	template<>	\
	const std::pair<char __VA_ARGS__::*, std::unique_ptr<::MCF::Impl::SerializerBase>>	\
		(::MCF::Impl::Serializer<__VA_ARGS__>::xSER_TABLE_TABLE)[]	\
	= {

#define SER_TABLE_TABLE_SELF_CUSTOM(ser_proc, uns_proc)	\
		std::make_pair(	\
			nullptr,	\
			std::make_unique<::MCF::Impl::Serializer<xObject, int>>(ser_proc, uns_proc)	\
		),

#define SER_TABLE_TABLE_MEMBER_DEFAULT(member)	\
		std::make_pair(	\
			reinterpret_cast<char xObject::*>(&xObject::member),	\
			std::make_unique<::MCF::Impl::Serializer<decltype(xObject::member)>>()	\
		),

#define SER_TABLE_TABLE_MEMBER_CUSTOM(member, ser_proc, uns_proc)	\
		std::make_pair(	\
			reinterpret_cast<char xObject::*>(&xObject::member),	\
			std::make_unique<::MCF::Impl::Serializer<decltype(xObject::member), int>>(ser_proc, uns_proc)	\
		),

#define SER_TABLE_TABLE_END	\
		std::make_pair(nullptr, nullptr)	\
	};

#endif
