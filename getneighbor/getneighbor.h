#ifndef FEATURE_NEIGHBOR_H
#define FEATURE_NEIGHBOR_H

#include "common/common_basic.h"
#include "extractfeature/feature.h"
#include <omp.h>

class FeatureNeighbor
{
public:
	static int Max_Remain_Neighbor;
	std::map<int, int> mTransID;
	std::vector<std::vector<int> > mNeighbor;
	std::vector<std::vector<double> > mSimilarity;

public:
	static double CalcSimilarity(const Feature& feature1, const Feature& feature2);

public:
	FeatureNeighbor();
	~FeatureNeighbor();

	bool CheckValid() const;

	int Clear();

	int Merge(const FeatureNeighbor& neighbor);

	int Build(std::vector<std::map<int, double> > trainset, std::vector<std::map<int, double> > testset, std::vector<int> trainsetID, std::vector<int> testsetID, int printLog = SILENT);

	int GetNeighbor(int testID, int topK, std::vector<int>& neighborID) const;

	int GetNeighbor(int testID, int topK, std::vector<int>& neighborID, std::vector<double>& neighborSimilarity) const;

	int SaveBin(std::string fileName, int printLog = SILENT) const;

	int LoadBin(std::string fileName, int printLog = SILENT);

};

#endif