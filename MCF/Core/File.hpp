// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef MCF_FILE_HPP_
#define MCF_FILE_HPP_

#include "Utilities.hpp"
#include "StringObserver.hpp"
#include <memory>
#include <functional>
#include <cstddef>
#include <cstdint>

namespace MCF {

class File : NO_COPY, ABSTRACT {
public:
	enum : std::uint64_t {
		INVALID_SIZE = (std::uint64_t)-1
	};

	typedef std::function<void ()> AsyncProc;

public:
	static std::unique_ptr<File> Open(const WideStringObserver &wsoPath, bool bToRead, bool bToWrite, bool bAutoCreate);
	static std::unique_ptr<File> OpenNoThrow(const WideStringObserver &wsoPath, bool bToRead, bool bToWrite, bool bAutoCreate);

public:
	std::uint64_t GetSize() const;
	void Resize(std::uint64_t u64NewSize);
	void Clear();

	std::size_t Read(void *pBuffer, std::size_t uBytesToRead, std::uint64_t u64Offset) const;
	std::size_t Read(void *pBuffer, std::size_t uBytesToRead, std::uint64_t u64Offset, AsyncProc fnAsyncProc) const;
	void Write(std::uint64_t u64Offset, const void *pBuffer, std::size_t uBytesToWrite);
	void Write(std::uint64_t u64Offset, const void *pBuffer, std::size_t uBytesToWrite, AsyncProc fnAsyncProc);
};

}

#endif
