// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_STREAMS_FILE_INPUT_STREAM_HPP_
#define MCF_STREAMS_FILE_INPUT_STREAM_HPP_

#include "AbstractInputStream.hpp"
#include "../Core/File.hpp"

namespace MCF {

// 性能警告：FileInputStream 不提供 I/O 缓冲。

class FileInputStream : public AbstractInputStream {
private:
	File x_vFile;
	std::uint64_t x_u64Offset;

public:
	FileInputStream() noexcept
		: x_vFile(), x_u64Offset(0)
	{
	}
	explicit FileInputStream(File &&vFile, std::uint64_t u64Offset = 0) noexcept
		: x_vFile(std::move(vFile)), x_u64Offset(u64Offset)
	{
	}
	~FileInputStream() override;

	FileInputStream(FileInputStream &&) noexcept = default;
	FileInputStream &operator=(FileInputStream &&) noexcept = default;

private:
	std::size_t X_ReadFromCurrentOffset(void *pData, std::size_t uSize);
	std::size_t X_DiscardFromCurrentOffset(std::size_t uSize);

public:
	int Peek() override;
	int Get() override;
	bool Discard() override;

	std::size_t Peek(void *pData, std::size_t uSize) override;
	std::size_t Get(void *pData, std::size_t uSize) override;
	std::size_t Discard(std::size_t uSize) override;

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

	void Swap(FileInputStream &rhs) noexcept {
		using std::swap;
		swap(x_vFile,     rhs.x_vFile);
		swap(x_u64Offset, rhs.x_u64Offset);
	}

public:
	friend void swap(FileInputStream &lhs, FileInputStream &rhs) noexcept {
		lhs.Swap(rhs);
	}
};

}

#endif
