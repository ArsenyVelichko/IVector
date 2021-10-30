#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>
#include <vector>
#include <random>

#include <ICompact.h>
#include <ISet.h>
#include <IVector.h>

using namespace std;

void printVector(IVector* vector) {
	function<void(double)> print = [](double x) { cout << x << ' '; };
	cout << "( ";
	vector->foreach (print);
	cout << ")" << endl;
}

void printBool(bool x) {
	cout << (x ? "true" : "false") << endl;
}

bool fuzzyCompare(double x, double y, double tol) { return fabs(x - y) < tol; }

void vectorTest() {
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

void setTest() {
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
	printBool(isSubset);
	assert(isSubset == true);

	cout << "Set B is subset of Set A: ";
	isSubset = ISet::subSet(set2, set1, IVector::NORM::SECOND, tol);
	printBool(isSubset);
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
	printBool(isInside);
	assert(isInside == false);

	cout << "Is point inside Compact A: ";
	vec->setCoord(1, 3);
	printVector(vec);
	isInside = compact1->isInside(vec);
	printBool(isInside);
	assert(isInside == true);

	delete vec;

	delete compact1;
	delete compact2;
	delete order;
	delete steps;
	delete bound1;
	delete bound2;
}

int main() {
	ILogger* logger = ILogger::createLogger("Log.txt");
	logger->info(RC::SUCCESS);

	IVector::setLogger(logger);
	ISet::setLogger(logger);
	ISet::IIterator::setLogger(logger);
	ICompact::setLogger(logger);
	ICompact::IIterator::setLogger(logger);
	IMultiIndex::setLogger(logger);

	cout << "### Vector test ###\n\n";
	vectorTest();

	cout << "### Set test ###\n\n";
	setTest();

	cout << "### Compact test ###\n\n";
	compactTest(logger);

	return 0;
}
