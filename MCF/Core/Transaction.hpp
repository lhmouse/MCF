// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_TRANSACTION_HPP_
#define MCF_CORE_TRANSACTION_HPP_

#include "../Utilities/Noncopyable.hpp"
#include "../Containers/Deque.hpp"
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
class TransactionTemplate: public TransactionItemBase {
private:
	const LockFuncT x_fnLock;
	const CommitFuncT x_fnCommit;
	const UnlockFuncT x_fnUnlock;

public:
	TransactionTemplate(LockFuncT fnLock, CommitFuncT fnCommit, UnlockFuncT fnUnlock)
		: x_fnLock(std::move(fnLock)), x_fnCommit(std::move(fnCommit))
		, x_fnUnlock(std::move(fnUnlock))
	{
	}

public:
	bool Lock() override {
		return x_fnLock();
	}
	void Commit() noexcept override {
		x_fnCommit();
	}
	void Unlock() noexcept override {
		x_fnUnlock();
	}
};

class Transaction : NONCOPYABLE {
private:
	Deque<UniquePtr<TransactionItemBase>> x_deqItems;

public:
	bool IsEmpty() const noexcept {
		return x_deqItems.IsEmpty();
	}
	void Add(UniquePtr<TransactionItemBase> &&pItem){
		x_deqItems.Push(std::move(pItem));
	}
	void Clear() noexcept {
		x_deqItems.Clear();
	}

	template<typename LockFuncT, typename CommitFuncT, typename UnlockFuncT>
	void Add(LockFuncT fnLock, CommitFuncT fnCommit, UnlockFuncT fnUnlock){
		Add(MakeUnique<
			TransactionTemplate<LockFuncT, CommitFuncT, UnlockFuncT>
			>(std::move(fnLock), std::move(fnCommit), std::move(fnUnlock)));
	}

	bool Commit() const;
};

}

#endif
