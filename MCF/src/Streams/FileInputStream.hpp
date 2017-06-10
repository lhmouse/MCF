// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

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
	explicit FileInputStream(File &&vFile = File(), std::uint64_t u64Offset = 0) noexcept
		: x_vFile(std::move(vFile)), x_u64Offset(u64Offset)
	{ }
	~FileInputStream() override;

public:
	int Peek() override;
	int Get() override;
	bool Discard() override;
	std::size_t Peek(void *pData, std::size_t uSize) override;
	std::size_t Get(void *pData, std::size_t uSize) override;
	std::size_t Discard(std::size_t uSize) override;
	void Invalidate() override;

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
};

}

#endif
