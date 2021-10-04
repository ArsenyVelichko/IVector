#pragma once
#include <IVector.h>

#include "LogUtils.h"

using LogUtils::LogContainer;

namespace {

	class Vector : public IVector, public LogContainer<Vector> {
	public:
		IVector* clone() const override;
		double const* getData() const override;
		RC setData(size_t dim, double const* const& data) override;

		RC getCoord(size_t index, double& val) const override;
		RC setCoord(size_t index, double val) override;
		RC scale(double multiplier) override;
		size_t getDim() const override;

		RC inc(IVector const* const& op) override;
		RC dec(IVector const* const& op) override;

		double norm(NORM n) const override;

		RC applyFunction(const std::function<double(double)>& fun) override;
		RC foreach (const std::function<void(double)>& fun) const override;

		size_t sizeAllocated() const override;

		static Vector* createVector(size_t dim, const double* const& data);

	private:
		explicit Vector(size_t dim);

		double* getData();

		double infiniteNorm() const;
		double firstNorm() const;
		double secondNorm() const;

		size_t m_dim;
	};

} // namespace
