#include <random>
#include <vector>
#include <iostream>
#include <cassert>

#include "Tests.h"
#include "PrintUtils.h"

void Tests::setTest(ILogger* logger) {
	ISet::setLogger(logger);
	ISet::IIterator::setLogger(logger);

	std::random_device rd;
	std::default_random_engine eng(rd());
	std::uniform_real_distribution<double> distr(-100, 100);

	size_t dim = 4;
	size_t vecNum = 5;
	double tol = 1.0e-10;

	std::cout << "Creating Set A: " << std::endl;

	ISet* set1 = ISet::createSet();
	for (int i = 0; i < vecNum; i++) {
		std::vector<double> coords(dim);

		for (int j = 0; j < dim; j++) {
			coords[j] = distr(eng);
		}

		auto vector = IVector::createVector(dim, coords.data());
		assert(set1->insert(vector, IVector::NORM::SECOND, tol) == RC::SUCCESS);
		delete vector;
	}
	PrintUtils::printSet(set1);
	assert(set1->getDim() == dim);
	assert(set1->getSize() == vecNum);

	std::cout << "Cloning Set A as Set B:" << std::endl;
	auto set2 = set1->clone();
	PrintUtils::printSet(set2);

	std::cout << "Removing std::vector with index 3:" << std::endl;
	IVector* removedVec;
	set1->getCopy(3, removedVec);
	set1->remove(3);
	PrintUtils::printSet(set1);
	assert(set1->getSize() == vecNum - 1);
	assert(set1->findFirst(removedVec, IVector::NORM::SECOND, tol) != RC::SUCCESS);

	std::cout << "Intersection of Set A and Set B: " << std::endl;
	auto intersection = ISet::makeIntersection(set1, set2, IVector::NORM::SECOND, tol);
	PrintUtils::printSet(intersection);
	assert(ISet::equals(intersection, set1, IVector::NORM::SECOND, tol));
	delete intersection;

	std::cout << "Union of Set A and Set B: " << std::endl;
	auto unionRes = ISet::makeUnion(set1, set2, IVector::NORM::SECOND, tol);
	PrintUtils::printSet(unionRes);
	assert(ISet::equals(unionRes, set2, IVector::NORM::SECOND, tol));
	delete unionRes;

	std::cout << "Subtraction of Set A and Set B: " << std::endl;
	auto sub = ISet::sub(set1, set2, IVector::NORM::SECOND, tol);
	PrintUtils::printSet(sub);
	assert(sub->getSize() == 0);
	delete sub;

	std::cout << "Symmetric Subtraction of Set A and Set B: " << std::endl;
	auto sumSub = ISet::symSub(set2, set1, IVector::NORM::SECOND, tol);
	PrintUtils::printSet(sumSub);
	assert(sumSub->getSize() == 1);
	assert(sumSub->findFirst(removedVec, IVector::NORM::SECOND, tol) == RC::SUCCESS);
	delete sumSub;

	delete removedVec;

	std::cout << "Set A is subset of Set B: ";
	bool isSubset = ISet::subSet(set1, set2, IVector::NORM::SECOND, tol);
	std::cout << isSubset << std::endl;
	assert(isSubset == true);

	std::cout << "Set B is subset of Set A: ";
	isSubset = ISet::subSet(set2, set1, IVector::NORM::SECOND, tol);
	std::cout << isSubset << std::endl;
	assert(isSubset == false);

	delete set1;
	delete set2;

	std::cout << "Set test successfully finished\n\n";
}
