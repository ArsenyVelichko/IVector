#include <limits>
#include <cmath>

#include "Compact.h"
#include "CompactControlBlock.h"


Compact::Compact(IVector* vec1, IVector* vec2, IMultiIndex* nodeQuantities) :
	m_controlBlock(new CompactControlBlock(this)) {
	m_lowerBound = vec1;
	m_upperBound = vec2;
	m_nodeQuantities = nodeQuantities;
}

Compact* Compact::createCompact(IVector const* vec1,
								IVector const* vec2,
								IMultiIndex const* nodeQuantities) {
	size_t dim = vec1->getDim();
	if (dim != vec2->getDim() || dim != nodeQuantities->getDim()) {
		log_warning(RC::MISMATCHING_DIMENSIONS);
		return nullptr;
	}

	IVector* leftBound = vec1->clone();
	IVector* rightBound = vec2->clone();
	IMultiIndex* steps = nodeQuantities->clone();

	if (!leftBound || !rightBound || !steps) {
		delete leftBound;
		delete rightBound;
		delete steps;
		log_warning(RC::ALLOCATION_ERROR);
		return nullptr;
	}

	size_t const* stepsData = steps->getData();
	size_t index;
	for (size_t i = 0; i < dim; i++) {
		index = stepsData[i] == 0 ? 0 : stepsData[i] - 1;
		steps->setAxisIndex(i, index);
	}
	double minCoord, maxCoord;
	const double* leftVecArray = leftBound->getData();
	const double* rightVecArray = rightBound->getData();
	for (size_t i = 0; i < dim; i++) {
		minCoord = fmin(leftVecArray[i], rightVecArray[i]);
		maxCoord = fmax(leftVecArray[i], rightVecArray[i]);
		leftBound->setCoord(i, minCoord);
		rightBound->setCoord(i, maxCoord);
	}
	Compact* compact = new (std::nothrow) Compact(leftBound, rightBound, steps);

	if (!compact) {
		log_warning(RC::ALLOCATION_ERROR);

		delete compact;
		delete leftBound;
		delete rightBound;
		delete steps;

		return nullptr;
	}

	return compact;
}

ICompact* Compact::clone() const { return createCompact(m_lowerBound, m_upperBound, m_nodeQuantities); }

size_t Compact::getDim() const { return m_lowerBound->getDim(); }

IMultiIndex* Compact::getGrid() const { return m_nodeQuantities; }

bool Compact::isInside(IVector const* const& vec) const {
	size_t dim = getDim();

	if (vec->getDim() != dim) {
		log_warning(RC::MISMATCHING_DIMENSIONS);
		return false;
	}

	for (size_t i = 0; i < dim; i++) {
		if (!(m_lowerBound->getData()[i] <= vec->getData()[i]
			  && vec->getData()[i] <= m_upperBound->getData()[i])) {
			return false;
		}
	}
	return true;
}

RC Compact::getVectorCopy(IMultiIndex const* index, IVector*& val) const {
	IVector* result = m_lowerBound->clone();
	size_t dim = getDim();

	if (!result) {
		return RC::ALLOCATION_ERROR;
	}
	if (index->getDim() != dim) {
		log_warning(RC::MISMATCHING_DIMENSIONS);
		delete result;
		return RC::MISMATCHING_DIMENSIONS;
	}

	double coord = 0, lambda = 0;
	const size_t* grid = m_nodeQuantities->getData();
	const size_t* place = index->getData();
	const double* leftVec = m_lowerBound->getData();
	const double* rightVec = m_upperBound->getData();
	for (size_t i = 0; i < dim; i++) {
		lambda = 0;

		if (index->getData()[i] > m_nodeQuantities->getData()[i]) {
			log_warning(RC::INDEX_OUT_OF_BOUND);
			delete result;
			return RC::INDEX_OUT_OF_BOUND;
		}

		if (grid[i] != 0) {
			lambda = (double)place[i] / grid[i];
		}
		coord = (1.0 - lambda) * leftVec[i] + lambda * rightVec[i];
		result->setCoord(i, coord);
	}
	val = result;
	return RC::SUCCESS;
}

RC Compact::getVectorCoords(IMultiIndex const* index, IVector* const& val) const {
	size_t dim = getDim();

	if (index->getDim() != dim) {
		log_warning(RC::MISMATCHING_DIMENSIONS);
		return RC::MISMATCHING_DIMENSIONS;
	}

	const size_t* grid = m_nodeQuantities->getData();
	const size_t* place = index->getData();
	const double* leftVec = m_lowerBound->getData();
	const double* rightVec = m_upperBound->getData();
	double coord, lambda = 0.0;
	for (size_t i = 0; i < dim; i++) {
		lambda = 0.0;

		if (place[i] > grid[i]) {
			log_warning(RC::INDEX_OUT_OF_BOUND);
			return RC::INDEX_OUT_OF_BOUND;
		}

		if (grid[i] != 0) {
			lambda = (double)place[i] / grid[i];
		}
		coord = (1.0 - lambda) * leftVec[i] + lambda * rightVec[i];
		val->setCoord(i, coord);
	}
	return RC::SUCCESS;
}

RC Compact::getLeftBoundary(IVector*& vec) const {
	vec = m_lowerBound->clone();
	return vec == nullptr ? RC::ALLOCATION_ERROR : RC::SUCCESS;
}

RC Compact::getRightBoundary(IVector*& vec) const {
	vec = m_upperBound->clone();
	return vec == nullptr ? RC::ALLOCATION_ERROR : RC::SUCCESS;
}

ICompact* ICompact::createIntersection(ICompact const* op1,
									   ICompact const* op2,
									   IMultiIndex const* const grid,
									   double tol) {
	if (!op1 || !op2 || !grid) {
		Compact::log_warning(RC::NULLPTR_ERROR);
		return nullptr;
	}
	if (op1->getDim() != op2->getDim()) {
		Compact::log_warning(RC::MISMATCHING_DIMENSIONS);
		return nullptr;
	}

	IVector* lb1 = nullptr;
	IVector* rb1 = nullptr;
	IVector* lb2 = nullptr;
	IVector* rb2 = nullptr;
	IMultiIndex* intersectGrid = grid->clone();
	op1->getLeftBoundary(lb1);
	op2->getLeftBoundary(lb2);
	op1->getRightBoundary(rb1);
	op2->getRightBoundary(rb2);

	if (!lb1 || !rb1 || !lb2 || !rb2 || !intersectGrid) {
		delete lb1;
		delete lb2;
		delete rb1;
		delete rb2;
		delete intersectGrid;
		return nullptr;
	}

	IVector* intersectLeft = lb1->clone();
	IVector* intersectRight = lb1->clone();

	if (!intersectLeft || !intersectRight) {
		delete intersectLeft;
		delete intersectRight;
		delete lb1;
		delete lb2;
		delete rb1;
		delete rb2;
		delete intersectGrid;
		return nullptr;
	}

	const double* lb1Data = lb1->getData();
	const double* rb1Data = rb1->getData();
	const double* lb2Data = lb2->getData();
	const double* rb2Data = rb2->getData();
	bool fIntersect = true;
	for (size_t i = 0; i < op1->getDim(); i++) {
		if (lb1Data[i] >= lb2Data[i] && lb1Data[i] <= rb2Data[i]) {
			intersectLeft->setCoord(i, lb1Data[i]);
			if (rb2Data[i] - lb1Data[i] < tol) {
				intersectRight->setCoord(i, lb1Data[i]);
				intersectGrid->setAxisIndex(i, 0);
			} else {
				intersectRight->setCoord(i, rb2Data[i]);
			}
		} else if (lb2Data[i] >= lb1Data[i] && lb2Data[i] <= rb1Data[i]) {
			intersectLeft->setCoord(i, lb2Data[i]);
			if (rb1Data[i] - lb2Data[i] < tol) {
				intersectRight->setCoord(i, lb2Data[i]);
				intersectGrid->setAxisIndex(i, 0);
			} else {
				intersectRight->setCoord(i, rb1Data[i]);
			}

		} else {
			fIntersect = false;
			break;
		}
	}
	delete lb1;
	delete lb2;
	delete rb1;
	delete rb2;
	ICompact* Intersection
		= fIntersect ? ICompact::createCompact(intersectLeft, intersectRight, intersectGrid)
					 : nullptr;
	delete intersectLeft;
	delete intersectRight;
	delete intersectGrid;
	return Intersection;
}

ICompact* ICompact::createCompactSpan(ICompact const* op1,
									  ICompact const* op2,
									  IMultiIndex const* const grid) {
	size_t dim = op1->getDim();

	if (!op1 || !op2 || !grid) {
		Compact::log_warning(RC::NULLPTR_ERROR);
		return nullptr;
	}
	if (dim != op2->getDim() || dim != grid->getDim()) {
		Compact::log_warning(RC::MISMATCHING_DIMENSIONS);
		return nullptr;
	}

	IVector *lb1, *lb2, *rb1, *rb2;
	op1->getLeftBoundary(lb1);
	op2->getLeftBoundary(lb2);
	op1->getRightBoundary(rb1);
	op2->getRightBoundary(rb2);
	double* spanLeft = new (std::nothrow) double[dim];
	double* spanRight = new (std::nothrow) double[dim];

	if (!lb1 || !lb2 || !rb1 || !rb2 || !spanLeft || !spanRight) {
		Compact::log_warning(RC::ALLOCATION_ERROR);

		delete lb1;
		delete lb2;
		delete rb1;
		delete rb2;
		delete[] spanLeft;
		delete[] spanRight;
		return nullptr;
	}

	const double* lb1Data = lb1->getData();
	const double* lb2Data = lb2->getData();
	const double* rb1Data = rb1->getData();
	const double* rb2Data = rb2->getData();
	for (size_t i = 0; i < dim; i++) {
		spanLeft[i] = fmin(lb1Data[i], lb2Data[i]);
		spanRight[i] = fmax(rb1Data[i], rb2Data[i]);
	}
	IVector* spanLeftVec = IVector::createVector(dim, spanLeft);
	IVector* spanRightVec = IVector::createVector(dim, spanRight);

	if (!spanLeftVec || !spanRightVec) {
		delete lb1;
		delete lb2;
		delete rb1;
		delete rb2;
		delete[] spanLeft;
		delete[] spanRight;
		delete spanLeftVec;
		delete spanRightVec;
		return nullptr;
	}

	ICompact* span = ICompact::createCompact(spanLeftVec, spanRightVec, grid);
	delete lb1;
	delete lb2;
	delete rb1;
	delete rb2;
	delete[] spanLeft;
	delete[] spanRight;
	delete spanLeftVec;
	delete spanRightVec;
	return span;
}

ICompact* ICompact::createCompact(const IVector* vec1,
								  const IVector* vec2,
								  const IMultiIndex* nodeQuantities) {
	return Compact::createCompact(vec1, vec2, nodeQuantities);
}

RC ICompact::setLogger(ILogger* const logger) { return LogProducer<Compact>::setLogger(logger); }
ILogger* ICompact::getLogger() { return LogProducer<Compact>::getLogger(); }

ICompact::~ICompact() = default;

Compact::~Compact() {
	m_controlBlock->invalidateCompact();

	delete m_lowerBound;
	delete m_upperBound;
	delete m_nodeQuantities;
}


ICompact::IIterator* Compact::getEnd(const IMultiIndex* const& bypassOrder) const {
	size_t dim = getDim();

	if (dim != bypassOrder->getDim()) {
		log_warning(RC::MISMATCHING_DIMENSIONS);
		return nullptr;
	}

	size_t* place = new (std::nothrow) size_t[dim];

	if (!place) {
		log_warning(RC::ALLOCATION_ERROR);
		return nullptr;
	}

	memcpy(place, m_nodeQuantities->getData(), dim * sizeof(size_t));
	IMultiIndex* iterPlace = IMultiIndex::createMultiIndex(dim, place);
	IMultiIndex* iterBypass = bypassOrder->clone();
	delete[] place;

	if (!iterPlace || !iterBypass) {
		delete iterPlace;
		delete iterBypass;
		return nullptr;
	}

	IVector* vector = nullptr;
	RC code = getVectorCopy(iterPlace, vector);

	if (code != RC::SUCCESS) {
		log_warning(code);

		delete iterPlace;
		delete iterBypass;
		return nullptr;
	}

	Iterator* iterator = new (std::nothrow) Iterator(m_controlBlock, iterPlace, iterBypass, vector);

	if (!iterator) {
		log_warning(RC::ALLOCATION_ERROR);

		delete iterPlace;
		delete iterBypass;
		delete vector;
		return nullptr;
	}

	return iterator;
}

int comparator(const void* a, const void* b) { return (int)(*(ssize_t*)a - *(ssize_t*)b); }

// check bypass for multiple indexes and for dimensions
ICompact::IIterator* Compact::getIterator(const IMultiIndex* const& index,
										  const IMultiIndex* const& bypassOrder) const {

	size_t dim = bypassOrder->getDim();
	if (dim != index->getDim()) {
		log_warning(RC::MISMATCHING_DIMENSIONS);
		return nullptr;
	}

	const size_t* place = index->getData();
	const size_t* grid = m_nodeQuantities->getData();
	size_t* sortedByPass = new (std::nothrow) size_t[dim];

	if (!sortedByPass) {
		log_warning(RC::ALLOCATION_ERROR);
		return nullptr;
	}

	memcpy(sortedByPass, bypassOrder->getData(), dim * sizeof(size_t));
	qsort(sortedByPass, dim, sizeof(size_t), comparator);
	for (size_t i = 0; i < dim; i++) {
		if (place[i] > grid[i]) {
			log_warning(RC::SET_INDEX_OVERFLOW);
			delete[] sortedByPass;
			return nullptr;
		}

		if (sortedByPass[i] != i) {
			log_warning(RC::INVALID_ARGUMENT);
			delete[] sortedByPass;
			return nullptr;
		}
	}
	delete[] sortedByPass;

	IMultiIndex* iterPlace = index->clone();
	IMultiIndex* iterOrder = bypassOrder->clone();

	if (!iterPlace || !iterOrder) {
		delete iterPlace;
		delete iterOrder;
		return nullptr;
	}

	IVector* vector = nullptr;
	RC code = getVectorCopy(iterPlace, vector);

	if (code != RC::SUCCESS) {
		log_warning(code);

		delete iterPlace;
		delete iterOrder;
		return nullptr;
	}

	auto res = new (std::nothrow) Iterator(m_controlBlock, iterPlace, iterOrder, vector);

	if (!res) {
		log_warning(RC::ALLOCATION_ERROR);

		delete iterPlace;
		delete iterOrder;
		delete vector;
		return nullptr;
	}

	return res;
}

ICompact::IIterator* Compact::getBegin(const IMultiIndex* const& bypassOrder) const {
	size_t dim = getDim();

	if (dim != bypassOrder->getDim()) {
		log_warning(RC::MISMATCHING_DIMENSIONS);
		return nullptr;
	}

	size_t* place = (size_t*)calloc(dim, sizeof(size_t));

	if (!place) {
		log_warning(RC::ALLOCATION_ERROR);
		return nullptr;
	}

	IMultiIndex* iterPlace = IMultiIndex::createMultiIndex(dim, place);
	free(place);

	if (!iterPlace) {
		delete iterPlace;
		return nullptr;
	}

	IIterator* iterator = getIterator(iterPlace, bypassOrder);
	delete iterPlace;
	return iterator;
}

RC Compact::advance(IMultiIndex* const& place, const IMultiIndex* const& bypassOrder) {
	auto placeData = place->getData();
	auto orderData = bypassOrder->getData();
	auto gridData = m_nodeQuantities->getData();

	for (size_t i = 0; i < bypassOrder->getDim(); i++) {

		if (placeData[orderData[i]] == gridData[orderData[i]]) {
			place->setAxisIndex(orderData[i], 0);

		} else if (placeData[orderData[i]] < gridData[orderData[i]]) {
			place->setAxisIndex(orderData[i], placeData[orderData[i]] + 1);
			return RC::SUCCESS;
		}
	}

	place->setAxisIndex(0, std::numeric_limits<size_t>::max());
	return RC::SET_INDEX_OVERFLOW;
}
