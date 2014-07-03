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
		TO_READ			= 0x00000100,
		TO_WRITE		= 0x00000200,

		// 创建行为控制。如果没有指定 TO_WRITE，这些选项都无效。
		NO_CREATE		= 0x00001000,	// 文件不存在则失败。若指定该选项则 FAIL_IF_EXISTS 无效。
		FAIL_IF_EXISTS	= 0x00002000,	// 文件已存在则失败。

		// 杂项。
		NO_BUFFERING	= 0x10000000,
		WRITE_THROUGH	= 0x20000000,
		DEL_ON_CLOSE	= 0x40000000,
	};

	typedef std::function<void ()> AsyncProc;

	// 用于在同一时刻确定同一台计算机上的文件。
	// 参见 WinSDK 中的 BY_HANDLE_FILE_INFORMATION 描述。
	struct UniqueId {
		std::uint32_t u32VolumeSN;
		std::uint32_t u32IndexLow;
		std::uint32_t u32IndexHigh;
		std::uint32_t u32Reserved;

		bool operator==(const UniqueId &rhs) const noexcept {
			return BComp(*this, rhs) == 0;
		}
		bool operator!=(const UniqueId &rhs) const noexcept {
			return !(*this == rhs);
		}
		bool operator<(const UniqueId &rhs) const noexcept {
			return BComp(*this, rhs) < 0;
		}
		bool operator>(const UniqueId &rhs) const noexcept {
			return rhs < *this;
		}
		bool operator<=(const UniqueId &rhs) const noexcept {
			return !(rhs < *this);
		}
		bool operator>=(const UniqueId &rhs) const noexcept {
			return !(*this < rhs);
		}
	};

public:
	// nCreateDisposition
	//   > 0	文件存在则打开，不存在则创建（OPEN_ALWAYS）
	//   = 0	文件存在则打开，不存在则失败（OPEN_EXISTING）
	//   < 0	文件存在则失败，不存在则创建（OPEN_ALWAYS）
	static std::unique_ptr<File> Open(const WideStringObserver &wsoPath, std::uint32_t uFlags);
	static std::unique_ptr<File> Open(const WideString &wcsPath, std::uint32_t uFlags);

	static std::unique_ptr<File> OpenNoThrow(const WideStringObserver &wsoPath, std::uint32_t uFlags);
	static std::unique_ptr<File> OpenNoThrow(const WideString &wcsPath, std::uint32_t uFlags);

public:
	std::uint64_t GetSize() const;
	void Resize(std::uint64_t u64NewSize);
	void Clear();

	UniqueId GetUniqueId() const;

	// 异步过程调用总是会被执行，即使读取或写入操作失败。
	// 异步过程可以抛出异常；在这种情况下，异常将在读取或写入操作完成后被重新抛出。
	std::size_t Read(void *pBuffer, std::size_t uBytesToRead, std::uint64_t u64Offset) const;
	std::size_t Read(void *pBuffer, std::size_t uBytesToRead, std::uint64_t u64Offset, const AsyncProc &fnAsyncProc) const;
	void Write(std::uint64_t u64Offset, const void *pBuffer, std::size_t uBytesToWrite);
	void Write(std::uint64_t u64Offset, const void *pBuffer, std::size_t uBytesToWrite, const AsyncProc &fnAsyncProc);
	void Flush() const;
};

}

#endif
