// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_CONTAINERS_ENUMERATOR_TEMPLATE_HPP_
#define MCF_CONTAINERS_ENUMERATOR_TEMPLATE_HPP_

#include "../Utilities/Assert.hpp"
#include <iterator>

namespace MCF {

namespace Impl_EnumeratorTemplate {
	// ConstEnumerator 和 Enumerator 需求：
	// 0) 成员类型：
	//   ElementType
	// 1) 成员函数：
	//   const ElementType *GetPrev(const ElementType *) const;
	//   ElementType *GetNext(ElementType *);
	//   const ElementType *GetPrev(const ElementType *) const;
	//   ElementType *GetNext(ElementType *);

	template<typename ContainerT>
	class ConstEnumerator;

	template<typename ContainerT>
	class Enumerator : public std::iterator<std::forward_iterator_tag, typename ContainerT::ElementType> {
		friend ConstEnumerator<ContainerT>;

	public:
		using ElementType = typename ContainerT::ElementType;

	private:
		ContainerT *x_pContainer;
		ElementType *x_pElement;

	public:
		explicit constexpr Enumerator() noexcept
			: x_pContainer(nullptr), x_pElement(nullptr)
		{
		}
		explicit constexpr Enumerator(ContainerT &vContainer, ElementType *pElement) noexcept
			: x_pContainer(&vContainer), x_pElement(pElement)
		{
		}

	public:
		bool operator==(const Enumerator &rhs) const noexcept {
			return x_pElement == rhs.x_pElement;
		}
		bool operator!=(const Enumerator &rhs) const noexcept {
			return !(*this == rhs);
		}

		Enumerator &operator++(){
			ASSERT(x_pContainer);
			ASSERT(x_pElement);

			x_pElement = x_pContainer->GetNext(x_pElement);
			return *this;
		}
		Enumerator &operator--(){
			ASSERT(x_pContainer);
			ASSERT(x_pElement);

			x_pElement = x_pContainer->GetPrev(x_pElement);
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
			ASSERT(x_pElement);

			return *x_pElement;
		}
		ElementType *operator->() const noexcept {
			return x_pElement;
		}

		explicit operator bool() const noexcept {
			return !!x_pElement;
		}
		operator ElementType *() const noexcept {
			return x_pElement;
		}
	};

	template<typename ContainerT>
	class ConstEnumerator : public std::iterator<std::forward_iterator_tag, const typename ContainerT::ElementType> {
	public:
		using ElementType = const typename ContainerT::ElementType;

	private:
		const ContainerT *x_pContainer;
		const ElementType *x_pElement;

	public:
		explicit constexpr ConstEnumerator() noexcept
			: x_pContainer(nullptr), x_pElement(nullptr)
		{
		}
		explicit constexpr ConstEnumerator(const ContainerT &vContainer, const ElementType *pElement) noexcept
			: x_pContainer(&vContainer), x_pElement(pElement)
		{
		}
		constexpr ConstEnumerator(const Enumerator<ContainerT> &rhs) noexcept
			: x_pContainer(rhs.x_pContainer), x_pElement(rhs.x_pElement)
		{
		}

	public:
		bool operator==(const ConstEnumerator &rhs) const noexcept {
			return x_pElement == rhs.x_pElement;
		}
		bool operator!=(const ConstEnumerator &rhs) const noexcept {
			return !(*this == rhs);
		}

		ConstEnumerator &operator++(){
			ASSERT(x_pContainer);
			ASSERT(x_pElement);

			x_pElement = x_pContainer->GetNext(x_pElement);
			return *this;
		}
		ConstEnumerator &operator--(){
			ASSERT(x_pContainer);
			ASSERT(x_pElement);

			x_pElement = x_pContainer->GetPrev(x_pElement);
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
			ASSERT(x_pElement);

			return *x_pElement;
		}
		const ElementType *operator->() const noexcept {
			return x_pElement;
		}

		explicit operator bool() const noexcept {
			return !!x_pElement;
		}
		operator const ElementType *() const noexcept {
			return x_pElement;
		}
	};

}

}

#endif
