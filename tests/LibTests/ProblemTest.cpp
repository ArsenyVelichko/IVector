#include <iostream>

#include <vector>
#include <cassert>

#include <IDiffProblem.h>

#include "Tests.h"
#include "DllLoader.h"
#include "PrintUtils.h"

void Tests::problemTest(ILogger* logger) {
	auto impl = IBroker::INTERFACE_IMPL::IPROBLEM;

	DllLoader problemLoader;
	assert(problemLoader.loadLibrary("libProblem"));
	auto problem = reinterpret_cast<IDiffProblem*>(problemLoader.loadImplementation(impl));

	double tol = 1.0e-10;
	size_t argDim = 2;

	std::vector<double> leftBoundData = { -20, -20 };
	std::vector<double> rightBoundData = { 20, 20 };
	std::vector<size_t> gridData = { 100, 100 };

	IVector* argsLeftBound = IVector::createVector(argDim, leftBoundData.data());
	IVector* argsRightBound = IVector::createVector(argDim, rightBoundData.data());
	IMultiIndex* argsGrid = IMultiIndex::createMultiIndex(argDim, gridData.data());

	ICompact* argsDomain = ICompact::createCompact(argsLeftBound, argsRightBound, argsGrid);

	delete argsLeftBound;
	delete argsRightBound;
	delete argsGrid;

	size_t paramDim = 4;

	leftBoundData = { -10, -10, -10, -10 };
	rightBoundData = { 10, 10, 10, 10 };
	gridData = { 50, 50, 50, 50 };

	IVector* paramsLeftBound = IVector::createVector(paramDim, leftBoundData.data());
	IVector* paramsRightBound = IVector::createVector(paramDim, rightBoundData.data());
	IMultiIndex* paramsGrid = IMultiIndex::createMultiIndex(paramDim, gridData.data());

	ICompact* paramsDomain = ICompact::createCompact(paramsLeftBound, paramsRightBound, paramsGrid);

	delete paramsLeftBound;
	delete paramsRightBound;
	delete paramsGrid;

	problem->setArgsDomain(argsDomain, logger);
	problem->setParamsDomain(paramsDomain);

	delete argsDomain;
	delete paramsDomain;

	auto argPoint = IVector::createVector(argDim, std::vector<double>{ -0.5, -2 }.data());
	std::cout << "Arguments values: ";
	PrintUtils::printVector(argPoint);

	auto paramPoint = IVector::createVector(paramDim, std::vector<double>{ 3, -1, 7, -9 }.data());
	std::cout << "Parameters values: ";
	PrintUtils::printVector(paramPoint);

	double funcValue = 3.0 / 16.0 - 4.0 + 7.0 * sin(4.5);

	problem->setArgs(argPoint);
	double val = problem->evalByParams(paramPoint);
	std::cout << "Evaluate by params result: " << val << std::endl;
	assert(Tests::fuzzyCompare(funcValue, val, tol));

	problem->setParams(paramPoint);

	val = problem->evalByArgs(argPoint);
	std::cout << "Evaluate by args result: " << val << std::endl;
	assert(Tests::fuzzyCompare(funcValue, val, tol));

	IMultiIndex* argDerivOrder = IMultiIndex::createMultiIndex(argDim, std::vector<size_t>{ 0, 0 }.data());

	auto argDerivTest = [&] (double correctRes) {
		std::cout << "Arguments derivative order: ";
		PrintUtils::printMultiIndex(argDerivOrder);

		double val = problem->evalDerivativeByArgs(argPoint, argDerivOrder);
		std::cout << "Derivative by args result: " << val << std::endl;
		assert(Tests::fuzzyCompare(correctRes, val, tol));
	};

	argDerivTest(funcValue);

	argDerivOrder->setAxisIndex(1, 1);
	argDerivTest(4.0);

	argDerivOrder->setAxisIndex(1, 3);
	argDerivTest(0.0);

	argDerivOrder->setAxisIndex(0, 1);
	argDerivOrder->setAxisIndex(1, 1);
	argDerivTest(0.0);

	argDerivOrder->setAxisIndex(0, 5);
	argDerivOrder->setAxisIndex(1, 0);

	double correctRes = -7 * pow(9.0, 5) * cos(4.5);
	argDerivTest(correctRes);

	delete argDerivOrder;

	IMultiIndex* paramDerivOrder = IMultiIndex::createMultiIndex(paramDim, std::vector<size_t>{ 0, 0, 0, 0 }.data());

	auto paramDerivTest = [&] (double correctRes) {
		std::cout << "Parameters derivative order: ";
		PrintUtils::printMultiIndex(paramDerivOrder);

		double val = problem->evalDerivativeByParams(paramPoint, paramDerivOrder);
		std::cout << "Derivative by params result: " << val << std::endl;
		assert(Tests::fuzzyCompare(correctRes, val, tol));
	};

	paramDerivTest(funcValue);

	paramDerivOrder->setAxisIndex(1, 1);
	paramDerivTest(4.0);

	paramDerivOrder->setAxisIndex(2, 1);
	paramDerivTest(0.0);

	paramDerivOrder->setAxisIndex(0, 2);
	paramDerivOrder->setAxisIndex(1, 0);
	paramDerivTest(0.0);

	paramDerivOrder->setAxisIndex(0, 0);
	paramDerivOrder->setAxisIndex(3, 3);

	correctRes = -pow(-0.5, 3) * cos(4.5);
	paramDerivTest(correctRes);

	delete paramDerivOrder;

	IVector* argGradient = IVector::createVector(argDim, std::vector<double>(argDim).data());
	problem->evalGradientByArgs(argPoint, argGradient);
	std::cout << "Gradient by args: ";
	PrintUtils::printVector(argGradient);

	std::vector<double> correctGradData = { -1.5 - 63 * cos(4.5), 4 };
	IVector* correctGrad = IVector::createVector(argDim, correctGradData.data());
	assert(IVector::equals(argGradient, correctGrad, IVector::NORM::SECOND, tol));

	delete argGradient;
	delete correctGrad;

	IVector* paramsGradient = IVector::createVector(paramDim, std::vector<double>(paramDim).data());
	problem->evalGradientByParams(paramPoint, paramsGradient);
	std::cout << "Gradient by params: ";
	PrintUtils::printVector(paramsGradient);

	correctGradData = { 1.0 / 16.0, 4.0, sin(4.5), -3.5 * cos(4.5) };
	correctGrad = IVector::createVector(paramDim, correctGradData.data());
	assert(IVector::equals(paramsGradient, correctGrad, IVector::NORM::SECOND, tol));

	delete paramsGradient;
	delete correctGrad;

	delete argPoint;
	delete paramPoint;

	delete problem;

	std::cout << "Problem test successfully finished\n\n";
}