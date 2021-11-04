#pragma once

#include <ISolver.h>

#include "LogUtils.h"

using LogUtils::LogContainer;

class Solver: public ISolver, public LogContainer<Solver> {
public:
	static Solver* createSolver();

	ISolver* clone() const override;

	RC setProblem(IDiffProblem const* const& problem) override;

	bool isValidArgsDomain(ICompact const* const& args) const override;
	bool isValidParamsDomain(ICompact const* const& params) const override;

	RC setArgsDomain(ICompact const* const& args, ILogger* logger = nullptr) override;
	RC setParamsDomain(ICompact const* const& params) override;

	RC solveByArgs(IVector const* const& initArg, IVector const* const& solverParams) override;
	RC solveByParams(IVector const* const& initParam, IVector const* const& solverParams) override;
	RC getSolution(IVector*& solution) const override;

	~Solver() override;

private:
	RC setSolution(const IVector* solution);

	using IsValidMethod = bool (IProblem::*)(IVector const* const&) const;
	bool isValidDomain(ICompact const* compact, IsValidMethod isValid) const;

	enum MethodParams {
		PRECISION,

		// All parameters below must have values between 0 and 1
		INITIAL_STEP,
		SPLIT_COEFFICIENT,
		DELTA, //Constant from split condition f(x_(k+1)) - f(x_k) <= -DELTA * step * ||grad(f(x))||^2

		PARAMS_NUMBER
	};

	static bool isValidSolverParams(IVector const* params);

	RC splitStepMethod(IVector const* initApprox,
					   double precision,
					   double initialStep,
					   double splitCoefficient,
					   double delta);

	static constexpr size_t MAX_ITER_NUMBER = 5000;

	IVector* m_solution = nullptr;
	IDiffProblem* m_problem = nullptr;
	ICompact* m_argsDomain = nullptr;
	ICompact* m_paramsDomain = nullptr;
};
