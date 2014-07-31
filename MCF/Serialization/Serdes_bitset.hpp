// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2014, LH_Mouse. All wrongs reserved.

#ifndef MCF_SERDES_BITSET_HPP_
#define MCF_SERDES_BITSET_HPP_

#include "Serdes.hpp"
#include <bitset>
#include <iterator>

namespace MCF {

namespace Impl {
	template<std::size_t BIT_COUNT>
	class BitSetReadIterator :
		public std::iterator<std::input_iterator_tag, bool>
	{
	private:
		const std::bitset<BIT_COUNT> &xm_bsetOwner;
		std::size_t xm_uIndex;

	public:
		explicit BitSetReadIterator(const std::bitset<BIT_COUNT> &bsetOwner)
			: xm_bsetOwner	(bsetOwner)
			, xm_uIndex		(0)
		{
		}

	public:
		BitSetReadIterator &operator++() noexcept {
			++xm_uIndex;
			return *this;
		}
		BitSetReadIterator operator++(int) noexcept {
			auto itRet = *this;
			++*this;
			return std::move(itRet);
		}

		bool operator*() const noexcept {
			ASSERT(xm_uIndex < BIT_COUNT);
			return xm_bsetOwner[xm_uIndex];
		}
	};
	template<std::size_t BIT_COUNT>
	class BitSetWriteIterator :
		public std::iterator<std::output_iterator_tag, bool>
	{
	private:
		std::bitset<BIT_COUNT> &xm_bsetOwner;
		std::size_t xm_uIndex;

	public:
		explicit BitSetWriteIterator(std::bitset<BIT_COUNT> &bsetOwner)
			: xm_bsetOwner	(bsetOwner)
			, xm_uIndex		(0)
		{
		}

	public:
		BitSetWriteIterator &operator++() noexcept {
			++xm_uIndex;
			return *this;
		}
		BitSetWriteIterator operator++(int) noexcept {
			auto itRet = *this;
			++*this;
			return std::move(itRet);
		}

		BitSetWriteIterator &operator*() noexcept {
			return *this;
		}
		BitSetWriteIterator &operator=(bool b) noexcept {
			ASSERT(xm_uIndex < BIT_COUNT);
			xm_bsetOwner.set(xm_uIndex, b);
			return *this;
		}
	};
}

template<std::size_t SIZE>
void operator>>=(const std::bitset<SIZE> &vSource, StreamBuffer &sbufSink){
	MakeSeqInserter<bool>(Impl::BitSetReadIterator<SIZE>(vSource), SIZE) >>= sbufSink;
}
template<std::size_t SIZE>
void operator<<=(std::bitset<SIZE> &vSink, StreamBuffer &sbufSource){
	MakeSeqExtractor<bool>(Impl::BitSetWriteIterator<SIZE>(vSink), SIZE) <<= sbufSource;
}

}

#endif
