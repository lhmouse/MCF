// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_STREAM_FILTERS_BUFFERED_OUTPUT_STREAM_HPP_
#define MCF_STREAM_FILTERS_BUFFERED_OUTPUT_STREAM_HPP_

#include "../Streams/AbstractOutputStream.hpp"
#include "../Core/StreamBuffer.hpp"
#include "../Containers/Vector.hpp"

namespace MCF {

namespace Impl_BufferedOutputStream {
	class BufferedOutputStream {
	private:
		PolyIntrusivePtr<AbstractOutputStream> x_pUnderlyingStream;

		StreamBuffer x_sbufBufferedData;
		Vector<unsigned char> x_vecBackBuffer;

	public:
		explicit BufferedOutputStream(PolyIntrusivePtr<AbstractOutputStream> pUnderlyingStream) noexcept
			: x_pUnderlyingStream(std::move(pUnderlyingStream))
		{
		}
		~BufferedOutputStream();

		BufferedOutputStream(BufferedOutputStream &&) noexcept = default;
		BufferedOutputStream &operator=(BufferedOutputStream &&) noexcept = default;

	private:
		void X_FlushTempBuffer(std::size_t uThreshold);

	public:
		void Put(unsigned char byData);

		void Put(const void *pData, std::size_t uSize);

		void Flush(bool bHard);

		const PolyIntrusivePtr<AbstractOutputStream> &GetUnderlyingStream() const noexcept {
			return x_pUnderlyingStream;
		}

		void Swap(BufferedOutputStream &rhs) noexcept {
			using std::swap;
			swap(x_pUnderlyingStream, rhs.x_pUnderlyingStream);
			swap(x_sbufBufferedData,  rhs.x_sbufBufferedData);
			swap(x_vecBackBuffer,     rhs.x_vecBackBuffer);
		}

		friend void swap(BufferedOutputStream &lhs, BufferedOutputStream &rhs) noexcept {
			lhs.Swap(rhs);
		}
	};
}

}

#endif
