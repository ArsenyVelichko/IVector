#pragma once
#include <IVector.h>

#include "LogProducer.h"

constexpr int foo() { return 0; }

namespace {

	class Vector : public IVector, public LogProducer<Vector> {
	public:
		IVector* clone() const override;
		double const* getData() const override;
		RC setData(size_t dim, double const* const& data) override;

		RC getCord(size_t index, double& val) const override;
		RC setCord(size_t index, double val) override;
		RC scale(double multiplier) override;
		size_t getDim() const override;

		RC inc(IVector const* const& op) override;
		RC dec(IVector const* const& op) override;

		double norm(NORM n) const override;

		RC applyFunction(const std::function<double(double)>& fun) override;
		RC foreach (const std::function<void(double)>& fun) const override;

		size_t sizeAllocated() const override;

		~Vector();

		static Vector* createVector(size_t dim, const double* const& data);

	private:
		Vector(size_t dim);

		double* getData();

		double infiniteNorm() const;
		double firstNorm() const;
		double secondNorm() const;

		size_t m_dim;
	};

} // namespace
