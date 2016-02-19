// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_STREAMS_FILE_STREAM_SINK_HPP_
#define MCF_STREAMS_FILE_STREAM_SINK_HPP_

#include "AbstractStreamSink.hpp"
#include "../Core/File.hpp"

namespace MCF {

// 性能警告：FileStreamSink 不提供 I/O 缓冲。
class FileStreamSink : public AbstractStreamSink {
private:
	File x_vFile;
	std::uint64_t x_u64Offset;

public:
	constexpr FileStreamSink() noexcept
		: x_vFile(), x_u64Offset(0)
	{
	}
	explicit FileStreamSink(File vFile, std::uint64_t uOffset = 0) noexcept
		: FileStreamSink()
	{
		SetFile(std::move(vFile), uOffset);
	}
	~FileStreamSink() override;

public:
	void Put(unsigned char byData) override;

	void Put(const void *pData, std::size_t uSize) override;
	void Put(unsigned char byData, std::size_t uSize) override;

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
