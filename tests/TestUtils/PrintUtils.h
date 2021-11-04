#pragma once

#include <ICompact.h>
#include <ISet.h>
#include <IVector.h>

namespace PrintUtils {
	void printVector(IVector* vector);
	void printMultiIndex(IMultiIndex* index);
	void printSet(ISet* set);
	void printCompact(ICompact* compact, IMultiIndex* byPass);
}