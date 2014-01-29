// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_FILE_HPP__
#define __MCF_FILE_HPP__

#include "NoCopy.hpp"
#include <memory>
#include <functional>
#include <cstddef>

namespace MCF {

class File : NO_COPY {
public:
	enum : unsigned long long {
		INVALID_SIZE = (unsigned long long)-1
	};

	typedef std::function<void()> ASYNC_PROC;
private:
	class xDelegate;
private:
	std::unique_ptr<xDelegate> xm_pDelegate;
public:
	File() noexcept;
	File(const wchar_t *pwszPath, bool bToRead, bool bToWrite, bool bAutoCreate);
	~File();
public:
	bool IsOpen() const noexcept;
	bool Open(const wchar_t *pwszPath, bool bToRead, bool bToWrite, bool bAutoCreate) noexcept;
	void Close() noexcept;

	std::uint64_t GetSize() const noexcept;
	bool Resize(std::uint64_t u64NewSize) noexcept;

	std::uint32_t Read(void *pBuffer, std::uint64_t u64Offset, std::uint32_t u32BytesToRead) const noexcept;
	std::uint32_t Read(void *pBuffer, std::uint64_t u64Offset, std::uint32_t u32BytesToRead, ASYNC_PROC fnAsyncProc) const;
	std::uint32_t Write(std::uint64_t u64Offset, const void *pBuffer, std::uint32_t u32BytesToWrite) noexcept;
	std::uint32_t Write(std::uint64_t u64Offset, const void *pBuffer, std::uint32_t u32BytesToWrite, ASYNC_PROC fnAsyncProc);
public:
	explicit operator bool() const noexcept {
		return IsOpen();
	}
};

}

#endif
