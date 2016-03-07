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

		StreamBuffer x_sbufBufferedData;
		Vector<unsigned char> x_vecBackBuffer;

	public:
		explicit BufferedInputStream(PolyIntrusivePtr<AbstractInputStream> pUnderlyingStream) noexcept
			: x_pUnderlyingStream(std::move(pUnderlyingStream))
		{
		}
		~BufferedInputStream();

		BufferedInputStream(BufferedInputStream &&) noexcept = default;
		BufferedInputStream &operator=(BufferedInputStream &&) noexcept = default;

	private:
		void X_PopulateTempBuffer(std::size_t uExpected);

	public:
		int Peek();
		int Get();
		bool Discard();

		std::size_t Peek(void *pData, std::size_t uSize);
		std::size_t Get(void *pData, std::size_t uSize);
		std::size_t Discard(std::size_t uSize);

		void UncheckedDiscard() noexcept;

		void UncheckedDiscard(std::size_t uSize) noexcept;

		const PolyIntrusivePtr<AbstractInputStream> &GetUnderlyingStream() const noexcept {
			return x_pUnderlyingStream;
		}
		void SetUnderlyingStream(PolyIntrusivePtr<AbstractInputStream> pUnderlyingStream) noexcept {
			x_pUnderlyingStream = std::move(pUnderlyingStream);
		}

		void Swap(BufferedInputStream &rhs) noexcept {
			using std::swap;
			swap(x_pUnderlyingStream, rhs.x_pUnderlyingStream);
			swap(x_sbufBufferedData,  rhs.x_sbufBufferedData);
			swap(x_vecBackBuffer,     rhs.x_vecBackBuffer);
		}

		friend void swap(BufferedInputStream &lhs, BufferedInputStream &rhs) noexcept {
			lhs.Swap(rhs);
		}
	};
}

}

#endif
