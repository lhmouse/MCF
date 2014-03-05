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

class File : MOVABLE {
public:
	enum : std::uint64_t {
		INVALID_SIZE = (std::uint64_t)-1
	};

	typedef std::function<void ()> ASYNC_PROC;
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
	unsigned long Open(const wchar_t *pwszPath, bool bToRead, bool bToWrite, bool bAutoCreate);
	void Close() noexcept;

	std::uint64_t GetSize() const;
	void Resize(std::uint64_t u64NewSize);

	std::uint32_t Read(void *pBuffer, std::uint64_t u64Offset, std::uint32_t u32BytesToRead) const;
	std::uint32_t Read(void *pBuffer, std::uint64_t u64Offset, std::uint32_t u32BytesToRead, ASYNC_PROC fnAsyncProc) const;
	void Write(std::uint64_t u64Offset, const void *pBuffer, std::uint32_t u32BytesToWrite);
	void Write(std::uint64_t u64Offset, const void *pBuffer, std::uint32_t u32BytesToWrite, ASYNC_PROC fnAsyncProc);
public:
	explicit operator bool() const noexcept {
		return IsOpen();
	}
};

}

#endif
