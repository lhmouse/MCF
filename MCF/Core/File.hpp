// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_FILE_HPP_
#define MCF_CORE_FILE_HPP_

#include "String.hpp"
#include "UniqueHandle.hpp"
#include <functional>
#include <cstddef>
#include <cstdint>

namespace MCF {

class File {
public:
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

private:
	struct xFileCloser {
		void *operator()() const noexcept;
		void operator()(void *hFile) const noexcept;
	};

private:
	UniqueHandle<xFileCloser> xm_hFile;

public:
	File(const wchar_t *pwszPath, std::uint32_t u32Flags);
	File(const WideString &wsPath, std::uint32_t u32Flags);

	constexpr File() noexcept = default;
	File(File &&) noexcept = default;
	File &operator=(File &&) = default;

	File(const File &) = delete;
	File &operator=(const File &) = delete;

public:
	bool IsOpen() const noexcept;
	void Open(const wchar_t *pwszPath, std::uint32_t u32Flags);
	void Open(const WideString &wsPath, std::uint32_t u32Flags);
	bool OpenNoThrow(const wchar_t *pwszPath, std::uint32_t u32Flags);
	bool OpenNoThrow(const WideString &wsPath, std::uint32_t u32Flags);
	void Close() noexcept;

	std::uint64_t GetSize() const;
	void Resize(std::uint64_t u64NewSize);
	void Clear();

	// 1. fnAsyncProc 总是会被执行一次，即使读取或写入操作失败；
	// 2. 如果 IO 请求不能一次完成（例如尝试在 64 位环境下一次读取超过 4GiB 的数据），将会拆分为多次进行。
	//    但是在这种情况下，只有第一次的操作是异步的并且会触发回调；
	// 3. 所有的回调函数都可以抛出异常；在这种情况下，异常将在读取或写入操作完成或失败后被重新抛出。
	// 4. 当且仅当 fnAsyncProc 成功返回且异步操作成功后 fnCompleteCallback 才会被执行。
	std::size_t Read(void *pBuffer, std::size_t uBytesToRead, std::uint64_t u64Offset,
		const std::function<void ()> &fnAsyncProc = std::function<void ()>(),
		const std::function<void ()> &fnCompleteCallback = std::function<void ()>()) const;
	void Write(std::uint64_t u64Offset, const void *pBuffer, std::size_t uBytesToWrite,
		const std::function<void ()> &fnAsyncProc = std::function<void ()>(),
		const std::function<void ()> &fnCompleteCallback = std::function<void ()>());
	void Flush() const;

public:
	explicit operator bool() const noexcept {
		return IsOpen();
	}
};

}

#endif
