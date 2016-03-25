// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_STREAM_FILTERS_BUFFERED_INPUT_STREAM_HPP_
#define MCF_STREAM_FILTERS_BUFFERED_INPUT_STREAM_HPP_

#include "../Streams/AbstractInputStream.hpp"
#include "../Core/StreamBuffer.hpp"
#include "../Containers/Vector.hpp"

namespace MCF {

namespace Impl_BufferedInputStream {
	class BufferedInputStream {
	private:
		PolyIntrusivePtr<AbstractInputStream> x_pUnderlyingStream;

		StreamBuffer x_sbufFrontBuffer;
		Vector<unsigned char> x_vecBackBuffer;

	public:
		explicit BufferedInputStream(PolyIntrusivePtr<AbstractInputStream> pUnderlyingStream) noexcept
			: x_pUnderlyingStream(std::move(pUnderlyingStream))
		{
		}
		~BufferedInputStream();

		BufferedInputStream(BufferedInputStream &&) noexcept = default;
		BufferedInputStream &operator=(BufferedInputStream &&) noexcept = default;

	public:
		void PopulateBuffer(std::size_t uExpected);

		int BufferedPeek() noexcept {
			return x_sbufFrontBuffer.Peek();
		}
		int BufferedGet() noexcept {
			return x_sbufFrontBuffer.Get();
		}
		bool BufferedDiscard() noexcept {
			return x_sbufFrontBuffer.Discard();
		}

		std::size_t BufferedPeek(void *pData, std::size_t uSize) noexcept {
			return x_sbufFrontBuffer.Peek(pData, uSize);
		}
		std::size_t BufferedGet(void *pData, std::size_t uSize) noexcept {
			return x_sbufFrontBuffer.Get(pData, uSize);
		}
		std::size_t BufferedDiscard(std::size_t uSize) noexcept {
			return x_sbufFrontBuffer.Discard(uSize);
		}

		const PolyIntrusivePtr<AbstractInputStream> &GetUnderlyingStream() const noexcept {
			return x_pUnderlyingStream;
		}

		void Swap(BufferedInputStream &rhs) noexcept {
			using std::swap;
			swap(x_pUnderlyingStream, rhs.x_pUnderlyingStream);
			swap(x_sbufFrontBuffer,   rhs.x_sbufFrontBuffer);
			swap(x_vecBackBuffer,     rhs.x_vecBackBuffer);
		}

	public:
		friend void swap(BufferedInputStream &lhs, BufferedInputStream &rhs) noexcept {
			lhs.Swap(rhs);
		}
	};
}

}

#endif
