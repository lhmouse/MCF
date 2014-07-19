// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef MCF_FILE_HPP_
#define MCF_FILE_HPP_

#include "Utilities.hpp"
#include "String.hpp"
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

	enum : std::uint32_t {
		// 权限控制。
		TO_READ			= 0x00000001,
		TO_WRITE		= 0x00000002,

		// 创建行为控制。如果没有指定 TO_WRITE，这些选项都无效。
		NO_CREATE		= 0x00000004,	// 文件不存在则失败。若指定该选项则 FAIL_IF_EXISTS 无效。
		FAIL_IF_EXISTS	= 0x00000008,	// 文件已存在则失败。

		// 杂项。
		NO_BUFFERING	= 0x00000010,
		WRITE_THROUGH	= 0x00000020,
		DEL_ON_CLOSE	= 0x00000040,
		NO_TRUNC		= 0x00000080,	// 默认情况下使用 TO_WRITE 打开文件会清空现有内容。
	};

public:
	static std::unique_ptr<File> Open(const WideStringObserver &wsoPath, std::uint32_t u32Flags);
	static std::unique_ptr<File> Open(const WideString &wcsPath, std::uint32_t u32Flags);

	static std::unique_ptr<File> OpenNoThrow(const WideStringObserver &wsoPath, std::uint32_t u32Flags);
	static std::unique_ptr<File> OpenNoThrow(const WideString &wcsPath, std::uint32_t u32Flags);

public:
	std::uint64_t GetSize() const;
	void Resize(std::uint64_t u64NewSize);
	void Clear();

	// 异步过程调用总是会被执行，即使读取或写入操作失败。
	// 异步过程可以抛出异常；在这种情况下，异常将在读取或写入操作完成后被重新抛出。
	std::size_t Read(void *pBuffer, std::size_t uBytesToRead, std::uint64_t u64Offset) const;
	std::size_t Read(void *pBuffer, std::size_t uBytesToRead, std::uint64_t u64Offset, const std::function<void ()> &fnAsyncProc) const;

	void Write(std::uint64_t u64Offset, const void *pBuffer, std::size_t uBytesToWrite);
	void Write(std::uint64_t u64Offset, const void *pBuffer, std::size_t uBytesToWrite, const std::function<void ()> &fnAsyncProc);

	void Flush() const;
};

}

#endif
