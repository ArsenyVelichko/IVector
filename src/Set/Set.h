#pragma once

#include <memory>

#include <ISet.h>
#include <ISetControlBlock.h>

#include "LogProducer.h"

class SetControlBlock;

class Set : public ISet, public LogProducer<Set> {
public:
	ISet* clone() const override;

	size_t getDim() const override;
	size_t getSize() const override;
	RC getCopy(size_t index, IVector*& val) const override;
	RC findFirstAndCopy(IVector const* const& pat,
						IVector::NORM n,
						double tol,
						IVector*& val) const override;

	RC getCoords(size_t index, IVector* const& val) const override;
	RC findFirstAndCopyCoords(IVector const* const& pat,
							  IVector::NORM n,
							  double tol,
							  IVector* const& val) const override;

	RC findFirst(const IVector* const& pat, IVector::NORM n, double tol) const override;

	RC insert(IVector const* const& val, IVector::NORM n, double tol) override;

	RC remove(size_t index) override;
	RC remove(IVector const* const& pat, IVector::NORM n, double tol) override;

	class Iterator : public ISet::IIterator, public LogProducer<Iterator> {
	public:
		Iterator(const std::shared_ptr<SetControlBlock>& controlBlock,
				 IVector* vector,
				 size_t hash);

		IIterator* getNext(size_t indexInc = 1) const override;
		IIterator* getPrevious(size_t indexInc = 1) const override;
		IIterator* clone() const override;

		RC next(size_t indexInc = 1) override;
		RC previous(size_t indexInc = 1) override;

		bool isValid() const override;

		RC makeBegin() override;
		RC makeEnd() override;

		RC getVectorCopy(IVector*& val) const override;
		RC getVectorCoords(IVector* const& val) const override;

		~Iterator() override;

	private:
		IVector* m_vector;
		size_t m_hash;
		std::shared_ptr<SetControlBlock> m_controlBlock;

		bool m_isValid = true;
	};

	IIterator* getIterator(size_t index) const override;
	IIterator* getBegin() const override;
	IIterator* getEnd() const override;

	RC getNextVec(IVector* vector, size_t& key, size_t inc);
	RC getPrevVec(IVector* vector, size_t& key, size_t dec);
	RC getBeginVec(IVector* vector, size_t& key);
	RC getEndVec(IVector* vector, size_t& key);

	static Set* createSet();
	static IVector* createZeroVec(size_t dim);

	~Set();

private:
	double* m_data = nullptr;
	size_t* m_hashArr = nullptr;
	size_t m_topHash = 0;

	size_t m_dim = 0;
	size_t m_capacity = 0;
	size_t m_size = 0;

	std::shared_ptr<SetControlBlock> m_controlBlock;

	size_t vecDataSize() const;

	RC findFirst(IVector const* pat, IVector::NORM n, double tol, size_t& index) const;
	double* getData(size_t index) const;

	bool enlarge();
};
