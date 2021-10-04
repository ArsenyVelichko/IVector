#include "VectorUtils.h"
#include "LogUtils.h"

IVector* VectorUtils::createZeroVec(size_t dim) {
	auto zeroData = new (std::nothrow) double[dim]();
	auto vec = IVector::createVector(dim, zeroData);
	delete[] zeroData;
	return vec;
}

IVector* VectorUtils::binaryOp(const IVector* a, const IVector* b, const BinaryOp& op) {
	if (!a || !b) {
		log_severe_in(IVector::getLogger(), RC::NULLPTR_ERROR);
		return nullptr;
	}

	size_t dim = a->getDim();
	if (dim != b->getDim()) {
		log_warning_in(IVector::getLogger(), RC::MISMATCHING_DIMENSIONS);
		return nullptr;
	}

	auto dataA = a->getData();
	auto dataB = b->getData();

	auto res = createZeroVec(dim);
	if (!res) {
		return nullptr;
	}

	for (size_t i = 0; i < dim; i++) {
		RC rc = res->setCoord(i, op(dataA[i], dataB[i]));
		if (rc != RC::SUCCESS) {
			delete res;
			return nullptr;
		}
	}

	return res;
}

IVector* VectorUtils::min(const IVector* a, const IVector* b) {
	return binaryOp(a, b, std::min<double>);
}

IVector* VectorUtils::max(const IVector* a, const IVector* b) {
    return binaryOp(a, b, std::max<double>);
}
