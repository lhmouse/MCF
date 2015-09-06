// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_CONTAINERS_ENUMERATOR_TEMPLATE_HPP_
#define MCF_CONTAINERS_ENUMERATOR_TEMPLATE_HPP_

#include "../Utilities/Assert.hpp"
#include <iterator>

namespace MCF {

namespace Impl_EnumeratorTemplate {
	template<typename ContainerT>
	class ConstEnumerator;

	template<typename ContainerT>
	class Enumerator : public std::iterator<std::forward_iterator_tag, typename ContainerT::ElementType> {
		friend ConstEnumerator<ContainerT>;

	public:
		using ElementType = typename ContainerT::ElementType;

	private:
		ContainerT *$pContainer;
		ElementType *$pElement;

	public:
		explicit constexpr Enumerator() noexcept
			: $pContainer(nullptr), $pElement(nullptr)
		{
		}
		explicit constexpr Enumerator(ContainerT &vContainer, ElementType *pElement) noexcept
			: $pContainer(&vContainer), $pElement(pElement)
		{
		}

	public:
		bool operator==(const Enumerator &rhs) const noexcept {
			return $pElement == rhs.$pElement;
		}
		bool operator!=(const Enumerator &rhs) const noexcept {
			return !(*this == rhs);
		}

		Enumerator &operator++(){
			ASSERT($pContainer);
			ASSERT($pElement);

			$pElement = $pContainer->GetNext($pElement);
			return *this;
		}
		Enumerator &operator--(){
			ASSERT($pContainer);
			ASSERT($pElement);

			$pElement = $pContainer->GetPrev($pElement);
			return *this;
		}

		Enumerator operator++(int){
			auto enRet = *this;
			++(*this);
			return enRet;
		}
		Enumerator operator--(int){
			auto enRet = *this;
			--(*this);
			return enRet;
		}

		ElementType &operator*() const noexcept {
			ASSERT($pElement);

			return *$pElement;
		}

		explicit operator bool() const noexcept {
			return !!$pElement;
		}
	};

	template<typename ContainerT>
	class ConstEnumerator : public std::iterator<std::forward_iterator_tag, const typename ContainerT::ElementType> {
	public:
		using ElementType = const typename ContainerT::ElementType;

	private:
		const ContainerT *$pContainer;
		const ElementType *$pElement;

	public:
		explicit constexpr ConstEnumerator() noexcept
			: $pContainer(nullptr), $pElement(nullptr)
		{
		}
		explicit constexpr ConstEnumerator(const ContainerT &vContainer, const ElementType *pElement) noexcept
			: $pContainer(&vContainer), $pElement(pElement)
		{
		}
		constexpr ConstEnumerator(const Enumerator<ContainerT> &rhs) noexcept
			: $pContainer(rhs.$pContainer), $pElement(rhs.$pElement)
		{
		}

	public:
		bool operator==(const ConstEnumerator &rhs) const noexcept {
			return $pElement == rhs.$pElement;
		}
		bool operator!=(const ConstEnumerator &rhs) const noexcept {
			return !(*this == rhs);
		}

		ConstEnumerator &operator++(){
			ASSERT($pContainer);
			ASSERT($pElement);

			$pElement = $pContainer->GetNext($pElement);
			return *this;
		}
		ConstEnumerator &operator--(){
			ASSERT($pContainer);
			ASSERT($pElement);

			$pElement = $pContainer->GetPrev($pElement);
			return *this;
		}

		ConstEnumerator operator++(int){
			auto enRet = *this;
			++(*this);
			return enRet;
		}
		ConstEnumerator operator--(int){
			auto enRet = *this;
			--(*this);
			return enRet;
		}

		const ElementType &operator*() const noexcept {
			ASSERT($pElement);

			return *$pElement;
		}

		explicit operator bool() const noexcept {
			return !!$pElement;
		}
	};

}

}

#endif
