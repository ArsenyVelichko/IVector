#include <cstdlib>
#include <cmath>

#include "Solver.h"
#include "VectorUtils.h"

Solver* Solver::createSolver() {
	return new (std::nothrow) Solver();
}

ISolver* ISolver::createSolver() {
	return Solver::createSolver();
}

RC ISolver::setLogger(ILogger* const logger) {
	return LogUtils::LogContainer<Solver>::setInstance(logger);
}

ILogger* ISolver::getLogger() {
	return LogUtils::LogContainer<Solver>::getInstance();
}

ISolver* Solver::clone() const {
	Solver* clone = createSolver();
	if (!clone) {
		log_warning(RC::ALLOCATION_ERROR);
		return nullptr;
	}

	if (m_problem && clone->setProblem(m_problem) != RC::SUCCESS ||
		m_solution && clone->setSolution(m_solution) != RC::SUCCESS ||
		m_argsDomain && clone->setArgsDomain(m_argsDomain) != RC::SUCCESS ||
		m_paramsDomain && clone->setParamsDomain(m_paramsDomain) != RC::SUCCESS) {

		delete clone;
		return nullptr;
	}
	return clone;
}

RC Solver::setProblem(IDiffProblem const* const& pProblem) {
	m_problem = pProblem->clone();

	if (!m_problem) {
		return RC::ALLOCATION_ERROR;
	}
	return RC::SUCCESS;
}

bool Solver::isValidDomain(const ICompact* compact, IsValidMethod isValid) const {
	if (!m_problem) {
		log_warning(RC::NO_PROBLEM_SET);
		return false;
	}

	IVector* leftBound = nullptr;
	IVector* rightBound = nullptr;

	if (compact->getLeftBoundary(leftBound) != RC::SUCCESS ||
		compact->getRightBoundary(rightBound) != RC::SUCCESS) {

		delete leftBound;
		delete rightBound;
		return false;
	}

	bool isValidFlag = (m_problem->*isValid)(leftBound) &&
					   (m_problem->*isValid)(rightBound);

	delete rightBound;
	delete leftBound;
	return isValidFlag;
}

bool Solver::isValidArgsDomain(ICompact const* const& args) const {
	return isValidDomain(args, &IProblem::isValidArgs);
}

bool Solver::isValidParamsDomain(ICompact const* const& params) const {
	return isValidDomain(params, &IProblem::isValidParams);
}

RC Solver::setArgsDomain(ICompact const* const& args, ILogger* logger) {
	if (logger) {
		setLogger(logger);
	}

	if (!isValidArgsDomain(args)) {
		log_warning(RC::INVALID_ARGUMENT);
		return RC::INVALID_ARGUMENT;
	}

	auto clone = args->clone();
	if (!clone) {
		return RC::ALLOCATION_ERROR;
	}

	delete m_solution;
	m_solution = nullptr;

	delete m_argsDomain;
	m_argsDomain = clone;
	return RC::SUCCESS;
}

RC Solver::setParamsDomain(ICompact const* const& params) {
	if (!isValidParamsDomain(params)) {
		log_warning(RC::INVALID_ARGUMENT);
		return RC::INVALID_ARGUMENT;
	}

	auto clone = params->clone();
	if (!clone) {
		return RC::ALLOCATION_ERROR;
	}

	delete m_solution;
	m_solution = nullptr;

	delete m_paramsDomain;
	m_paramsDomain = clone;
	return RC::SUCCESS;
}

RC Solver::solveByArgs(IVector const* const& initArg, IVector const* const& solverParams) {
	if (!m_argsDomain) {
		log_warning(RC::NO_ARGS_SET);
		return RC::NO_ARGS_SET;
	}

	if (!m_problem) {
		log_warning(RC::NO_PROBLEM_SET);
		return RC::NO_PROBLEM_SET;
	}

	if (!m_argsDomain->isInside(initArg) || !isValidSolverParams(solverParams)) {
		log_warning(RC::INVALID_ARGUMENT);
		return RC::INVALID_ARGUMENT;
	}

	auto paramsData = solverParams->getData();
	return splitStepMethod(initArg,
						   paramsData[PRECISION],
						   paramsData[INITIAL_STEP],
						   paramsData[SPLIT_COEFFICIENT],
						   paramsData[DELTA]);
}


RC Solver::solveByParams(IVector const* const& initParam, IVector const* const& solverParams) {
	log_severe(RC::UNKNOWN);
	return RC::UNKNOWN;
}

RC Solver::setSolution(const IVector* solution) {
	auto clone = solution->clone();
	if (!clone) {
		return RC::ALLOCATION_ERROR;
	}

	delete m_solution;
	m_solution = clone;
	return RC::SUCCESS;
}

RC Solver::getSolution(IVector*& solution) const {
	if (!m_solution) {
		return RC::NULLPTR_ERROR;
	}

	auto clone = m_solution->clone();
	if (!clone) {
		return RC::ALLOCATION_ERROR;
	}

	solution = clone;
	return RC::SUCCESS;
}

Solver::~Solver() {
	delete m_problem;
	delete m_argsDomain;
	delete m_paramsDomain;
	delete m_solution;
}

RC Solver::splitStepMethod(const IVector* initApprox,
						   double precision,
						   double initialStep,
						   double splitCoefficient,
						   double delta) {
	RC rc = RC::SUCCESS;

	IVector* currApprox = initApprox->clone();
	IVector* nextApprox = VectorUtils::createZeroVec(initApprox->getDim());
	IVector* grad = VectorUtils::createZeroVec(initApprox->getDim());

	if (!currApprox || !grad || !nextApprox) {
		rc = RC::ALLOCATION_ERROR;
	}

	for (size_t k = 0; rc == RC::SUCCESS; k++) {

		rc = m_problem->evalGradientByArgs(currApprox, grad);
		if (rc != RC::SUCCESS) { break; }

		double gradNorm = grad->norm(IVector::NORM::SECOND);
		if (gradNorm < precision) { break; }

		if (k == MAX_ITER_NUMBER) {
			rc = RC::NOT_NUMBER;
			break;
		}

		double currValue = m_problem->evalByArgs(currApprox);
		if (std::isnan(currValue)) {
			rc = RC::NOT_NUMBER;
			break;
		}

		double step = initialStep;
		double coeff = -delta * gradNorm * gradNorm;

		while (step > 0.0) {
			IVector::copyInstance(nextApprox, grad);
			nextApprox->scale(-step);

			rc = nextApprox->inc(currApprox);
			if (rc != RC::SUCCESS) { break; }

			double nextValue = m_problem->evalByArgs(nextApprox);
			if (std::isnan(nextValue)) {
				rc = RC::NOT_NUMBER;
				break;
			}

			if (nextValue - currValue < coeff * step) { break; }
			step *= splitCoefficient;
		}
		std::swap(currApprox, nextApprox);
	}

	if (rc == RC::SUCCESS) {
		setSolution(currApprox);
	}

	delete currApprox;
	delete nextApprox;
	delete grad;

	return rc;
}

bool Solver::isValidSolverParams(const IVector* params) {
	if (params->getDim() != PARAMS_NUMBER) { return false; }

	auto paramsData = params->getData();
	if (paramsData[PRECISION] <= 0) { return false; }

	for (size_t i = INITIAL_STEP; i < PARAMS_NUMBER; i++) {
		if (paramsData[i] <= 0 || paramsData[i] >= 1) {
			return false;
		}
	}
	return true;
}

ISolver::~ISolver() = default;
