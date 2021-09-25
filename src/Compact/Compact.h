#pragma once

#include <memory>

#include <ICompact.h>

#include "LogProducer.h"

class CompactControlBlock;

class Compact : public ICompact, public LogProducer<Compact> {
public:
	static Compact* createCompact(IVector const* vec1,
								  IVector const* vec2,
								  IMultiIndex const* nodeQuantities);

	ICompact* clone() const override;

	size_t getDim() const override;
	IMultiIndex* getGrid() const override;

	bool isInside(IVector const* const& vec) const override;

	RC getVectorCopy(IMultiIndex const* index, IVector*& val) const override;
	RC getVectorCoords(IMultiIndex const* index, IVector* const& val) const override;

	RC getLeftBoundary(IVector*& vec) const override;
	RC getRightBoundary(IVector*& vec) const override;

	class Iterator : public ICompact::IIterator, public LogProducer<Iterator> {
	public:
		Iterator(std::shared_ptr<CompactControlBlock> const& block,
				 IMultiIndex* startPos,
				 IMultiIndex* byPass,
				 IVector* vector);

		bool isValid() const override;

		IIterator* getNext() override;
		IIterator* clone() const override;

		RC next() override;

		RC getVectorCopy(IVector*& val) const override;
		RC getVectorCoords(IVector* const& val) const override;

		~Iterator() override;

	private:
		IMultiIndex* m_order;
		IMultiIndex* m_place;
		IVector* m_vector;
		std::shared_ptr<CompactControlBlock> m_controlBlock;

		mutable bool m_placeChanged = false;
		bool m_isValid = true;
	};

	IIterator* getIterator(IMultiIndex const* const& index,
						   IMultiIndex const* const& bypassOrder) const override;

	IIterator* getBegin(IMultiIndex const* const& bypassOrder) const override;
	IIterator* getEnd(IMultiIndex const* const& bypassOrder) const override;

	RC advance(IMultiIndex* const& place, IMultiIndex const* const& bypassOrder);

	~Compact() override;

private:
	Compact(IVector* vec1, IVector* vec2, IMultiIndex* nodeQuantities);

	IVector* m_lowerBound;
	IVector* m_upperBound;
	IMultiIndex* m_nodeQuantities;
	std::shared_ptr<CompactControlBlock> m_controlBlock;
};
