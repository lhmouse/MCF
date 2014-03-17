// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef __MCF_FILE_HPP__
#define __MCF_FILE_HPP__

#include "NoCopy.hpp"
#include <memory>
#include <functional>
#include <cstddef>
#include <cstdint>

namespace MCF {

class File : NO_COPY {
public:
	enum : std::uint64_t {
		INVALID_SIZE = (std::uint64_t)-1
	};

	typedef std::function<void ()> AsyncProc;

private:
	class xDelegate;

private:
	std::unique_ptr<xDelegate> xm_pDelegate;

public:
	File() noexcept;
	File(const wchar_t *pwszPath, bool bToRead, bool bToWrite, bool bAutoCreate);
	File(File &&rhs) noexcept;
	File &operator=(File &&rhs) noexcept;
	~File();

public:
	bool IsOpen() const noexcept;
	unsigned long OpenNoThrow(const wchar_t *pwszPath, bool bToRead, bool bToWrite, bool bAutoCreate);
	void Open(const wchar_t *pwszPath, bool bToRead, bool bToWrite, bool bAutoCreate);
	void Close() noexcept;

	std::uint64_t GetSize() const;
	void Resize(std::uint64_t u64NewSize);

	std::uint32_t Read(void *pBuffer, std::uint64_t u64Offset, std::uint32_t u32BytesToRead) const;
	std::uint32_t Read(void *pBuffer, std::uint64_t u64Offset, std::uint32_t u32BytesToRead, AsyncProc fnAsyncProc) const;
	void Write(std::uint64_t u64Offset, const void *pBuffer, std::uint32_t u32BytesToWrite);
	void Write(std::uint64_t u64Offset, const void *pBuffer, std::uint32_t u32BytesToWrite, AsyncProc fnAsyncProc);

public:
	explicit operator bool() const noexcept {
		return IsOpen();
	}
};

}

#endif
