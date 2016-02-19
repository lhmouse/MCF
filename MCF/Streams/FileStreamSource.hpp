// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_STREAMS_BUFFER_FILE_SOURCE_HPP_
#define MCF_STREAMS_BUFFER_FILE_SOURCE_HPP_

#include "AbstractStreamSource.hpp"
#include "../Core/File.hpp"

namespace MCF {

// 性能警告：FileStreamSource 不提供 I/O 缓冲。
class FileStreamSource : public AbstractStreamSource {
private:
	File x_vFile;
	std::uint64_t x_u64Offset;

public:
	constexpr FileStreamSource() noexcept
		: x_vFile(), x_u64Offset(0)
	{
	}
	explicit FileStreamSource(File vFile, std::uint64_t uOffset = 0) noexcept
		: FileStreamSource()
	{
		SetFile(std::move(vFile), uOffset);
	}
	~FileStreamSource() override;

public:
	int Peek() const override;
	int Get() override;
	void Discard() override;

	std::size_t Peek(void *pData, std::size_t uSize) const override;
	std::size_t Get(void *pData, std::size_t uSize) override;
	std::size_t Discard(std::size_t uSize) override;

	const File &GetFile() const noexcept {
		return x_vFile;
	}
	File &GetFile() noexcept {
		return x_vFile;
	}
	void SetFile(File vFile, std::uint64_t uOffset = 0) noexcept {
		x_vFile = std::move(vFile);
		x_u64Offset = uOffset;
	}

	std::uint64_t GetOffset() const noexcept {
		return x_u64Offset;
	}
	void SetOffset(std::uint64_t uOffset) noexcept {
		x_u64Offset = uOffset;
	}
};

}

#endif
