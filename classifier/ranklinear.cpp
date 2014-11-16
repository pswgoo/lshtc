#define _CRT_SECURE_NO_WARNINGS
#include"medline\basic.h"
#include"liblinear\blas\blas.h"
#include"liblinear\blas\blasp.h"
#include"ranklinear.h"

int RankInstanceSet::clear()
{
	mQueries.clear();
	mLabels.clear();
	mFeatures.clear();
	return 0;
}
