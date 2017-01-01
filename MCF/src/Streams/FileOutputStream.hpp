// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#ifndef MCF_STREAMS_FILE_OUTPUT_STREAM_HPP_
#define MCF_STREAMS_FILE_OUTPUT_STREAM_HPP_

#include "AbstractOutputStream.hpp"
#include "../Core/File.hpp"

namespace MCF {

// 性能警告：FileOutputStream 不提供 I/O 缓冲。

class FileOutputStream : public AbstractOutputStream {
private:
	File x_vFile;
	std::uint64_t x_u64Offset;

public:
	FileOutputStream() noexcept
		: x_vFile(), x_u64Offset(0)
	{
	}
	explicit FileOutputStream(File &&vFile, std::uint64_t u64Offset = 0) noexcept
		: x_vFile(std::move(vFile)), x_u64Offset(u64Offset)
	{
	}
	~FileOutputStream() override;

	FileOutputStream(FileOutputStream &&) noexcept = default;
	FileOutputStream &operator=(FileOutputStream &&) noexcept = default;

private:
	std::size_t X_WriteFromCurrentOffset(const void *pData, std::size_t uSize);
	void X_Flush(bool bHard);

public:
	void Put(unsigned char byData) override;
	void Put(const void *pData, std::size_t uSize) override;
	void Flush(bool bHard) override;

	const File &GetFile() const noexcept {
		return x_vFile;
	}
	File &GetFile() noexcept {
		return x_vFile;
	}
	void SetFile(File &&vFile, std::uint64_t u64Offset = 0) noexcept {
		x_vFile     = std::move(vFile);
		x_u64Offset = u64Offset;
	}

	std::uint64_t GetOffset() const noexcept {
		return x_u64Offset;
	}
	void SetOffset(std::uint64_t u64Offset) noexcept {
		x_u64Offset = u64Offset;
	}

	void Swap(FileOutputStream &rhs) noexcept {
		using std::swap;
		swap(x_vFile,     rhs.x_vFile);
		swap(x_u64Offset, rhs.x_u64Offset);
	}

public:
	friend void swap(FileOutputStream &lhs, FileOutputStream &rhs) noexcept {
		lhs.Swap(rhs);
	}
};

}

#endif
