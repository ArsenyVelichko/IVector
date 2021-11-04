#include <cmath>
#include <cstring>

#include "Vector.h"
#include "VectorUtils.h"

using std::isinf;
using std::isnan;

Vector* Vector::createVector(size_t dim, double const* const& data) {
	size_t size = sizeof(Vector) + dim * sizeof(double);
	auto mem = new (std::nothrow) uint8_t[size];
	if (!mem) {
		log_warning(RC::ALLOCATION_ERROR);
		return nullptr;
	}
	auto vector = new (mem) Vector(dim);

	if (vector->setData(dim, data) != RC::SUCCESS) {
		delete[] mem;
		return nullptr;
	}

	return vector;
}

RC IVector::setLogger(ILogger* const logger) {
	return LogContainer<Vector>::setInstance(logger);
}

ILogger* IVector::getLogger() {
	return LogContainer<Vector>::getInstance();
}

IVector* Vector::clone() const { return Vector::createVector(m_dim, getData()); }

const double* Vector::getData() const {
	auto* memBegin = reinterpret_cast<const int8_t*>(this);
	auto* dataBegin = memBegin + sizeof(Vector);
	return reinterpret_cast<const double*>(dataBegin);
}

double* Vector::getData() {
	auto* memBegin = reinterpret_cast<int8_t*>(this);
	auto* dataBegin = memBegin + sizeof(Vector);
	return reinterpret_cast<double*>(dataBegin);
}

double Vector::infiniteNorm() const {
	auto data = getData();
	double norm = 0;

	for (size_t i = 0; i < m_dim; i++) {
		norm = fmax(norm, fabs(data[i]));
	}

	return norm;
}

double Vector::firstNorm() const {
	auto data = getData();
	double norm = 0;

	for (size_t i = 0; i < m_dim; i++) {
		norm += fabs(data[i]);
	}

	return norm;
}

double Vector::secondNorm() const {
	auto data = getData();
	double norm = 0;

	for (size_t i = 0; i < m_dim; i++) {
		norm += pow(data[i], 2);
	}

	return sqrt(norm);
}

RC Vector::setData(size_t dim, double const* const& data) {

	if (dim != m_dim) {
		return RC::MISMATCHING_DIMENSIONS;
	}

	for (size_t i = 0; i < dim; i++) {
		RC rc = setCoord(i, data[i]);
		if (rc != RC::SUCCESS) {
			return rc;
		}
	}

	return RC::SUCCESS;
}

RC Vector::getCoord(size_t index, double& val) const {

	if (index >= m_dim) {
		return RC::INDEX_OUT_OF_BOUND;
	}

	val = getData()[index];
	return RC::SUCCESS;
}

RC Vector::setCoord(size_t index, double val) {

	if (index >= m_dim) {
		log_warning(RC::INDEX_OUT_OF_BOUND);
		return RC::INDEX_OUT_OF_BOUND;
	}

	if (isnan(val)) {
		log_warning(RC::NOT_NUMBER);
		return RC::NOT_NUMBER;
	}

	if (isinf(val)) {
		log_warning(RC::INFINITY_OVERFLOW);
		return RC::INFINITY_OVERFLOW;
	}

	getData()[index] = val;
	return RC::SUCCESS;
}

RC Vector::scale(double multiplier) {

	if (isnan(multiplier) || isinf(multiplier)) {
		log_warning(RC::INVALID_ARGUMENT);
		return RC::INVALID_ARGUMENT;
	}

	auto multiply = [multiplier](double x) { return multiplier * x; };
	return applyFunction(multiply);
}

size_t Vector::getDim() const { return m_dim; }

RC Vector::inc(IVector const* const& op) {
	if (m_dim != op->getDim()) {
		log_warning(RC::MISMATCHING_DIMENSIONS);
		return RC::MISMATCHING_DIMENSIONS;
	}

	double* data = getData();
	const double* opData = op->getData();

	for (size_t i = 0; i < m_dim; i++) {
		RC rc = setCoord(i, data[i] + opData[i]);
		if (rc != RC::SUCCESS) {
			return rc;
		}
	}
	return RC::SUCCESS;
}

RC Vector::dec(IVector const* const& op) {

	if (m_dim != op->getDim()) {
		log_warning(RC::MISMATCHING_DIMENSIONS);
		return RC::MISMATCHING_DIMENSIONS;
	}

	double* data = getData();
	const double* opData = op->getData();

	for (size_t i = 0; i < m_dim; i++) {
		RC rc = setCoord(i, data[i] - opData[i]);
		if (rc != RC::SUCCESS) {
			return rc;
		}
	}
	return RC::SUCCESS;
}

double Vector::norm(NORM n) const {
	double res = NAN;

	switch (n) {
	case NORM::FIRST:
		res = firstNorm();
		break;

	case NORM::SECOND:
		res = secondNorm();
		break;

	case NORM::CHEBYSHEV:
		res = infiniteNorm();
		break;

	default:
		log_severe(RC::UNKNOWN);
		return res;
	}

	if (isinf(res)) {
		log_warning(RC::INFINITY_OVERFLOW);
		return res;
	}

	return res;
}

RC Vector::applyFunction(const std::function<double(double)>& fun) {
	double* data = getData();

	for (size_t i = 0; i < m_dim; i++) {
		RC rc = setCoord(i, fun(data[i]));
		if (rc != RC::SUCCESS) {
			return rc;
		}
	}
	return RC::SUCCESS;
}

RC Vector::foreach (const std::function<void(double)>& fun) const {
	const double* data = getData();
	for (size_t i = 0; i < m_dim; i++) {
		fun(data[i]);
	}
	return RC::SUCCESS;
}

size_t Vector::sizeAllocated() const { return sizeof(Vector) + m_dim * sizeof(double); }

Vector::Vector(size_t dim) { m_dim = dim; }

IVector* IVector::createVector(size_t dim, double const* const& ptr_data) {
	return Vector::createVector(dim, ptr_data);
}

RC IVector::copyInstance(IVector* const dest, IVector const* const& src) {
	if (!dest || !src) {
		log_severe(RC::NULLPTR_ERROR);
		return RC::NULLPTR_ERROR;
	}

	if (dest->getDim() != src->getDim()) {
		return RC::MISMATCHING_DIMENSIONS;
	}

	auto srcMemBegin = reinterpret_cast<const uint8_t*>(src);
	auto srcMemEnd = srcMemBegin + src->sizeAllocated();
	auto destMemBegin = reinterpret_cast<uint8_t*>(dest);
	auto destMemEnd = destMemBegin + dest->sizeAllocated();

	if (dest >= src && destMemBegin <= srcMemEnd || src >= dest && srcMemBegin <= destMemEnd) {
		return RC::MEMORY_INTERSECTION;
	}

	memcpy(dest, src, src->sizeAllocated());
	return RC::SUCCESS;
}

RC IVector::moveInstance(IVector* const dest, IVector*& src) {
	RC rc = copyInstance(dest, src);
	if (rc != RC::SUCCESS) {
		return rc;
	}

	delete src;
	src = nullptr;
	return RC::SUCCESS;
}

IVector* IVector::add(IVector const* const& op1, IVector const* const& op2) {
	return VectorUtils::binaryOp(op1, op2, [](double x, double y) { return x + y; });
}

IVector* IVector::sub(IVector const* const& op1, IVector const* const& op2) {
	return VectorUtils::binaryOp(op1, op2, [](double x, double y) { return x - y; });
}

double IVector::dot(IVector const* const& op1, IVector const* const& op2) {
	if (!op1 || !op2) {
		log_severe(RC::NULLPTR_ERROR);
		return false;
	}

	if (op1->getDim() != op2->getDim()) {
		log_warning(RC::MISMATCHING_DIMENSIONS);
		return NAN;
	}

	size_t dim = op1->getDim();
	double res = 0;

	const double* data1 = op1->getData();
	const double* data2 = op2->getData();

	for (size_t i = 0; i < dim; i++) {
		res += data1[i] * data2[i];
	}

	if (isinf(res)) {
		log_warning(RC::INFINITY_OVERFLOW);
		return NAN;
	}

	return res;
}

bool IVector::equals(IVector const* const& op1, IVector const* const& op2, NORM n, double tol) {
	IVector* diff = sub(op1, op2);
	if (!diff) {
		return false;
	}

	double diffNorm = diff->norm(n);
	delete diff;

	if (isnan(diffNorm)) {
		return false;
	}

	return diffNorm < tol;
}

IVector::~IVector() = default;
