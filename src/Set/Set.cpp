#include <algorithm>
#include <cstring>

#include "VectorUtils.h"

#include "Set.h"
#include "SetControlBlock.h"

RC ISet::setLogger(ILogger* const logger) {
	return LogContainer<Set>::setInstance(logger);
}

ILogger* ISet::getLogger() {
	return LogContainer<Set>::getInstance();
}

size_t Set::vecDataSize() const { return m_dim * sizeof(double); }

size_t Set::getDim() const { return m_dim; }

size_t Set::getSize() const { return m_size; }

RC Set::getCopy(size_t index, IVector*& val) const {
	if (index >= m_size) {
		log_warning(RC::INDEX_OUT_OF_BOUND);
		return RC::INDEX_OUT_OF_BOUND;
	}

	IVector* vector = IVector::createVector(m_dim, getData(index));
	if (!vector) {
		return RC::ALLOCATION_ERROR;
	}

	val = vector;
	return RC::SUCCESS;
}

RC Set::findFirst(IVector const* pat, IVector::NORM n, double tol, size_t& index) const {
	if (pat->getDim() != m_dim) {
		log_warning(RC::MISMATCHING_DIMENSIONS);
		return RC::MISMATCHING_DIMENSIONS;
	}

	auto vec = VectorUtils::createZeroVec(m_dim);
	if (!vec) {
		return RC::ALLOCATION_ERROR;
	}

	for (size_t i = 0; i < m_size; i++) {
		vec->setData(m_dim, getData(i));

		if (IVector::equals(pat, vec, n, tol)) {
			index = i;
			delete vec;
			return RC::SUCCESS;
		}
	}

	delete vec;
	return RC::VECTOR_NOT_FOUND;
}

double* Set::getData(size_t index) const { return m_data + m_dim * index; }

RC Set::findFirstAndCopy(IVector const* const& pat,
						 IVector::NORM n,
						 double tol,
						 IVector*& val) const {
	size_t index;
	RC rc = findFirst(pat, n, tol, index);
	if (rc != RC::SUCCESS) {
		return rc;
	}

	return getCopy(index, val);
}

RC Set::findFirstAndCopyCoords(const IVector* const& pat,
							   IVector::NORM n,
							   double tol,
							   IVector* const& val) const {
	size_t index;
	RC code = findFirst(pat, n, tol, index);
	if (code != RC::SUCCESS) {
		return code;
	}

	return getCoords(index, val);
}

RC Set::findFirst(const IVector* const& pat, IVector::NORM n, double tol) const {
	size_t _;
	return findFirst(pat, n, tol, _);
}

RC Set::getCoords(size_t index, IVector* const& val) const {
	if (index >= m_size) {
		log_warning(RC::INDEX_OUT_OF_BOUND);
		return RC::INDEX_OUT_OF_BOUND;
	}

	if (val->getDim() != m_dim) {
		log_warning(RC::MISMATCHING_DIMENSIONS);
		return RC::MISMATCHING_DIMENSIONS;
	}

	return val->setData(m_dim, getData(index));
}

bool Set::enlarge() {
	size_t newSize = std::max(size_t(1), m_capacity * 2);

	auto newData = new (std::nothrow) double[newSize * m_dim];
	auto newHash = new (std::nothrow) size_t[newSize];

	if (!newData || !newHash) {
		delete[] newData;
		delete[] newHash;
		return false;
	}

	if (m_size != 0) {
		memcpy(newData, m_data, m_size * vecDataSize());
		delete[] m_data;

		memcpy(newHash, m_hashArr, m_size * sizeof(size_t));
		delete[] m_hashArr;
	}

	m_capacity = newSize;
	m_data = newData;
	m_hashArr = newHash;
	return true;
}

RC Set::insert(IVector const* const& val, IVector::NORM n, double tol) {
	if (m_size == 0) {
		m_dim = val->getDim();
	}

	RC rc = findFirst(val, n, tol);
	if (rc == RC::SUCCESS) {
		log_info(RC::VECTOR_ALREADY_EXIST);
		return RC::VECTOR_ALREADY_EXIST;

	} else if (rc != RC::VECTOR_NOT_FOUND) {
		return rc;
	}

	if (m_size == m_capacity) {
		if (!enlarge()) {
			log_warning(RC::ALLOCATION_ERROR);
			return RC::ALLOCATION_ERROR;
		}
	}

	memcpy(getData(m_size), val->getData(), vecDataSize());
	m_hashArr[m_size] = m_topHash;
	m_topHash++;
	m_size++;

	return RC::SUCCESS;
}

RC Set::remove(size_t index) {
	if (index >= m_size) {
		log_warning(RC::INDEX_OUT_OF_BOUND);
		return RC::INDEX_OUT_OF_BOUND;
	}

	if (index != m_size - 1) {
		size_t nextVecNum = m_size - index - 1;
		memmove(getData(index), getData(index + 1), nextVecNum * vecDataSize());
		memmove(m_hashArr + index, m_hashArr + index + 1, nextVecNum * sizeof(size_t));
	}

	m_size--;
	return RC::SUCCESS;
}

RC Set::remove(IVector const* const& pat, IVector::NORM n, double tol) {
	size_t index;
	RC rc = findFirst(pat, n, tol, index);
	if (rc != RC::SUCCESS) {
		return rc;
	}

	return remove(index);
}

Set::~Set() {
	m_controlBlock->invalidateSet();
	delete[] m_data;
	delete[] m_hashArr;
}

ISet* ISet::createSet() { return Set::createSet(); }

ISet* ISet::makeIntersection(ISet const* const& op1,
							 ISet const* const& op2,
							 IVector::NORM n,
							 double tol) {
	if (!op1 || !op2) {
		log_severe(RC::NULLPTR_ERROR);
		return nullptr;
	}

	ISet* intersection = createSet();
	if (!intersection) {
		return nullptr;
	}

	IVector* vec = VectorUtils::createZeroVec(op1->getDim());
	if (!vec) {
		return nullptr;
	}

	for (size_t i = 0; i < op1->getSize(); i++) {
		op1->getCoords(i, vec);

		if (op2->findFirst(vec, n, tol) == RC::SUCCESS) {

			RC rc = intersection->insert(vec, n, tol);
			if (rc != RC::SUCCESS) {
				delete vec;
				delete intersection;
				return nullptr;
			}
		}
	}

	delete vec;
	return intersection;
}

ISet* ISet::makeUnion(ISet const* const& op1, ISet const* const& op2, IVector::NORM n, double tol) {
	if (!op1 || !op2) {
		log_severe(RC::NULLPTR_ERROR);
		return nullptr;
	}

	ISet* unionSet = op1->clone();
	if (!unionSet) {
		return nullptr;
	}

	IVector* vec = VectorUtils::createZeroVec(op2->getDim());
	if (!vec) {
		return nullptr;
	}

	for (size_t i = 0; i < op2->getSize(); i++) {
		op2->getCoords(i, vec);

		RC rc = unionSet->insert(vec, n, tol);
		if (rc != RC::SUCCESS && rc != RC::VECTOR_ALREADY_EXIST) {
			delete vec;
			delete unionSet;
			return nullptr;
		}
	}

	delete vec;
	return unionSet;
}

ISet* ISet::sub(ISet const* const& op1, ISet const* const& op2, IVector::NORM n, double tol) {
	if (!op1 || !op2) {
		log_severe(RC::NULLPTR_ERROR);
		return nullptr;
	}

	ISet* sub = op1->clone();
	if (!sub) {
		return nullptr;
	}

	IVector* vec = VectorUtils::createZeroVec(op2->getDim());
	if (!vec) {
		return nullptr;
	}

	for (size_t i = 0; i < op2->getSize(); i++) {
		op2->getCoords(i, vec);

		RC rc = sub->remove(vec, n, tol);
		if (rc != RC::SUCCESS && rc != RC::VECTOR_NOT_FOUND) {
			delete vec;
			delete sub;
			return nullptr;
		}
	}

	delete vec;
	return sub;
}

ISet* ISet::symSub(ISet const* const& op1, ISet const* const& op2, IVector::NORM n, double tol) {
	auto unionSet = makeUnion(op1, op2, n, tol);
	auto intersectionSet = makeIntersection(op1, op2, n, tol);
	ISet* symSub = sub(unionSet, intersectionSet, n, tol);

	delete unionSet;
	delete intersectionSet;
	return symSub;
}

bool ISet::equals(ISet const* const& op1, ISet const* const& op2, IVector::NORM n, double tol) {
	ISet* subRes = symSub(op1, op2, n, tol);
	bool res = subRes && !subRes->getSize();
	delete subRes;
	return res;
}

bool ISet::subSet(ISet const* const& op1, ISet const* const& op2, IVector::NORM n, double tol) {
	auto intersectionSet = makeIntersection(op1, op2, n, tol);
	bool res = equals(intersectionSet, op1, n, tol);
	delete intersectionSet;
	return res;
}

ISet* Set::clone() const {
	Set* copy = createSet();
	if (!copy) {
		return copy;
	}

	copy->m_dim = m_dim;
	copy->m_capacity = m_capacity;
	copy->m_topHash = m_topHash;
	copy->m_size = m_size;

	copy->m_data = new (std::nothrow) double[m_dim * m_capacity];
	copy->m_hashArr = new (std::nothrow) size_t[m_capacity];

	if (!copy->m_data || !copy->m_hashArr) {
		log_warning(RC::ALLOCATION_ERROR);
		delete copy;
		return nullptr;
	}

	memcpy(copy->m_data, m_data, vecDataSize() * m_size);
	memcpy(copy->m_hashArr, m_hashArr, sizeof(size_t) * m_size);
	return copy;
}

ISet::~ISet() = default;

RC Set::getNextVec(IVector* vector, size_t& key, size_t inc) {
	auto it = std::upper_bound(m_hashArr, m_hashArr + m_size, key);
	it += inc - 1;

	if (it >= m_hashArr + m_size) {
		return RC::SET_INDEX_OVERFLOW;
	}

	size_t index = it - m_hashArr;
	RC rc = getCoords(index, vector);
	if (rc != RC::SUCCESS) {
		return rc;
	}

	key = m_hashArr[index];
	return RC::SUCCESS;
}

RC Set::getPrevVec(IVector* vector, size_t& key, size_t dec) {
	auto it = std::lower_bound(m_hashArr, m_hashArr + m_size, key);
	it -= dec;

	if (it < m_hashArr) {
		return RC::SET_INDEX_OVERFLOW;
	}

	size_t index = it - m_hashArr;
	RC rc = getCoords(index, vector);
	if (rc != RC::SUCCESS) {
		return rc;
	}

	key = m_hashArr[index];
	return RC::SUCCESS;
}

RC Set::getBeginVec(IVector* vector, size_t& key) {
	RC rc = getCoords(0, vector);
	if (rc != RC::SUCCESS) {
		return rc;
	}

	key = m_hashArr[0];
	return RC::SUCCESS;
}

RC Set::getEndVec(IVector* vector, size_t& key) {
	RC rc = getCoords(m_size - 1, vector);
	if (rc != RC::SUCCESS) {
		return rc;
	}

	key = m_hashArr[m_size - 1];
	return RC::SUCCESS;
}

Set* Set::createSet() {
	auto set = new (std::nothrow) Set();
	if (!set) {
		log_warning(RC::ALLOCATION_ERROR);
		return nullptr;
	}

	auto controlBlock = new (std::nothrow) SetControlBlock(set);
	if (!controlBlock) {
		log_warning(RC::ALLOCATION_ERROR);
		delete set;
		return nullptr;
	}

	set->m_controlBlock.reset(controlBlock);
	return set;
}

ISet::IIterator* Set::getIterator(size_t index) const {
	if (index >= m_size) {
		log_warning(RC::INDEX_OUT_OF_BOUND);
	}

	IVector* vec;
	if (getCopy(index, vec) != RC::SUCCESS) {
		return nullptr;
	}

	auto iterator = new (std::nothrow) Set::Iterator(m_controlBlock, vec, m_hashArr[index]);
	if (!iterator) {
		log_warning(RC::ALLOCATION_ERROR);
		delete vec;
		return nullptr;
	}
	return iterator;
}

ISet::IIterator* Set::getBegin() const { return getIterator(0); }

ISet::IIterator* Set::getEnd() const { return getIterator(m_size - 1); }
