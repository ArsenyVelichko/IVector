#include <cmath>
#include <cstring>

#include "LogUtils.h"
#include "VectorImpl.h"

using std::isinf;
using std::isnan;
using std::memcpy;

ILogger* VectorImpl::m_logger = nullptr;

VectorImpl* VectorImpl::createVector(size_t dim, double const* const& data) {
	size_t size = sizeof(VectorImpl) + dim * sizeof(double);
	uint8_t* mem = new (std::nothrow) uint8_t[size];
	if (!mem) {
		log_warning(m_logger, RC::ALLOCATION_ERROR);
		return nullptr;
	}
	auto vector = new (mem) VectorImpl(dim);

	if (vector->setData(dim, data) != RC::SUCCESS) {
		delete[] mem;
		return nullptr;
	}

	return vector;
}

RC VectorImpl::setLogger(ILogger* logger) {
	m_logger = logger;
	return RC::SUCCESS;
}

ILogger* VectorImpl::getLogger() { return m_logger; }
ILogger* IVector::getLogger() { return VectorImpl::getLogger(); }

IVector* VectorImpl::clone() const { return VectorImpl::createVector(m_dim, getData()); }

const double* VectorImpl::getData() const {
	auto* memBegin = reinterpret_cast<const int8_t*>(this);
	auto* dataBegin = memBegin + sizeof(VectorImpl);
	return reinterpret_cast<const double*>(dataBegin);
}

double* VectorImpl::getData() {
	auto* memBegin = reinterpret_cast<int8_t*>(this);
	auto* dataBegin = memBegin + sizeof(VectorImpl);
	return reinterpret_cast<double*>(dataBegin);
}

double VectorImpl::infiniteNorm() const {
	auto data = getData();
	double norm = 0;

	for (size_t i = 0; i < m_dim; i++) {
		norm = fmax(norm, fabs(data[i]));
	}

	return norm;
}

double VectorImpl::firstNorm() const {
	auto data = getData();
	double norm = 0;

	for (size_t i = 0; i < m_dim; i++) {
		norm += fabs(data[i]);
	}

	return norm;
}

double VectorImpl::secondNorm() const {
	auto data = getData();
	double norm = 0;

	for (size_t i = 0; i < m_dim; i++) {
		norm += pow(data[i], 2);
	}

	return sqrt(norm);
}

RC VectorImpl::setData(size_t dim, double const* const& data) {

	if (dim != m_dim) {
		return RC::MISMATCHING_DIMENSIONS;
	}

	for (size_t i = 0; i < dim; i++) {
		RC rc = setCord(i, data[i]);
		if (rc != RC::SUCCESS) {
			return rc;
		}
	}

	return RC::SUCCESS;
}

RC VectorImpl::getCord(size_t index, double& val) const {

	if (index >= m_dim) {
		return RC::INDEX_OUT_OF_BOUND;
	}

	val = getData()[index];
	return RC::SUCCESS;
}

RC VectorImpl::setCord(size_t index, double val) {

	if (index >= m_dim) {
		log_warning(m_logger, RC::INDEX_OUT_OF_BOUND);
		return RC::INDEX_OUT_OF_BOUND;
	}

	if (isnan(val)) {
		log_warning(m_logger, RC::NOT_NUMBER);
		return RC::NOT_NUMBER;
	}

	if (isinf(val)) {
		log_warning(m_logger, RC::INFINITY_OVERFLOW);
		return RC::INFINITY_OVERFLOW;
	}

	getData()[index] = val;
	return RC::SUCCESS;
}

RC VectorImpl::scale(double multiplier) {

	if (isnan(multiplier) || isinf(multiplier)) {
		log_warning(m_logger, RC::INVALID_ARGUMENT);
		return RC::INVALID_ARGUMENT;
	}

	auto multiply = [multiplier](double x) { return multiplier * x; };
	return applyFunction(multiply);
}

size_t VectorImpl::getDim() const { return m_dim; }

RC VectorImpl::inc(IVector const* const& op) {
	if (m_dim != op->getDim()) {
		log_warning(m_logger, RC::MISMATCHING_DIMENSIONS);
		return RC::MISMATCHING_DIMENSIONS;
	}

	double* data = getData();
	const double* opData = op->getData();

	for (size_t i = 0; i < m_dim; i++) {
		RC rc = setCord(i, data[i] + opData[i]);
		if (rc != RC::SUCCESS) {
			return rc;
		}
	}
	return RC::SUCCESS;
}

RC VectorImpl::dec(IVector const* const& op) {

	if (m_dim != op->getDim()) {
		log_warning(m_logger, RC::MISMATCHING_DIMENSIONS);
		return RC::MISMATCHING_DIMENSIONS;
	}

	double* data = getData();
	const double* opData = op->getData();

	for (size_t i = 0; i < m_dim; i++) {
		RC rc = setCord(i, data[i] - opData[i]);
		if (rc != RC::SUCCESS) {
			return rc;
		}
	}
	return RC::SUCCESS;
}

double VectorImpl::norm(NORM n) const {
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
		log_severe(m_logger, RC::UNKNOWN);
		return res;
	}

	if (isinf(res)) {
		log_warning(m_logger, RC::INFINITY_OVERFLOW);
		return res;
	}

	return res;
}

RC VectorImpl::applyFunction(const std::function<double(double)>& fun) {
	double* data = getData();

	for (size_t i = 0; i < m_dim; i++) {
		RC rc = setCord(i, fun(data[i]));
		if (rc != RC::SUCCESS) {
			return rc;
		}
	}
	return RC::SUCCESS;
}

RC VectorImpl::foreach (const std::function<void(double)>& fun) const {
	const double* data = getData();
	for (size_t i = 0; i < m_dim; i++) {
		fun(data[i]);
	}
	return RC::SUCCESS;
}

size_t VectorImpl::sizeAllocated() const { return sizeof(VectorImpl) + m_dim * sizeof(double); }

VectorImpl::~VectorImpl() = default;

VectorImpl::VectorImpl(size_t dim) { m_dim = dim; }

IVector* IVector::createVector(size_t dim, double const* const& ptr_data) {
	return VectorImpl::createVector(dim, ptr_data);
}

RC IVector::copyInstance(IVector* const dest, IVector const* const& src) {

	if (dest->sizeAllocated() != src->sizeAllocated()) {
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

RC IVector::setLogger(ILogger* const logger) { return VectorImpl::setLogger(logger); }

IVector* IVector::add(IVector const* const& op1, IVector const* const& op2) {

	if (op1->getDim() != op2->getDim()) {
		log_warning(getLogger(), RC::MISMATCHING_DIMENSIONS);
		return nullptr;
	}

	IVector* sum = createVector(op1->getDim(), op1->getData());
	if (!sum) {
		return nullptr;
	}
	sum->inc(op2);
	return sum;
}

IVector* IVector::sub(IVector const* const& op1, IVector const* const& op2) {

	if (op1->getDim() != op2->getDim()) {
		log_warning(getLogger(), RC::MISMATCHING_DIMENSIONS);
		return nullptr;
	}

	IVector* diff = op1->clone();
	if (!diff) {
		return nullptr;
	}
	diff->dec(op2);
	return diff;
}

double IVector::dot(IVector const* const& op1, IVector const* const& op2) {

	if (op1->getDim() != op2->getDim()) {
		log_warning(getLogger(), RC::MISMATCHING_DIMENSIONS);
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
		log_warning(getLogger(), RC::INFINITY_OVERFLOW);
		return NAN;
	}

	return res;
}

bool IVector::equals(IVector const* const& op1, IVector const* const& op2, NORM n, double tol) {
	IVector* diff = sub(op1, op2);
	if (!diff) {
		log_warning(getLogger(), RC::ALLOCATION_ERROR);
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
