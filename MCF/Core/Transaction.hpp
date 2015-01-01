// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_TRANSACTION_HPP_
#define MCF_CORE_TRANSACTION_HPP_

#include "../Utilities/Noncopyable.hpp"
#include "../Containers/Vector.hpp"
#include "../SmartPointers/UniquePtr.hpp"
#include <utility>

namespace MCF {

class TransactionItemBase : NONCOPYABLE {
public:
	virtual ~TransactionItemBase();

public:
	virtual bool Lock() = 0;
	virtual void Commit() noexcept = 0;
	virtual void Unlock() noexcept = 0;
};

template<typename LockFuncT, typename CommitFuncT, typename UnlockFuncT>
class TransactionItem: public TransactionItemBase {
private:
	const LockFuncT xm_fnLock;
	const CommitFuncT xm_fnCommit;
	const UnlockFuncT xm_fnUnlock;

public:
	TransactionItem(LockFuncT fnLock, CommitFuncT fnCommit, UnlockFuncT fnUnlock)
		: xm_fnLock(std::move(fnLock)), xm_fnCommit(std::move(fnCommit))\
		, xm_fnUnlock(std::move(fnUnlock))
	{
	}

public:
	bool Lock() override {
		return xm_fnLock();
	}
	void Commit() noexcept override {
		return xm_fnCommit();
	}
	void Unlock() noexcept override {
		return xm_fnUnlock();
	}
};

class Transaction : NONCOPYABLE {
private:
	Vector<UniquePtr<TransactionItemBase>> xm_vecItems;

public:
	bool IsEmpty() const noexcept;
	void AddItem(UniquePtr<TransactionItemBase> &&pItem);
	void Clear() noexcept;

	bool Commit() const;

	template<typename LockFuncT, typename CommitFuncT, typename UnlockFuncT>
	void AddItem(LockFuncT &&fnLock, CommitFuncT &&fnCommit, UnlockFuncT &&fnUnlock){
		AddItem(std::make_unique(std::forward<LockFuncT>(fnLock)),
			std::make_unique(std::forward<CommitFuncT>(fnCommit)),
			std::make_unique(std::forward<UnlockFuncT>(fnUnlock)));
	}
};

}

#endif
