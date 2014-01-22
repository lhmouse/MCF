// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2012 - 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_FILE_HPP__
#define __MCF_FILE_HPP__

#include "NoCopy.hpp"
#include <memory>
#include <cstddef>

namespace MCF {

class File : NO_COPY {
public:
	enum : unsigned long long {
		INVALID_SIZE = -1ull
	};
private:
	class xDelegate;
private:
	const std::unique_ptr<xDelegate> xm_pDelegate;
public:
	File();
	~File();
public:
	bool IsOpen() const noexcept;
	bool Open(const wchar_t *pwszPath, bool bToRead, bool bToWrite, bool bCreatesIfNotExist) noexcept;
	void Close() noexcept;

	unsigned long long GetSize() const noexcept;
	bool Resize(unsigned long long llNewSize) noexcept;

	unsigned long GetErrorCode() noexcept;

	bool IsIdle() const noexcept;
	bool WaitTimeout(unsigned long ulMilliSeconds) const noexcept;
	void Wait() const noexcept;

	void Read(unsigned char *pbyBuffer, unsigned long long ullOffset, std::size_t uBytesToRead) noexcept;
	void Write(unsigned long long ullOffset, const unsigned char *pbyBuffer, std::size_t uBytesToWrite) noexcept;
};

}

#endif
