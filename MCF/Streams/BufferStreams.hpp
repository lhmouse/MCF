// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_STREAMS_BUFFER_STREAMS_HPP_
#define MCF_STREAMS_BUFFER_STREAMS_HPP_

#include "AbstractStreams.hpp"
#include "../Core/StreamBuffer.hpp"

namespace MCF {

namespace Impl_BufferStreams {
	template<typename T>
	class BufferInputStreamInterface;
	template<typename T>
	class BufferOutputStreamInterface;

	class BufferStreamBase {
		template<typename>
		friend class BufferInputStreamInterface;
		template<typename>
		friend class BufferOutputStreamInterface;

	private:
		StreamBuffer x_vBuffer;

	public:
		constexpr BufferStreamBase() noexcept
			: x_vBuffer()
		{
		}
		explicit BufferStreamBase(StreamBuffer vBuffer) noexcept
			: BufferStreamBase()
		{
			SetBuffer(std::move(vBuffer));
		}

	private:
		int X_Peek() const noexcept {
			return x_vBuffer.Peek();
		}
		int X_Get() noexcept {
			return x_vBuffer.Get();
		}
		void X_Discard() noexcept {
			return x_vBuffer.Discard();
		}
		void X_Put(unsigned char byData){
			return x_vBuffer.Put(byData);
		}

		std::size_t X_Peek(void *pData, std::size_t uSize) const noexcept {
			return x_vBuffer.Peek(pData, uSize);
		}
		std::size_t X_Get(void *pData, std::size_t uSize) noexcept {
			return x_vBuffer.Get(pData, uSize);
		}
		std::size_t X_Discard(std::size_t uSize) noexcept {
			return x_vBuffer.Discard(uSize);
		}
		void X_Put(const void *pData, std::size_t uSize){
			x_vBuffer.Put(pData, uSize);
		}
		void X_Put(unsigned char byData, std::size_t uSize){
			x_vBuffer.Put(byData, uSize);
		}

	public:
		const StreamBuffer &GetBuffer() const noexcept {
			return x_vBuffer;
		}
		StreamBuffer &GetBuffer() noexcept {
			return x_vBuffer;
		}
		void SetBuffer(StreamBuffer vBuffer) noexcept {
			x_vBuffer = std::move(vBuffer);
		}
	};

	template<typename T>
	class BufferInputStreamInterface : public AbstractInputStream {
	private:
		const BufferStreamBase *X_UseMagic() const noexcept {
			return static_cast<const BufferStreamBase *>(static_cast<const T *>(this));
		}
		BufferStreamBase *X_UseMagic() noexcept {
			return static_cast<BufferStreamBase *>(static_cast<T *>(this));
		}

	public:
		int Peek() const noexcept override {
			return X_UseMagic()->X_Peek();
		}
		int Get() noexcept override {
			return X_UseMagic()->X_Get();
		}
		void Discard() noexcept override {
			return X_UseMagic()->X_Discard();
		}

		std::size_t Peek(void *pData, std::size_t uSize) const noexcept override {
			return X_UseMagic()->X_Peek(pData, uSize);
		}
		std::size_t Get(void *pData, std::size_t uSize) noexcept override {
			return X_UseMagic()->X_Get(pData, uSize);
		}
		std::size_t Discard(std::size_t uSize) noexcept override {
			return X_UseMagic()->X_Discard(uSize);
		}
	};

	template<typename T>
	class BufferOutputStreamInterface : public AbstractOutputStream {
	private:
		const BufferStreamBase *X_UseMagic() const noexcept {
			return static_cast<const BufferStreamBase *>(static_cast<const T *>(this));
		}
		BufferStreamBase *X_UseMagic() noexcept {
			return static_cast<BufferStreamBase *>(static_cast<T *>(this));
		}

	public:
		void Put(unsigned char byData) override {
			X_UseMagic()->X_Put(byData);
		}

		void Put(const void *pData, std::size_t uSize) override {
			X_UseMagic()->X_Put(pData, uSize);
		}
		void Put(unsigned char byData, std::size_t uSize) override {
			X_UseMagic()->X_Put(byData, uSize);
		}
	};
}

class BufferInputStream : public Impl_BufferStreams::BufferStreamBase,
	public Impl_BufferStreams::BufferInputStreamInterface<BufferInputStream>
{
public:
	constexpr BufferInputStream() noexcept
		: Impl_BufferStreams::BufferStreamBase()
	{
	}
	explicit BufferInputStream(StreamBuffer vBuffer) noexcept
		: Impl_BufferStreams::BufferStreamBase(std::move(vBuffer))
	{
	}
	~BufferInputStream() override;
};

class BufferOutputStream : public Impl_BufferStreams::BufferStreamBase,
	public Impl_BufferStreams::BufferOutputStreamInterface<BufferOutputStream>
{
public:
	constexpr BufferOutputStream() noexcept
		: Impl_BufferStreams::BufferStreamBase()
	{
	}
	explicit BufferOutputStream(StreamBuffer vBuffer) noexcept
		: Impl_BufferStreams::BufferStreamBase(std::move(vBuffer))
	{
	}
	~BufferOutputStream() override;
};

class BufferStream : public Impl_BufferStreams::BufferStreamBase,
	public Impl_BufferStreams::BufferInputStreamInterface<BufferStream>, public Impl_BufferStreams::BufferOutputStreamInterface<BufferStream>
{
public:
	constexpr BufferStream() noexcept
		: Impl_BufferStreams::BufferStreamBase()
	{
	}
	explicit BufferStream(StreamBuffer vBuffer) noexcept
		: Impl_BufferStreams::BufferStreamBase(std::move(vBuffer))
	{
	}
	~BufferStream() override;
};

}

#endif
