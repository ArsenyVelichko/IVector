#include <cmath>

#include "DiffProblem.h"

const int DiffProblem::m_argsPows[ARGS_NUM] = { 4, 2 };

RC IProblem::setLogger(ILogger* const logger) {
	return LogContainer<DiffProblem>::setInstance(logger);
}

RC IDiffProblem::setLogger(ILogger* const logger) {
	return LogContainer<DiffProblem>::setInstance(logger);
}

ILogger* IProblem::getLogger() {
	return LogContainer<DiffProblem>::getInstance();
}

ILogger* IDiffProblem::getLogger() {
	return LogContainer<DiffProblem>::getInstance();
}

RC DiffProblem::setArgsDomain(ICompact const* const& args, ILogger* logger) {
	if (logger) {
		setLogger(logger);
	}

	if (args->getDim() != ARGS_NUM) {
		log_warning(RC::MISMATCHING_DIMENSIONS);
		return RC::MISMATCHING_DIMENSIONS;
	}

	if (m_args && !args->isInside(m_args)) {
		delete m_args;
		m_args = nullptr;
	}

	auto clone = args->clone();
	if (!clone) {
		return RC::ALLOCATION_ERROR;
	}

	delete m_argsDomain;
	m_argsDomain = clone;
	return RC::SUCCESS;
}

RC DiffProblem::setParamsDomain(ICompact const* const& params) {
	if (params->getDim() != PARAMS_NUM) {
		log_warning(RC::MISMATCHING_DIMENSIONS);
		return RC::MISMATCHING_DIMENSIONS;
	}

	if (m_params && !params->isInside(m_params)) {
		delete m_params;
		m_params = nullptr;
	}

	auto clone = params->clone();
	if (!clone) {
		return RC::ALLOCATION_ERROR;
	}

	delete m_paramsDomain;
	m_paramsDomain = clone;
	return RC::SUCCESS;
}

IDiffProblem* IDiffProblem::createDiffProblem() {
	return DiffProblem::createProblem();
}

IProblem* IProblem::createProblem() {
	return DiffProblem::createProblem();
}

IDiffProblem* DiffProblem::clone() const {
	IDiffProblem* clone = createProblem();

	if (!clone) {
		log_warning(RC::ALLOCATION_ERROR);
		return nullptr;
	}

	if (m_paramsDomain && clone->setParamsDomain(m_paramsDomain) != RC::SUCCESS ||
		m_params && clone->setParams(m_params) != RC::SUCCESS ||
		m_argsDomain && clone->setArgsDomain(m_argsDomain) != RC::SUCCESS ||
		m_args && clone->setArgs(m_args) != RC::SUCCESS) {

		delete clone;
		return nullptr;
	}
	return clone;
}

bool DiffProblem::isValidParams(IVector const* const& params) const {
	if (!m_paramsDomain) {
		log_warning(RC::NO_PARAMS_SET);
		return false;
	}
	return m_paramsDomain->isInside(params);
}

bool DiffProblem::isValidArgs(IVector const* const& args) const {
	if (!m_argsDomain) {
		log_warning(RC::NO_ARGS_SET);
		return false;
	}
	return m_argsDomain->isInside(args);
}

RC DiffProblem::setParams(IVector const* const& params) {
	if (!isValidParams(params)) {
		log_warning(RC::INVALID_ARGUMENT);
		return RC::INVALID_ARGUMENT;
	}

	auto clone = params->clone();
	if (!clone) {
		return RC::ALLOCATION_ERROR;
	}

	delete m_params;
	m_params = clone;
	return RC::SUCCESS;
}

RC DiffProblem::setArgs(IVector const* const& args) {
	if (!isValidArgs(args)) {
		log_warning(RC::INVALID_ARGUMENT);
		return RC::INVALID_ARGUMENT;
	}

	auto clone = args->clone();
	if (!clone) {
		return RC::ALLOCATION_ERROR;
	}

	delete m_args;
	m_args = clone;
	return RC::SUCCESS;
}

double DiffProblem::eval(const IVector* params, const IVector* args) {
	auto paramsData = params->getData();
	auto argsData = args->getData();

	return paramsData[A] * pow(argsData[X], m_argsPows[X]) +
		   paramsData[B] * pow(argsData[Y], m_argsPows[Y]) +
		   paramsData[C] * sin(paramsData[D] * argsData[X]);
}

double DiffProblem::evalByParams(IVector const* const& params) const {

	if (!m_args) {
		log_warning(RC::NO_ARGS_SET);
		return NAN;
	}

	if (!isValidParams(params)) {
		log_warning(RC::INVALID_ARGUMENT);
		return NAN;
	}

	return eval(params, m_args);
}

double DiffProblem::evalByArgs(IVector const* const& args) const {

	if (!m_params) {
		log_warning(RC::NO_PARAMS_SET);
		return NAN;
	}

	if (!isValidArgs(args)) {
		log_warning(RC::INVALID_ARGUMENT);
		return NAN;
	}

	return eval(m_params, args);
}

double DiffProblem::evalDerivativeByArgs(IVector const* const& args, IMultiIndex const* const& index) const {

	if (index->getDim() != ARGS_NUM) {
		log_warning(RC::MISMATCHING_DIMENSIONS);
		return NAN;
	}

	if (!isValidArgs(args)) {
		log_warning(RC::INVALID_ARGUMENT);
		return NAN;
	}

	if (!m_params) {
		log_warning(RC::NO_PARAMS_SET);
		return NAN;
	}

	auto argsData = args->getData();
	auto paramsData = m_params->getData();
	auto indexData = index->getData();

	auto singleDeriv = [&] (Args arg) {
		if (indexData[arg] > m_argsPows[arg]) { return 0.0; }

		size_t argPow = m_argsPows[arg] - indexData[arg];
		double multiplier = intervalFactorial(argPow + 1, m_argsPows[arg]);
		return paramsData[arg] * multiplier * pow(argsData[arg], argPow);
	};

	bool notNullFound = false;
	size_t notNullIdx;
	for (size_t i = 0; i < ARGS_NUM; i++) {
		if (indexData[i] > 0) {
			if (notNullFound) { return 0.0; }

			notNullIdx = i;
			notNullFound = true;
		}
	}

	if (!notNullFound) { return evalByArgs(args); }

	if (notNullIdx == Y) { return singleDeriv(Y); }

	double singleDerivX = singleDeriv(X);
	double argMul = paramsData[D] * argsData[X];

	double thirdTerm = paramsData[C] * pow(paramsData[D], indexData[X]);
	thirdTerm *= indexData[X] % 2 ? cos(argMul) : sin(argMul);
	thirdTerm *= pow(-1, indexData[X] / 2);

	return singleDerivX + thirdTerm;
}

double DiffProblem::evalDerivativeByParams(IVector const* const& params, IMultiIndex const* const& index) const {

	if (index->getDim() != PARAMS_NUM) {
		log_warning(RC::MISMATCHING_DIMENSIONS);
	}

	if (!isValidParams(params)) {
		log_warning(RC::INVALID_ARGUMENT);
		return NAN;
	}

	if (!m_args) {
		log_warning(RC::NO_ARGS_SET);
		return NAN;
	}

	auto argsData = m_args->getData();
	auto paramsData = params->getData();
	auto indexData = index->getData();

	bool notNullFound = false;
	size_t notNullIdx;
	for (size_t i = 0; i < PARAMS_NUM - 1; i++) {
		if (indexData[i] > 1) { return 0.0; }

		if (indexData[i] > 0) {
			if (notNullFound) { return 0.0; }

			notNullFound = true;
			notNullIdx = i;
		}
	}

	if (!notNullFound) { return evalByParams(params); }

	if (notNullIdx != C) {
		if (indexData[D]) { return 0.0; }

		return pow(argsData[notNullIdx], m_argsPows[notNullIdx]);
	}

	size_t paramPow = 1 - indexData[C];
	double thirdTerm = pow(paramsData[C], paramPow);
	double argMul = argsData[X] * paramsData[D];

	thirdTerm *= pow(argsData[X], indexData[D]);
	thirdTerm *= indexData[D] % 2 ? cos(argMul) : sin(argMul);
	thirdTerm *= pow(-1, indexData[D] / 2);

	return thirdTerm;
}

RC DiffProblem::evalGradientByArgs(IVector const* const& args, IVector* const& val) const {

	if (val->getDim() != ARGS_NUM) {
		log_warning(RC::MISMATCHING_DIMENSIONS);
		return RC::MISMATCHING_DIMENSIONS;
	}

	if (!isValidArgs(args)) {
		log_warning(RC::INVALID_ARGUMENT);
		return RC::INVALID_ARGUMENT;
	}

	if (!m_params) {
		log_warning(RC::NO_PARAMS_SET);
		return RC::NO_PARAMS_SET;
	}

	auto argsData = args->getData();
	auto paramsData = m_params->getData();

	double firstCoord = m_argsPows[X] * paramsData[A] * pow(argsData[X], m_argsPows[X] - 1) +
						paramsData[C] * paramsData[D] * cos(paramsData[D] * argsData[X]);

	double secondCoord = m_argsPows[Y] * paramsData[B] * pow(argsData[Y], m_argsPows[Y] - 1);

	val->setCoord(X, firstCoord);
	val->setCoord(Y, secondCoord);
	return RC::SUCCESS;
}

RC DiffProblem::evalGradientByParams(IVector const* const& params, IVector* const& val) const {

	if (val->getDim() != PARAMS_NUM) {
		log_warning(RC::MISMATCHING_DIMENSIONS);
	}

	if (!isValidParams(params)) {
		log_warning(RC::INVALID_ARGUMENT);
		return RC::INVALID_ARGUMENT;
	}

	if (!m_args) {
		log_warning(RC::NO_ARGS_SET);
		return RC::NO_ARGS_SET;
	}

	auto argsData = m_args->getData();
	auto paramsData = params->getData();

	double x = argsData[X];
	double y = argsData[Y];

	val->setCoord(A, pow(x, m_argsPows[X]));
	val->setCoord(B, pow(y, m_argsPows[Y]));
	val->setCoord(C, sin(paramsData[D] * x));
	val->setCoord(D, paramsData[C] * x * cos(paramsData[D] * x));

	return RC::SUCCESS;
}

DiffProblem::~DiffProblem() {
	delete m_paramsDomain;
	delete m_argsDomain;
	delete m_params;
	delete m_args;
}

DiffProblem* DiffProblem::createProblem() {
	return new (std::nothrow) DiffProblem;
}

size_t DiffProblem::intervalFactorial(size_t min, size_t max) {
	size_t fact = 1;
	for (size_t i = min; i <= max; i++) {
		fact *= i;
	}
	return fact;
}

IDiffProblem::~IDiffProblem() = default;

IProblem::~IProblem() = default;
