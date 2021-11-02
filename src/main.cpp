#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>
#include <vector>
#include <random>

#include <ICompact.h>
#include <ISet.h>
#include <IVector.h>

#include <IBroker.h>
#include <IDiffProblem.h>

#include "LibUtils.h"

using namespace std;

void printVector(IVector* vector) {
	function<void(double)> print = [](double x) { cout << x << ' '; };
	cout << "( ";
	vector->foreach (print);
	cout << ")" << endl;
}

void printMultiIndex(IMultiIndex* index) {
	auto indexData = index->getData();
	cout << "( ";
	for (size_t i = 0; i < index->getDim(); i++) {
		cout << indexData[i] << ' ';
	}
	cout << ")" << endl;
}

bool fuzzyCompare(double x, double y, double tol) { return fabs(x - y) < tol; }

void vectorTest(ILogger* logger) {
	IVector::setLogger(logger);

	double tol = 1.0e-10;
	size_t dim = 3;

	IVector* vec1 = IVector::createVector(dim, vector<double>{1, 2, 3}.data());
	IVector* vec2 = IVector::createVector(dim, vector<double>{-1, -2, -3}.data());

	cout << "Vector A: ";
	printVector(vec1);

	cout << "Vector B: ";
	printVector(vec2);

	IVector* subRes = IVector::sub(vec1, vec2);
	IVector* correctSubRes = IVector::createVector(dim, vector<double>{2, 4, 6}.data());
	cout << "Subtraction: ";
	printVector(subRes);
	assert(IVector::equals(subRes, correctSubRes, IVector::NORM::SECOND, tol));

	delete subRes;
	delete correctSubRes;

	double dotProduct = IVector::dot(vec1, vec2);
	cout << "Dot product: " << dotProduct << endl;
	assert(fuzzyCompare(dotProduct, -14, tol));

	double firstNorm = vec1->norm(IVector::NORM::FIRST);
	cout << "First norm of Vector A: " << firstNorm << endl;
	assert(fuzzyCompare(firstNorm, 6, tol));

	double infiniteNorm = vec2->norm(IVector::NORM::CHEBYSHEV);
	cout << "Infinite norm of Vector B: " << infiniteNorm << endl;
	assert(fuzzyCompare(infiniteNorm, 3, tol));

	cout << "Setting coord 1 of Vector B to -7 :";
	vec2->setCoord(1, -7);
	IVector* correctSetRes = IVector::createVector(dim, vector<double>{-1, -7, -3}.data());
	printVector(vec2);
	assert(IVector::equals(vec2, correctSetRes, IVector::NORM::SECOND, tol));

	delete correctSetRes;

	vec1->inc(vec2);
	IVector* correctIncRes = IVector::createVector(dim, vector<double>{0, -5, 0}.data());
	cout << "Incrementing Vector A with Vector B: ";
	printVector(vec1);
	assert(IVector::equals(vec1, correctIncRes, IVector::NORM::SECOND, tol));

	delete correctIncRes;

	cout << "Copying Vector A to Vector B: ";
	IVector::copyInstance(vec2, vec1);
	printVector(vec2);
	assert(IVector::equals(vec2, vec1, IVector::NORM::SECOND, tol));

	cout << "Moving Vector A to Vector B: ";
	IVector::moveInstance(vec2, vec1);
	printVector(vec2);
	assert(vec1 == nullptr);

	double inf = std::numeric_limits<double>::infinity();
	cout << "Trying set infinity coord value in Vector B: ";
	RC rc = vec2->setCoord(1, inf);
	printVector(vec2);
	assert(rc != RC::SUCCESS);

	delete vec2;

	cout << "Vector test successfully finished\n\n";
}

void printSet(ISet* set) {
	if (set->getSize() == 0) {
		cout << "[ empty ]" << endl;
		return;
	}

	IVector* vec;

	cout << "[ " << endl;

	auto it = set->getBegin();
	for (; it->isValid(); it->next()) {
		it->getVectorCopy(vec);
		printVector(vec);
		delete vec;
	}
	delete it;

	cout << "]" << endl;
}

void setTest(ILogger* logger) {
	ISet::setLogger(logger);
	ISet::IIterator::setLogger(logger);

	std::random_device rd;
	std::default_random_engine eng(rd());
	std::uniform_real_distribution<double> distr(-100, 100);

	size_t dim = 4;
	size_t vecNum = 5;
	double tol = 1.0e-10;

	cout << "Creating Set A: " << endl;

	ISet* set1 = ISet::createSet();
	for (int i = 0; i < vecNum; i++) {
		vector<double> coords(dim);

		for (int j = 0; j < dim; j++) {
			coords[j] = distr(eng);
		}

		auto vector = IVector::createVector(dim, coords.data());
		assert(set1->insert(vector, IVector::NORM::SECOND, tol) == RC::SUCCESS);
		delete vector;
	}
	printSet(set1);
	assert(set1->getDim() == dim);
	assert(set1->getSize() == vecNum);

	cout << "Cloning Set A as Set B:" << endl;
	auto set2 = set1->clone();
	printSet(set2);

	cout << "Removing vector with index 3:" << endl;
	IVector* removedVec;
	set1->getCopy(3, removedVec);
	set1->remove(3);
	printSet(set1);
	assert(set1->getSize() == vecNum - 1);
	assert(set1->findFirst(removedVec, IVector::NORM::SECOND, tol) != RC::SUCCESS);

	cout << "Intersection of Set A and Set B: " << endl;
	auto intersection = ISet::makeIntersection(set1, set2, IVector::NORM::SECOND, tol);
	printSet(intersection);
	assert(ISet::equals(intersection, set1, IVector::NORM::SECOND, tol));
	delete intersection;

	cout << "Union of Set A and Set B: " << endl;
	auto unionRes = ISet::makeUnion(set1, set2, IVector::NORM::SECOND, tol);
	printSet(unionRes);
	assert(ISet::equals(unionRes, set2, IVector::NORM::SECOND, tol));
	delete unionRes;

	cout << "Subtraction of Set A and Set B: " << endl;
	auto sub = ISet::sub(set1, set2, IVector::NORM::SECOND, tol);
	printSet(sub);
	assert(sub->getSize() == 0);
	delete sub;

	cout << "Symmetric Subtraction of Set A and Set B: " << endl;
	auto sumSub = ISet::symSub(set2, set1, IVector::NORM::SECOND, tol);
	printSet(sumSub);
	assert(sumSub->getSize() == 1);
	assert(sumSub->findFirst(removedVec, IVector::NORM::SECOND, tol) == RC::SUCCESS);
	delete sumSub;

	delete removedVec;

	cout << "Set A is subset of Set B: ";
	bool isSubset = ISet::subSet(set1, set2, IVector::NORM::SECOND, tol);
	cout << isSubset << endl;
	assert(isSubset == true);

	cout << "Set B is subset of Set A: ";
	isSubset = ISet::subSet(set2, set1, IVector::NORM::SECOND, tol);
	cout << isSubset << endl;
	assert(isSubset == false);

	delete set1;
	delete set2;

	cout << "Set test successfully finished\n\n";
}

void printCompact(ICompact* compact, IMultiIndex* byPass) {
	ICompact::IIterator* it = compact->getBegin(byPass);
	IVector* vec;

	cout << "{ " << endl;
	for (; it->isValid(); it->next()) {
		it->getVectorCopy(vec);
		printVector(vec);
		delete vec;
	}
	cout << "}" << endl;

	delete it;
}

void compactTest(ILogger* logger) {
	ICompact::setLogger(logger);
	ICompact::IIterator::setLogger(logger);
	IMultiIndex::setLogger(logger);

	size_t dim = 2;
	double tol = 1.0e-10;

	IMultiIndex* steps = IMultiIndex::createMultiIndex(dim, vector<size_t>{3, 2}.data());

	IVector* bound1 = IVector::createVector(dim, vector<double>{1, 5}.data());
	IVector* bound2 = IVector::createVector(dim, vector<double>{-4, 2}.data());

	IMultiIndex* order = IMultiIndex::createMultiIndex(dim, vector<size_t>{0, 1}.data());

	cout << "Creating Compact A:" << endl;
	ICompact* compact1 = ICompact::createCompact(bound1, bound2, steps);
	printCompact(compact1, order);
	assert(compact1->getDim() == dim);

	cout << "Compact A with reverse iteration:" << endl;
	order->setAxisIndex(0, 1);
	order->setAxisIndex(1, 0);
	printCompact(compact1, order);

	cout << "Trying to create iterator with invalid order: ";
	order->setAxisIndex(0, 1);
	order->setAxisIndex(1, 1);
	assert(compact1->getBegin(order) == nullptr);
	cout << "failed" << endl;
	order->setAxisIndex(0, 0);

	steps->setAxisIndex(0, 3);
	steps->setAxisIndex(1, 2);
	bound1->setCoord(0, -3);
	bound1->setCoord(1, 5);
	bound2->setCoord(0, 2);
	bound2->setCoord(1, 10);

	cout << "Creating Compact B:" << endl;
	ICompact* compact2 = ICompact::createCompact(bound1, bound2, steps);
	printCompact(compact2, order);
	assert(compact2->getDim() == dim);

	steps->setAxisIndex(1, 3);

	cout << "Intersection of Compact A and Compact B:" << endl;
	auto intersection = ICompact::createIntersection(compact1, compact2, steps, tol);
	printCompact(intersection, order);

	IVector* minBound;
	intersection->getLeftBoundary(minBound);
	IVector* correctMinBound = IVector::createVector(dim, vector<double>{-3, 5}.data());
	assert(IVector::equals(correctMinBound, minBound, IVector::NORM::SECOND, tol));

	delete correctMinBound;
	delete minBound;

	IVector* maxBound;
	intersection->getRightBoundary(maxBound);
	IVector* correctMaxBound = IVector::createVector(dim, vector<double>{1, 5}.data());
	assert(IVector::equals(correctMaxBound, maxBound, IVector::NORM::SECOND, tol));

	delete correctMaxBound;
	delete maxBound;

	delete intersection;

	cout << "Span of Compact A and Compact B:" << endl;
	auto span = ICompact::createCompactSpan(compact1, compact2, steps);
	printCompact(span, order);

	delete span;

	cout << "Is point inside Compact A: ";
	auto vec = IVector::createVector(dim, vector<double>{1, 1}.data());
	printVector(vec);
	bool isInside = compact1->isInside(vec);
	cout << isInside << endl;
	assert(isInside == false);

	cout << "Is point inside Compact A: ";
	vec->setCoord(1, 3);
	printVector(vec);
	isInside = compact1->isInside(vec);
	cout << isInside << endl;
	assert(isInside == true);

	delete vec;

	delete compact1;
	delete compact2;
	delete order;
	delete steps;
	delete bound1;
	delete bound2;

	cout << "Compact test successfully finished\n\n";
}

void problemTest(ILogger* logger) {
	LIB dll = LOAD_LIBRARY("libProblem");
	if (!dll) {
		cout << "Unable to load library" << endl;
		return;
	}

	auto getBroker = reinterpret_cast<GetBrokerFunc>(GET_BROKER(dll));
	if (!getBroker) {
		cout << "Unable to load procedure" << endl;
		CLOSE_LIBRARY(dll);
		return;
	}

	auto impl = IBroker::INTERFACE_IMPL::IPROBLEM;

	auto broker = reinterpret_cast<IBroker*>(getBroker());
	assert(broker->canCastTo(impl));

	auto problem = reinterpret_cast<IDiffProblem*>(broker->getInterfaceImpl(impl));

	double tol = 1.0e-10;
	size_t argDim = 2;

	vector<double> leftBoundData = { -20, -20 };
	vector<double> rightBoundData = { 20, 20 };
	vector<size_t> gridData = { 100, 100 };

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

	auto argPoint = IVector::createVector(argDim, vector<double>{ -0.5, -2 }.data());
	cout << "Arguments values: ";
	printVector(argPoint);

	auto paramPoint = IVector::createVector(paramDim, vector<double>{ 3, -1, 7, -9 }.data());
	cout << "Parameters values: ";
	printVector(argPoint);

	double funcValue = 8.75 + 7.0 * sin(4.5);

	problem->setArgs(argPoint);
	double val = problem->evalByParams(paramPoint);
	cout << "Evaluate by params result: " << val << endl;
	assert(fuzzyCompare(funcValue, val, tol));

	problem->setParams(paramPoint);

	val = problem->evalByArgs(argPoint);
	cout << "Evaluate by args result: " << val << endl;
	assert(fuzzyCompare(funcValue, val, tol));

	IMultiIndex* argDerivOrder = IMultiIndex::createMultiIndex(argDim, vector<size_t>{ 0, 0 }.data());

	auto argDerivTest = [&] (double correctRes) {
		cout << "Arguments derivative order: ";
		printMultiIndex(argDerivOrder);

		double val = problem->evalDerivativeByArgs(argPoint, argDerivOrder);
		cout << "Derivative by args result: " << val << endl;
		assert(fuzzyCompare(correctRes, val, tol));
	};

	argDerivTest(funcValue);

	argDerivOrder->setAxisIndex(1, 2);
	argDerivTest(12.0);

	argDerivOrder->setAxisIndex(1, 4);
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
		cout << "Parameters derivative order: ";
		printMultiIndex(paramDerivOrder);

		double val = problem->evalDerivativeByParams(paramPoint, paramDerivOrder);
		cout << "Derivative by params result: " << val << endl;
		assert(fuzzyCompare(correctRes, val, tol));
	};

	paramDerivTest(funcValue);

	paramDerivOrder->setAxisIndex(1, 1);
	paramDerivTest(-8.0);

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

	IVector* argGradient = IVector::createVector(argDim, vector<double>(argDim).data());
	problem->evalGradientByArgs(argPoint, argGradient);
	cout << "Gradient by args: ";
	printVector(argGradient);

	vector<double> correctGradData = { -3 - 63 * cos(4.5), -12 };
	IVector* correctGrad = IVector::createVector(argDim, correctGradData.data());
	assert(IVector::equals(argGradient, correctGrad, IVector::NORM::SECOND, tol));

	delete argGradient;
	delete correctGrad;

	IVector* paramsGradient = IVector::createVector(paramDim, vector<double>(paramDim).data());
	problem->evalGradientByParams(paramPoint, paramsGradient);
	cout << "Gradient by params: ";
	printVector(paramsGradient);

	correctGradData = { 0.25, -8.0, sin(4.5), -3.5 * cos(4.5) };
	correctGrad = IVector::createVector(paramDim, correctGradData.data());
	assert(IVector::equals(paramsGradient, correctGrad, IVector::NORM::SECOND, tol));

	delete paramsGradient;
	delete correctGrad;

	delete argPoint;
	delete paramPoint;

	delete problem;

	CLOSE_LIBRARY(dll);
	cout << "Problem test successfully finished\n\n";
}

int main() {
	ILogger* logger = ILogger::createLogger("Log.txt");

	cout << std::boolalpha;

	cout << "### Vector test ###\n\n";
	vectorTest(logger);

	cout << "### Set test ###\n\n";
	setTest(logger);

	cout << "### Compact test ###\n\n";
	compactTest(logger);

	cout << "### Problem test ###\n\n";
	problemTest(logger);

	return 0;
}
