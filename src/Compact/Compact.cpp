#include <algorithm>
#include <cstring>

#include "VectorUtils.h"

#include "Compact.h"
#include "CompactControlBlock.h"

struct Compact::CompactDef {
	IVector* minBound = nullptr;
	IVector* maxBound = nullptr;
	IMultiIndex* nodeQuantities = nullptr;

	bool isValid() const;
	void clear();
};

Compact* Compact::createCompact(const CompactDef& def) {
	auto compact = new (std::nothrow) Compact(def);
	if (!compact) {
		log_warning(RC::ALLOCATION_ERROR);
		return nullptr;
	}

	auto controlBlock = new (std::nothrow) CompactControlBlock(compact);
	if (!controlBlock) {
		log_warning(RC::ALLOCATION_ERROR);
		delete compact;
		return nullptr;
	}

	compact->m_controlBlock.reset(controlBlock);

	return compact;
}

Compact* Compact::createCompact(const IVector* vec1,
								const IVector* vec2,
								const IMultiIndex* nodeQuantities) {

	if (nodeQuantities->getDim() != vec1->getDim()) {
		log_warning(RC::MISMATCHING_DIMENSIONS);
		return nullptr;
	}

	CompactDef def;
	def.nodeQuantities = nodeQuantities->clone();
	def.minBound = VectorUtils::min(vec1, vec2);
	def.maxBound = VectorUtils::max(vec1, vec2);

	if (!def.isValid()) {
		def.clear();
		return nullptr;
	}

	auto compact = createCompact(def);
	if (!compact) {
		def.clear();
	}
	return compact;
}

ICompact* Compact::clone() const { return createCompact(m_minBound, m_maxBound, m_nodeQuantities); }

size_t Compact::getDim() const { return m_minBound->getDim(); }

IMultiIndex* Compact::getGrid() const { return m_nodeQuantities; }

bool Compact::isInside(IVector const* const& vec) const {

	if (vec->getDim() != getDim()) {
		log_warning(RC::MISMATCHING_DIMENSIONS);
		return false;
	}

	auto minData = m_minBound->getData();
	auto maxData = m_maxBound->getData();
	auto vecData = vec->getData();

	auto isInside = [](double x, double lo, double hi) { return x >= lo && x <= hi; };

	for (size_t i = 0; i < getDim(); i++) {
		if (!isInside(vecData[i], minData[i], maxData[i])) {
			return false;
		}
	}
	return true;
}

RC Compact::getVectorCopy(IMultiIndex const* index, IVector*& val) const {
	auto zeroVec = VectorUtils::createZeroVec(getDim());
	if (!zeroVec) {
		return RC::ALLOCATION_ERROR;
	}

	RC rc = getVectorCoords(index, zeroVec);
	if (rc != RC::SUCCESS) {
		delete zeroVec;
		return rc;
	}

	val = zeroVec;
	return RC::SUCCESS;
}

RC Compact::getVectorCoords(IMultiIndex const* index, IVector* const& val) const {
	if (index->getDim() != getDim()) {
		log_warning(RC::MISMATCHING_DIMENSIONS);
		return RC::MISMATCHING_DIMENSIONS;
	}

	if (!isIndexValid(index)) {
		log_warning(RC::INDEX_OUT_OF_BOUND);
		return RC::INDEX_OUT_OF_BOUND;
	}

	auto nodeData = m_nodeQuantities->getData();
	auto indexData = index->getData();
	auto minData = m_minBound->getData();
	auto maxData = m_maxBound->getData();

	for (size_t i = 0; i < getDim(); i++) {
		double aspect = 0.0;

		if (nodeData[i] > 1) {
			aspect = indexData[i] / double(nodeData[i] - 1);
		}

		double coord = minData[i] + aspect * (maxData[i] - minData[i]);
		val->setCoord(i, coord);
	}
	return RC::SUCCESS;
}

RC Compact::getLeftBoundary(IVector*& vec) const {
	IVector* cloneVec = m_minBound->clone();

	if (cloneVec == nullptr) {
		return RC::ALLOCATION_ERROR;
	}

	vec = cloneVec;
	return RC::SUCCESS;
}

RC Compact::getRightBoundary(IVector*& vec) const {
	IVector* cloneVec = m_maxBound->clone();

	if (cloneVec == nullptr) {
		return RC::ALLOCATION_ERROR;
	}

	vec = cloneVec;
	return RC::SUCCESS;
}

static bool applyTolerance(Compact::CompactDef& def, double tol) {
	size_t dim = def.minBound->getDim();

	auto minData = def.minBound->getData();
	auto maxData = def.maxBound->getData();
	auto nodeData = def.nodeQuantities->getData();

	for (size_t i = 0; i < dim; i++) {
		double dist = maxData[i] - minData[i];
		if (dist < 0) {
			return false;
		}

		size_t maxNodesNum = static_cast<size_t>(dist / tol) + 1;
		size_t nodeNum = std::min(nodeData[i], maxNodesNum);
		def.nodeQuantities->setAxisIndex(i, nodeNum);
	}

	return true;
}

ICompact* ICompact::createIntersection(ICompact const* op1,
									   ICompact const* op2,
									   IMultiIndex const* const grid,
									   double tol) {
	if (!op1 || !op2) {
		log_warning(RC::NULLPTR_ERROR);
		return nullptr;
	}

	Compact::CompactDef def1;
	op1->getLeftBoundary(def1.minBound);
	op1->getRightBoundary(def1.maxBound);

	Compact::CompactDef def2;
	op2->getLeftBoundary(def2.minBound);
	op2->getRightBoundary(def2.maxBound);

	Compact::CompactDef intersectionDef;
	intersectionDef.minBound = VectorUtils::max(def1.minBound, def2.minBound);
	intersectionDef.maxBound = VectorUtils::min(def1.maxBound, def2.maxBound);
	intersectionDef.nodeQuantities = grid->clone();

	if (!intersectionDef.isValid() || !applyTolerance(intersectionDef, tol)) {
		def1.clear();
		def2.clear();
		intersectionDef.clear();

		return nullptr;
	}

	auto intersection = Compact::createCompact(intersectionDef);
	if (!intersection) {
		log_warning(RC::ALLOCATION_ERROR);
		intersectionDef.clear();
	}

	def1.clear();
	def2.clear();
	return intersection;
}

ICompact* ICompact::createCompactSpan(ICompact const* op1,
									  ICompact const* op2,
									  IMultiIndex const* const grid) {
	if (!op1 || !op2) {
		log_warning(RC::NULLPTR_ERROR);
		return nullptr;
	}

	Compact::CompactDef def1;
	op1->getLeftBoundary(def1.minBound);
	op1->getRightBoundary(def1.maxBound);

	Compact::CompactDef def2;
	op2->getLeftBoundary(def2.minBound);
	op2->getRightBoundary(def2.maxBound);

	Compact::CompactDef spanDef;
	spanDef.minBound = VectorUtils::min(def1.minBound, def2.minBound);
	spanDef.maxBound = VectorUtils::max(def1.maxBound, def2.maxBound);
	spanDef.nodeQuantities = grid->clone();

	if (!spanDef.isValid()) {
		def1.clear();
		def2.clear();
		spanDef.clear();

		return nullptr;
	}

	auto span = Compact::createCompact(spanDef);
	if (!span) {
		log_warning(RC::ALLOCATION_ERROR);
		spanDef.clear();
	}

	def1.clear();
	def2.clear();
	return span;
}

ICompact* ICompact::createCompact(const IVector* vec1,
								  const IVector* vec2,
								  const IMultiIndex* nodeQuantities) {
	return Compact::createCompact(vec1, vec2, nodeQuantities);
}

RC ICompact::setLogger(ILogger* const logger) { return LogContainer<Compact>::setInstance(logger); }
ILogger* ICompact::getLogger() { return LogContainer<Compact>::getInstance(); }

ICompact::~ICompact() = default;

Compact::~Compact() {
	m_controlBlock->invalidateCompact();

	delete m_minBound;
	delete m_maxBound;
	delete m_nodeQuantities;
}

bool Compact::isIndexValid(const IMultiIndex* index) const {
	size_t dim = getDim();
	if (index->getDim() != dim) {
		log_warning(RC::MISMATCHING_DIMENSIONS);
		return false;
	}

	auto indexData = index->getData();
	auto nodeData = m_nodeQuantities->getData();

	for (size_t i = 0; i < dim; i++) {
		if (indexData[i] >= nodeData[i]) {
			return false;
		}
	}
	return true;
}
Compact::Compact(const CompactDef& def) :
	m_minBound(def.minBound), m_maxBound(def.maxBound), m_nodeQuantities(def.nodeQuantities) {}

RC Compact::advance(IMultiIndex* const& pos, const IMultiIndex* const& bypassOrder) {
	auto posData = pos->getData();
	auto orderData = bypassOrder->getData();
	auto gridData = m_nodeQuantities->getData();

	for (size_t i = 0; i < bypassOrder->getDim(); i++) {
		size_t currAxis = orderData[i];

		if (posData[currAxis] == gridData[currAxis] - 1) {
			pos->setAxisIndex(currAxis, 0);

		} else {
			pos->incAxisIndex(currAxis, 1);
			return RC::SUCCESS;
		}
	}

	return RC::INDEX_OUT_OF_BOUND;
}

bool Compact::isOrderValid(const IMultiIndex* order) const {
	size_t dim = getDim();
	if (order->getDim() != dim) {
		log_warning(RC::MISMATCHING_DIMENSIONS);
		return false;
	}

	auto orderData = new (std::nothrow) size_t[dim];
	if (!orderData) {
		log_warning(RC::ALLOCATION_ERROR);
		return false;
	}

	memcpy(orderData, order->getData(), dim * sizeof(size_t));
	std::sort(orderData, orderData + dim);

	for (size_t i = 0; i < dim; i++) {
		if (orderData[i] != i) {
			delete[] orderData;
			return false;
		}
	}

	delete[] orderData;
	return true;
}

bool Compact::CompactDef::isValid() const { return minBound && maxBound && nodeQuantities; }

void Compact::CompactDef::clear() {
	delete minBound;
	minBound = nullptr;

	delete maxBound;
	maxBound = nullptr;

	delete nodeQuantities;
	nodeQuantities = nullptr;
}
