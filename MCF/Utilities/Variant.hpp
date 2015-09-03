// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_UTILITIES_VARIANT_HPP_
#define MCF_UTILITIES_VARIANT_HPP_

#include "../Utilities/ParameterPackManipulators.hpp"
#include "../SmartPointers/UniquePtr.hpp"
#include <utility>
#include <typeinfo>
#include <cstddef>

namespace MCF {

template<typename ...ElementsT>
class Variant {
public:
	enum : std::size_t {
		kEmpty = (std::size_t)-1
	};

private:
	class xActiveElementBase {
	public:
		virtual ~xActiveElementBase(){
		}

	public:
		virtual std::size_t GetIndex() const noexcept = 0;
		virtual const std::type_info *GetTypeInfo() const noexcept = 0;
		virtual void *GetAddress() noexcept = 0;
		virtual UniquePtr<xActiveElementBase> Clone() const = 0;
	};

	template<typename ElementT>
	class xActiveElement : public xActiveElementBase {
	private:
		ElementT x_vElement;

	public:
		template<typename ...ParamsT>
		explicit xActiveElement(ParamsT &&...vParams)
			: x_vElement(std::forward<ParamsT>(vParams)...)
		{
		}
		~xActiveElement() override {
		}

	public:
		std::size_t GetIndex() const noexcept override {
			return FindFirstType<ElementT, ElementsT...>();
		}
		const std::type_info *GetTypeInfo() const noexcept override {
			return &typeid(ElementT);
		}
		void *GetAddress() noexcept override {
			return &reinterpret_cast<char &>(x_vElement);
		}
		UniquePtr<xActiveElementBase> Clone() const override {
			return UniquePtr<xActiveElementBase>(new auto(*this));
		}
	};

private:
	UniquePtr<xActiveElementBase> x_pElement;

public:
	constexpr Variant() noexcept
		: x_pElement()
	{
	}
	template<typename ElementT, std::enable_if_t<
		FindFirstType<ElementT, ElementsT...>() == FindLastType<ElementT, ElementsT...>(),
		int> = 0>
	Variant(ElementT vElement)
		: x_pElement(MakeUnique<xActiveElement<std::remove_cv_t<ElementT>>>(std::move(vElement)))
	{
	}
	Variant(const Variant &rhs)
		: x_pElement(rhs.x_pElement ? rhs.x_pElement->Clone() : nullptr)
	{
	}
	Variant(Variant &&rhs) noexcept
		: x_pElement(std::move(rhs.x_pElement))
	{
	}
	template<typename ElementT, std::enable_if_t<
		FindFirstType<ElementT, ElementsT...>() == FindLastType<ElementT, ElementsT...>(),
		int> = 0>
	Variant &operator=(ElementT vElement){
		x_pElement = MakeUnique<xActiveElement<std::remove_cv_t<ElementT>>>(std::move(vElement));
		return *this;
	}
	Variant &operator=(const Variant &rhs){
		x_pElement = rhs.x_pElement ? rhs.x_pElement->Clone() : nullptr;
		return *this;
	}
	Variant &operator=(Variant &&rhs) noexcept {
		x_pElement = std::move(rhs.x_pElement);
		return *this;
	}

public:
	std::size_t GetIndex() const noexcept {
		if(!x_pElement){
			return kEmpty;
		}
		return x_pElement->GetIndex();
	}
	const std::type_info *GetTypeInfo() const noexcept {
		if(!x_pElement){
			return nullptr;
		}
		return x_pElement->GetTypeInfo();
	}
	template<typename ElementT>
	const ElementT *Get() const noexcept {
		const auto pElement = dynamic_cast<xActiveElement<std::remove_cv_t<ElementT>> *>(x_pElement.Get());
		if(!pElement){
			return nullptr;
		}
		return static_cast<const ElementT *>(pElement->GetAddress());
	}
	template<typename ElementT>
	ElementT *Get() noexcept {
		const auto pElement = dynamic_cast<xActiveElement<std::remove_cv_t<ElementT>> *>(x_pElement.Get());
		if(!pElement){
			return nullptr;
		}
		return static_cast<ElementT *>(pElement->GetAddress());
	}
	template<typename ElementT, std::enable_if_t<
		FindFirstType<ElementT, ElementsT...>() == FindLastType<ElementT, ElementsT...>(),
		int> = 0>
	void Set(ElementT vElement){
		x_pElement = MakeUnique<xActiveElement<std::remove_cv_t<ElementT>>>(std::move(vElement));
	}
	template<typename ElementT, typename ...ParamsT, std::enable_if_t<
		FindFirstType<ElementT, ElementsT...>() == FindLastType<ElementT, ElementsT...>(),
		int> = 0>
	void Emplace(ParamsT &&...vParams){
		x_pElement = MakeUnique<xActiveElement<std::remove_cv_t<ElementT>>>(std::forward<ParamsT>(vParams)...);
	}

	void Swap(Variant<ElementsT...> &rhs) noexcept {
		x_pElement.Swap(rhs.x_pElement);
	}
};

template<typename ...ElementsT>
void swap(Variant<ElementsT...> &lhs, Variant<ElementsT...> &rhs) noexcept {
	lhs.Swap(rhs);
}

}

#endif
