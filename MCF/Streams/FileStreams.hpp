// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_STREAMS_FILE_STREAMS_HPP_
#define MCF_STREAMS_FILE_STREAMS_HPP_

#include "AbstractStreams.hpp"
#include "../Core/File.hpp"
#include "../Core/Exception.hpp"

namespace MCF {

// 性能警告：FileStream 不提供 I/O 缓冲。

namespace Impl_FileStreams {
	template<typename T>
	class FileInputStreamInterface;
	template<typename T>
	class FileOutputStreamInterface;

	class FileStreamBase {
		template<typename>
		friend class FileInputStreamInterface;
		template<typename>
		friend class FileOutputStreamInterface;

	private:
		File x_vFile;
		std::uint64_t x_u64OffsetRead;
		std::uint64_t x_u64OffsetWrite;

	public:
		constexpr FileStreamBase() noexcept
			: x_vFile(), x_u64OffsetRead(0), x_u64OffsetWrite(0)
		{
		}
		explicit FileStreamBase(File vFile, std::uint64_t u64Offset = 0) noexcept
			: FileStreamBase()
		{
			SetFile(std::move(vFile), u64Offset);
		}

	private:
		int X_Peek() const {
			int nRet = -1;
			unsigned char byData;
			if(X_Peek(&byData, 1) >= 1){
				nRet = byData;
			}
			return nRet;
		}
		int X_Get(){
			int nRet = -1;
			unsigned char byData;
			if(X_Get(&byData, 1) >= 1){
				nRet = byData;
			}
			return nRet;
		}
		bool X_Discard(){
			return X_Discard(1) >= 1;
		}
		void X_Put(unsigned char byData){
			X_Put(&byData, 1);
		}

		std::size_t X_Peek(void *pData, std::size_t uSize) const {
			auto uBytesRead = x_vFile.Read(pData, uSize, x_u64OffsetRead);
			return uBytesRead;
		}
		std::size_t X_Get(void *pData, std::size_t uSize){
			auto uBytesRead = x_vFile.Read(pData, uSize, x_u64OffsetRead);
			x_u64OffsetRead += uBytesRead;
			return uBytesRead;
		}
		std::size_t X_Discard(std::size_t uSize){
			std::size_t uBytesDiscarded = 0;
			const auto u64FileSize = x_vFile.GetSize();
			if(x_u64OffsetRead < u64FileSize){
				uBytesDiscarded = uSize;
				const auto uRemaining = u64FileSize - x_u64OffsetRead;
				if(uBytesDiscarded > uRemaining){
					uBytesDiscarded = static_cast<std::size_t>(uRemaining);
				}
				x_u64OffsetRead += uBytesDiscarded;
			}
			return uBytesDiscarded;
		}
		void X_Put(const void *pData, std::size_t uSize){
			const auto uBytesWritten = x_vFile.Write(x_u64OffsetWrite, pData, uSize);
			if(uBytesWritten < uSize){
				DEBUG_THROW(Exception, ERROR_HANDLE_DISK_FULL, "FileStreamBase: Partial contents written"_rcs);
			}
			x_u64OffsetWrite += uBytesWritten;
		}

	public:
		const File &GetFile() const noexcept {
			return x_vFile;
		}
		File &GetFile() noexcept {
			return x_vFile;
		}
		void SetFile(File vFile, std::uint64_t u64Offset = 0) noexcept {
			x_vFile          = std::move(vFile);
			x_u64OffsetRead  = u64Offset;
			x_u64OffsetWrite = u64Offset;
		}

		std::uint64_t GetOffsetRead() const noexcept {
			return x_u64OffsetRead;
		}
		void SetOffsetRead(std::uint64_t u64Offset = 0) noexcept {
			x_u64OffsetRead = u64Offset;
		}

		std::uint64_t GetOffsetWrite() const noexcept {
			return x_u64OffsetWrite;
		}
		void SetOffsetWrite(std::uint64_t u64Offset = 0) noexcept {
			x_u64OffsetWrite = u64Offset;
		}
	};

	template<typename T>
	class FileInputStreamInterface : public AbstractInputStream {
	private:
		const FileStreamBase &X_UseMagic() const noexcept {
			return static_cast<const FileStreamBase &>(static_cast<const T &>(*this));
		}
		FileStreamBase &X_UseMagic() noexcept {
			return static_cast<FileStreamBase &>(static_cast<T &>(*this));
		}

	public:
		int Peek() const noexcept override {
			return X_UseMagic().X_Peek();
		}
		int Get() noexcept override {
			return X_UseMagic().X_Get();
		}
		bool Discard() noexcept override {
			return X_UseMagic().X_Discard();
		}

		std::size_t Peek(void *pData, std::size_t uSize) const noexcept override {
			return X_UseMagic().X_Peek(pData, uSize);
		}
		std::size_t Get(void *pData, std::size_t uSize) noexcept override {
			return X_UseMagic().X_Get(pData, uSize);
		}
		std::size_t Discard(std::size_t uSize) noexcept override {
			return X_UseMagic().X_Discard(uSize);
		}
	};

	template<typename T>
	class FileOutputStreamInterface : public AbstractOutputStream {
	private:
		const FileStreamBase &X_UseMagic() const noexcept {
			return static_cast<const FileStreamBase &>(static_cast<const T &>(*this));
		}
		FileStreamBase &X_UseMagic() noexcept {
			return static_cast<FileStreamBase &>(static_cast<T &>(*this));
		}

	public:
		void Put(unsigned char byData) override {
			X_UseMagic().X_Put(byData);
		}

		void Put(const void *pData, std::size_t uSize) override {
			X_UseMagic().X_Put(pData, uSize);
		}
	};
}

class FileInputStream : public Impl_FileStreams::FileStreamBase,
	public Impl_FileStreams::FileInputStreamInterface<FileInputStream>
{
public:
	using Impl_FileStreams::FileStreamBase::FileStreamBase;

	FileInputStream(FileInputStream &&) noexcept = default;
	FileInputStream &operator=(FileInputStream &&) noexcept = default;

	~FileInputStream() override;
};

class FileOutputStream : public Impl_FileStreams::FileStreamBase,
	public Impl_FileStreams::FileOutputStreamInterface<FileOutputStream>
{
public:
	using Impl_FileStreams::FileStreamBase::FileStreamBase;

	FileOutputStream(FileOutputStream &&) noexcept = default;
	FileOutputStream &operator=(FileOutputStream &&) noexcept = default;

	~FileOutputStream() override;
};

class FileStream : public Impl_FileStreams::FileStreamBase,
	public Impl_FileStreams::FileInputStreamInterface<FileStream>, public Impl_FileStreams::FileOutputStreamInterface<FileStream>
{
public:
	using Impl_FileStreams::FileStreamBase::FileStreamBase;

	FileStream(FileStream &&) noexcept = default;
	FileStream &operator=(FileStream &&) noexcept = default;

	~FileStream() override;
};

}

#endif
