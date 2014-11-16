#ifndef _RANKLINEAR_H
#define _RANKLINEAR_H
#include "mylinear.h"

#include <iostream>
#include <map>
#include <vector>
#include <algorithm> 

class RankInstanceSet
{
public:
	std::vector<int> mQueries;
	std::vector<double> mLabels;
	std::vector<Feature> mFeatures;

	int clear();

};

#endif /*_RANKLINEAR_H*/