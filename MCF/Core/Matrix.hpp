// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_MATRIX_HPP_
#define MCF_CORE_MATRIX_HPP_

#include "Array.hpp"
#include <cstddef>

namespace MCF {

template<typename ElementT, std::size_t kRows, std::size_t kColumns>
class Matrix {
public:
	static constexpr std::size_t GetRowCount() noexcept {
		return kRows;
	}
	static constexpr std::size_t GetColumnCount() noexcept {
		return kColumns;
	}

public:
	Array<ElementT, kRows, kColumns> m_aStorage;

public:
	const Array<ElementT, kColumns> &Get(std::size_t kRow) const noexcept {
		return m_aStorage.Get(kRow);
	}
	Array<ElementT, kColumns> &Get(std::size_t kRow) noexcept {
		return m_aStorage.Get(kRow);
	}
	const Array<ElementT, kColumns> &UncheckedGet(std::size_t kRow) const noexcept {
		return m_aStorage.UncheckedGet(kRow);
	}
	Array<ElementT, kColumns> &UncheckedGet(std::size_t kRow) noexcept {
		return m_aStorage.UncheckedGet(kRow);
	}

	const Array<ElementT, kColumns> &operator[](std::size_t kRow) const noexcept {
		return m_aStorage[kRow];
	}
	Array<ElementT, kColumns> &operator[](std::size_t kRow) noexcept {
		return m_aStorage[kRow];
	}
};

template<typename ElementT, std::size_t kRows, std::size_t kColumns>
Matrix<ElementT, kRows, kColumns> &operator+=(Matrix<ElementT, kRows, kColumns> &lhs, const Matrix<ElementT, kRows, kColumns> &rhs){
#pragma GCC ivdep
	for(std::size_t r = 0; r < kRows; ++r){
#pragma GCC ivdep
		for(std::size_t c = 0; c < kColumns; ++c){
			lhs[r][c] += rhs[r][c];
		}
	}
	return lhs;
}
template<typename ElementT, std::size_t kRows, std::size_t kColumns>
Matrix<ElementT, kRows, kColumns> &operator-=(Matrix<ElementT, kRows, kColumns> &lhs, const Matrix<ElementT, kRows, kColumns> &rhs){
#pragma GCC ivdep
	for(std::size_t r = 0; r < kRows; ++r){
#pragma GCC ivdep
		for(std::size_t c = 0; c < kColumns; ++c){
			lhs[r][c] -= rhs[r][c];
		}
	}
	return lhs;
}
template<typename ElementT, std::size_t kRows, std::size_t kColumns>
Matrix<ElementT, kRows, kColumns> &operator*=(Matrix<ElementT, kRows, kColumns> &lhs, const ElementT &rhs){
#pragma GCC ivdep
	for(std::size_t r = 0; r < kRows; ++r){
#pragma GCC ivdep
		for(std::size_t c = 0; c < kColumns; ++c){
			lhs[r][c] *= rhs;
		}
	}
	return lhs;
}
template<typename ElementT, std::size_t kRows, std::size_t kColumns>
Matrix<ElementT, kRows, kColumns> &operator/=(Matrix<ElementT, kRows, kColumns> &lhs, const ElementT &rhs){
#pragma GCC ivdep
	for(std::size_t r = 0; r < kRows; ++r){
#pragma GCC ivdep
		for(std::size_t c = 0; c < kColumns; ++c){
			lhs[r][c] /= rhs;
		}
	}
	return lhs;
}

template<typename ElementT, std::size_t kRows, std::size_t kColumns>
Matrix<ElementT, kRows, kColumns> operator+(const Matrix<ElementT, kRows, kColumns> &rhs){
	return Matrix<ElementT, kRows, kColumns>() + rhs;
}
template<typename ElementT, std::size_t kRows, std::size_t kColumns>
Matrix<ElementT, kRows, kColumns> operator-(const Matrix<ElementT, kRows, kColumns> &rhs){
	return Matrix<ElementT, kRows, kColumns>() - rhs;
}

template<typename ElementT, std::size_t kRows, std::size_t kColumns>
Matrix<ElementT, kRows, kColumns> operator+(const Matrix<ElementT, kRows, kColumns> &lhs, const Matrix<ElementT, kRows, kColumns> &rhs){
	auto ret = lhs;
	ret += rhs;
	return ret;
}
template<typename ElementT, std::size_t kRows, std::size_t kColumns>
Matrix<ElementT, kRows, kColumns> operator+(Matrix<ElementT, kRows, kColumns> &&lhs, const Matrix<ElementT, kRows, kColumns> &rhs){
	return std::move(lhs += rhs);
}
template<typename ElementT, std::size_t kRows, std::size_t kColumns>
Matrix<ElementT, kRows, kColumns> operator-(const Matrix<ElementT, kRows, kColumns> &lhs, const Matrix<ElementT, kRows, kColumns> &rhs){
	auto ret = lhs;
	ret -= rhs;
	return ret;
}
template<typename ElementT, std::size_t kRows, std::size_t kColumns>
Matrix<ElementT, kRows, kColumns> operator-(Matrix<ElementT, kRows, kColumns> &&lhs, const Matrix<ElementT, kRows, kColumns> &rhs){
	return std::move(lhs -= rhs);
}
template<typename ElementT, std::size_t kRows, std::size_t kColumns>
Matrix<ElementT, kRows, kColumns> operator*(const Matrix<ElementT, kRows, kColumns> &lhs, const ElementT &rhs){
	auto ret = lhs;
	ret *= rhs;
	return ret;
}
template<typename ElementT, std::size_t kRows, std::size_t kColumns>
Matrix<ElementT, kRows, kColumns> operator*(Matrix<ElementT, kRows, kColumns> &&lhs, const ElementT &rhs){
	return std::move(lhs *= rhs);
}
template<typename ElementT, std::size_t kRows, std::size_t kColumns>
Matrix<ElementT, kRows, kColumns> operator/(const Matrix<ElementT, kRows, kColumns> &lhs, const ElementT &rhs){
	auto ret = lhs;
	ret /= rhs;
	return ret;
}
template<typename ElementT, std::size_t kRows, std::size_t kColumns>
Matrix<ElementT, kRows, kColumns> operator/(Matrix<ElementT, kRows, kColumns> &&lhs, const ElementT &rhs){
	return std::move(lhs /= rhs);
}

template<typename ElementT, std::size_t kRowsL, std::size_t kColumnsL, std::size_t kColumnsR>
Matrix<ElementT, kRowsL, kColumnsR> operator*(const Matrix<ElementT, kRowsL, kColumnsL> &lhs, const Matrix<ElementT, kColumnsL, kColumnsR> &rhs){
	auto ret = Matrix<ElementT, kRowsL, kColumnsR>();
#pragma GCC ivdep
	for(std::size_t r = 0; r < kRowsL; ++r){
#pragma GCC ivdep
		for(std::size_t c = 0; c < kColumnsR; ++c){
#pragma GCC ivdep
			for(std::size_t i = 0; i < kColumnsL; ++i){
				ret[r][c] += lhs[r][i] * rhs[i][c];
			}
		}
	}
	return ret;
}

}

#endif
