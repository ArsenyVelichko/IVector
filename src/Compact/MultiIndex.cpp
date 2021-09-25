#include <cstdint>
#include <cstring>

#include "MultiIndex.h"

RC IMultiIndex::setLogger(ILogger* const logger) {
	return LogProducer<MultiIndex>::setLogger(logger);
}

ILogger* IMultiIndex::getLogger() { //
	return LogProducer<MultiIndex>::getLogger();
}

MultiIndex::MultiIndex(size_t dim) { m_dim = dim; }

MultiIndex* MultiIndex::createMultiIndex(size_t dim, const size_t* indices) {
	size_t size = sizeof(MultiIndex) + dim * sizeof(size_t);
	uint8_t* mem = new (std::nothrow) uint8_t[size];

	if (!mem) {
		log_warning(RC::ALLOCATION_ERROR);
		return nullptr;
	}

	MultiIndex* mi = new (mem) MultiIndex(dim);
	mi->setData(dim, indices);
	return mi;
}

IMultiIndex* MultiIndex::clone() const { return createMultiIndex(m_dim, getData()); }

size_t MultiIndex::getDim() const { return m_dim; }

size_t* MultiIndex::getData() {
	auto memBegin = reinterpret_cast<uint8_t*>(this);
	auto dataBegin = memBegin + sizeof(MultiIndex);
	return reinterpret_cast<size_t*>(dataBegin);
}

const size_t* MultiIndex::getData() const {
	auto memBegin = reinterpret_cast<const uint8_t*>(this);
	auto dataBegin = memBegin + sizeof(MultiIndex);
	return reinterpret_cast<const size_t*>(dataBegin);
}

RC MultiIndex::setData(size_t dim, const size_t* const& ptr_data) {
	if (dim != m_dim) {
		log_warning(RC::MISMATCHING_DIMENSIONS);
		return RC::MISMATCHING_DIMENSIONS;
	}

	memcpy(getData(), ptr_data, dim);
	return RC::SUCCESS;
}

RC MultiIndex::getAxisIndex(size_t axisIndex, size_t& val) const {
	if (axisIndex >= m_dim) {
		log_warning(RC::INDEX_OUT_OF_BOUND);
		return RC::INDEX_OUT_OF_BOUND;
	}

	val = getData()[axisIndex];
	return RC::SUCCESS;
}

RC MultiIndex::setAxisIndex(size_t axisIndex, size_t val) {
	if (axisIndex >= m_dim) {
		log_warning(RC::INDEX_OUT_OF_BOUND);
		return RC::INDEX_OUT_OF_BOUND;
	}

	getData()[axisIndex] = val;
	return RC::SUCCESS;
}

RC MultiIndex::incAxisIndex(size_t axisIndex, size_t val) {
	if (axisIndex >= m_dim) {
		log_warning(RC::INDEX_OUT_OF_BOUND);
		return RC::INDEX_OUT_OF_BOUND;
	}

	getData()[axisIndex] += val;
	return RC::SUCCESS;
}

MultiIndex::~MultiIndex() = default;

IMultiIndex* IMultiIndex::createMultiIndex(size_t dim, const size_t* indices) {
	return MultiIndex::createMultiIndex(dim, indices);
}

IMultiIndex::~IMultiIndex() = default;
