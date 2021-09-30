#pragma once

#include <memory>

#include <IVector.h>

namespace VectorUtils {
	IVector* createZeroVec(size_t dim);

	using BinaryOp = std::function<double(double, double)>;

	IVector* binaryOp(const IVector* a, const IVector* b, const BinaryOp& op);

	IVector* min(const IVector* a, const IVector* b);
	IVector* max(const IVector* a, const IVector* b);
} // namespace VectorUtils
