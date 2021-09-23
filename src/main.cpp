#include <iostream>
#include <limits>

#include "../include/IVector.h"
#include "../include/ISet.h"
#include "../include/ICompact.h"
#include "../include/IBroker.h"
#include "../include/IDiffProblem.h"
#include "../include/ISolver.h"

using namespace std;

/*** vector testing section ***/

void vectorTest(ILogger* logger) {
	double data1[] = {1, 2, 3};
	double data2[] = {-1, -2, -3};
	IVector* vec1 = IVector::createVector(3, data1);
	IVector* vec2 = IVector::createVector(3, data2);
	IVector* bufVec;
	function<void(double)> print = [](double num) { std::cout << num << ' '; };
	IVector::setLogger(logger);

	cout << "vec1: ";
	vec1->foreach (print);
	cout << "\nvec2: ";
	vec2->foreach (print);

	cout << endl;

	bufVec = IVector::sub(vec1, vec2);
	cout << "\nvec1 - vec2 = ";
	bufVec->foreach (print);
	delete bufVec;

	cout << "\n(vec1,vec2) = " << IVector::dot(vec1, vec2) << endl;

	cout << "\n||vec1||_1 = " << vec1->norm(IVector::NORM::FIRST) << endl;
	cout << "||vec2||_inf = " << vec2->norm(IVector::NORM::CHEBYSHEV) << endl;

	cout << endl;
	cout << "Setting coord 1 in vec2 to 16" << endl;
	vec2->setCord(1, 16);
	cout << "Setting coord 5 in vec1 to 0" << endl;
	vec1->setCord(5, 0);

	cout << "vec1: ";
	vec1->foreach (print);
	cout << "\nvec2: ";
	vec2->foreach (print);

	cout << endl;
	cout << "\nvec1 memory allocated: " << vec1->sizeAllocated() << " bytes" << endl;
	cout << "Copying vec2 to vec1" << endl;
	IVector::copyInstance(vec1, vec2);
	cout << "vec1: ";
	vec1->foreach (print);
	cout << "\nvec2: ";
	vec2->foreach (print);

	cout << endl;
	delete vec1;
	delete vec2;

	data1[0] = numeric_limits<double>::infinity();
	cout << "Trying to construct a vector using array { " << data1[0] << " , " << data1[1] << " , "
		 << data1[2] << " }" << endl;
	vec1 = IVector::createVector(3, data1);

	cout << "\nIVector::createVector() returned " << vec1 << endl;
	cout << "\nTest is finished" << endl;
}

/*** set testing section ***/

constexpr size_t vecNum = 10;
constexpr size_t dim = 3;

void freeVecs(IVector** arr) {
	for (size_t i = 0; i < vecNum; i++) {
		delete arr[i];
	}
}

bool printSetContent(ISet* set, function<void(double)>& printer) {
	if (set->getSize() == 0) {
		cout << "{ empty set }" << endl;
		return true;
	}
	auto iter = set->getBegin();
	if (!iter) {
		return false;
	}
	double coords[dim] = {0};
	IVector* vec = IVector::createVector(dim, coords);
	if (!vec) {
		delete iter;
		return false;
	}
	cout << '{' << endl;
	while (iter->isValid()) {
		iter->getVectorCoords(vec);
		cout << "{ ";
		vec->foreach (printer);
		cout << '}' << endl;
		iter->next();
	}
	cout << '}' << endl;
	delete vec;
	delete iter;
	return true;
}

void setTest(ILogger* logger) {
	IVector* vecArray[vecNum] = {nullptr};
	double coords[dim] = {0};
	function<void(double)> print = [](double num) { std::cout << num << ' '; };
	IVector::setLogger(logger);
	ISet::setLogger(logger);
	for (int i = 0; i < vecNum; i++) {
		for (int j = 0; j < dim; j++) {
			coords[j] = i - j;
		}
		vecArray[i] = IVector::createVector(dim, coords);
		if (!vecArray[i]) {
			freeVecs(vecArray);
			cout << "Test failed: not enough heap memory" << endl;
			return;
		}
		cout << "vec_" << i << " = { ";
		vecArray[i]->foreach (print);
		cout << "}" << endl;
	}
	cout << "Inserting vectors to set1" << endl;
	ISet* set1 = ISet::createSet();
	if (!set1) {
		cout << "Test failed: not enough heap memory" << endl;
		freeVecs(vecArray);
		return;
	}
	for (auto vec : vecArray) {
		RC code = set1->insert(vec, IVector::NORM::FIRST, 1.0);
		if (code != RC::SUCCESS) {
			cout << "Test failed with error" << endl;
			logger->severe(code);
			freeVecs(vecArray);
			delete set1;
			return;
		}
	}
	cout << "Set1 content: ";
	if (!printSetContent(set1, print)) {
		cout << "set printing error, test is failed" << endl;
		freeVecs(vecArray);
		delete set1;
		return;
	}
	cout << "This output was generated using set iterator" << endl;
	cout << "Cloning set1 to set2" << endl;
	ISet* set2 = set1->clone();
	if (!set2) {
		cout << "Test failed: not enough heap memory" << endl;
		freeVecs(vecArray);
		delete set1;
		return;
	}
	cout << "Result of the operation \"is set1 equals to set2\": "
		 << ISet::equals(set1, set2, IVector::NORM::SECOND, 0.01) << endl;
	cout << "Deleting all vectors that are similar to {7, 6, 5} with infinite norm and tolerance 1"
		 << endl;
	set1->remove(vecArray[vecNum - 3], IVector::NORM::CHEBYSHEV, 1.0);
	cout << "Set1 content: ";
	if (!printSetContent(set1, print)) {
		cout << "set printing error, test is failed" << endl;
		freeVecs(vecArray);
		delete set1;
		delete set2;
		return;
	}
	cout << "Set2 content: ";
	if (!printSetContent(set2, print)) {
		cout << "set printing error, test is failed" << endl;
		freeVecs(vecArray);
		delete set1;
		delete set2;
		return;
	}
	cout << "Union of set1 and set2: ";
	ISet* set3 = ISet::makeUnion(set1, set2, IVector::NORM::SECOND, 0.01);
	if (!set3) {
		cout << "Test failed: not enough heap memory" << endl;
		freeVecs(vecArray);
		delete set1;
		delete set2;
		return;
	}
	if (!printSetContent(set3, print)) {
		cout << "set printing error, test is failed" << endl;
		freeVecs(vecArray);
		delete set1;
		delete set2;
		return;
	}
	delete set3;
	cout << "Intersection of set1 and set2: ";
	set3 = ISet::makeIntersection(set1, set2, IVector::NORM::SECOND, 0.01);
	if (!set3) {
		cout << "Test failed: not enough heap memory" << endl;
		freeVecs(vecArray);
		delete set1;
		delete set2;
		return;
	}
	if (!printSetContent(set3, print)) {
		cout << "set printing error, test is failed" << endl;
		freeVecs(vecArray);
		delete set1;
		delete set2;
		return;
	}
	delete set3;
	cout << "Difference of set1 and set2: ";
	set3 = ISet::sub(set1, set2, IVector::NORM::SECOND, 0.01);
	if (!set3) {
		cout << "Test failed: not enough heap memory" << endl;
		freeVecs(vecArray);
		delete set1;
		delete set2;
		return;
	}
	if (!printSetContent(set3, print)) {
		cout << "set printing error, test is failed" << endl;
		freeVecs(vecArray);
		delete set1;
		delete set2;
		return;
	}
	delete set3;
	cout << "Symmetric difference of set1 and set2: ";
	set3 = ISet::symSub(set1, set2, IVector::NORM::SECOND, 0.01);
	if (!set3) {
		cout << "Test failed: not enough heap memory" << endl;
		freeVecs(vecArray);
		delete set1;
		delete set2;
		return;
	}
	if (!printSetContent(set3, print)) {
		cout << "set printing error, test is failed" << endl;
		freeVecs(vecArray);
		delete set1;
		delete set2;
		return;
	}
	delete set3;
	cout << "Result of the operation \"is set1 subset of set2\": " << ISet::subSet(set1, set2, IVector::NORM::SECOND, 0.01) << endl;
	cout << "Adding vector {0, 0, 0} to set1" << endl;
	coords[0] = coords[1] = coords[2] = 0;
	vecArray[0]->setData(dim, coords);
	if (set1->insert(vecArray[0], IVector::NORM::SECOND, 0.01) != RC::SUCCESS) {
		cout << "Test failed: troubles with inserting" << endl;
		freeVecs(vecArray);
		delete set1;
		delete set2;
		return;
	}
	cout << "Result of the operation \"is set1 subset of set2\": " << ISet::subSet(set1, set2, IVector::NORM::SECOND, 0.01) << endl;
	cout << "Inserting 10000 vectors to set2" << endl;
	for (int i = 0; i < 10000; i++) {
		coords[0] = (i * i - 7 * i + 3) % 1301;
		coords[1] = (-3 * i * i + 255 * i + 32) % 1553;
		coords[2] = (i * i - 7833 * i - 98) % 1217;
		vecArray[0]->setData(dim, coords);
		RC code = set2->insert(vecArray[0], IVector::NORM::SECOND, 0.01);
		if (code != RC::SUCCESS) {
			logger->severe(code);
			break;
		}
		if (i % 1000 == 0) {
			cout << '.';
			cout.flush();
		}
	}
	cout << endl;
	cout << "Set2 size: " << set2->getSize() << endl;
	cout << "Deleting set2" << endl;
	delete set2;
	cout << "Set1 content: ";
	if (!printSetContent(set1, print)) {
		cout << "\nSet printing error, test is failed" << endl;
		freeVecs(vecArray);
		delete set1;
		return;
	}
	cout << "Getting iterator to the element with index 3" << endl;
	auto iterator = set1->getIterator(3);
	if (!iterator) {
		cout << "\nUnable to get iterator. Test error" << endl;
		delete set1;
		freeVecs(vecArray);
		return;
	}
	cout << "Vector inside iterator: { ";
	IVector* vec = nullptr;
	if (iterator->getVectorCopy(vec) != RC::SUCCESS) {
		cout << "\nUnable to get vector from the iterator. Test error" << endl;
		delete set1;
		freeVecs(vecArray);
		return;
	}
	vec->foreach(print);
	cout << "}\nDeleting vector with index 3 from the set" << endl;
	set1->remove(3);
	cout << "Vector inside iterator: { ";
	if (iterator->getVectorCoords(vec) != RC::SUCCESS) {
		cout << "\nUnable to get vector from the iterator. Test error" << endl;
		delete set1;
		freeVecs(vecArray);
		return;
	}
	vec->foreach(print);
	cout << "}\nMoving iterator forward" << endl;
	iterator->next();
	cout << "Vector inside iterator: { ";
	if (iterator->getVectorCoords(vec) != RC::SUCCESS) {
		cout << "Unable to get vector from the iterator. Test error" << endl;
		delete set1;
		freeVecs(vecArray);
		return;
	}
	vec->foreach(print);
	cout << "}\nMoving iterator back" << endl;
	iterator->previous();
	cout << "Vector inside iterator: { ";
	if (iterator->getVectorCoords(vec) != RC::SUCCESS) {
		cout << "Unable to get vector from the iterator. Test error" << endl;
		delete set1;
		freeVecs(vecArray);
		return;
	}
	vec->foreach(print);
	cout << '}' << endl;
	cout << "Set1 content: ";
	if (!printSetContent(set1, print)) {
		cout << "\nSet printing error, test is failed" << endl;
		freeVecs(vecArray);
		delete iterator;
		delete vec;
		delete set1;
		return;
	}
	cout << "Printing only vectors with even indexes in set1 (using iterators): {" << endl;
	iterator->makeBegin();
	while (iterator->isValid()) {
		iterator->getVectorCoords(vec);
		cout << "{ ";
		vec->foreach(print);
		cout << "}" << endl;
		iterator->next(2);
	}
	cout << "}\n\nTest is finished successfully" << endl;
	delete iterator;
	delete vec;
	delete set1;
	freeVecs(vecArray);
}

int main() {
	ILogger* logger = ILogger::createLogger("Log.txt");

	cout << "### Vector test ###\n\n";
	vectorTest(logger);

	cout << "### Set test ###\n\n";
	setTest(logger);

	return 0;
}
