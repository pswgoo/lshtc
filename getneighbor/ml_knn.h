#ifndef ML_KNN_H
#define ML_KNN_H

#include "getneighbor.h"
#include <map>

class MultiLabelKnn
{
public:
	const static int mLaplsSmooth = 1; //used as Laplace smoothing
	int mK;
	std::map<int, int> mLabelToIndex;
	std::map<int, int> mIndexToLabel;

	//In the class MultiLabelKnn, all label is replaced by the index.
	std::map<int, std::vector<int>> mTrainSet;
	std::vector<double> mPriorProb;
	std::vector<int> mPosSum;
	std::vector<std::map<int, int>> mPosCnt;
	std::vector<int> mNegSum;
	std::vector<std::map<int, int>> mNegCnt;

private:
	int Resize(int size);
public:
	MultiLabelKnn(int k = 3);
	~MultiLabelKnn();

	//the function Clear() not clear 'mK'
	int Clear();

	int Initialize(const std::map<int, std::vector<int>>& trainSet, const std::map<int, std::vector<int>>& testSet, const FeatureNeighbor& neighbors);

	int Predict(const FeatureNeighbor& neighbors, int testId, std::vector<std::pair<int, double>>& scores) const;

	int Predict(const FeatureNeighbor& neighbors, int testId, std::vector<int>& labels, double threshold = 1.0) const;

	int Save(std::string file_name, int print_log = SILENT);

	int Save(FILE* out_file, int print_log = SILENT);

	int Load(std::string file_name, int print_log = SILENT);

	int Load(FILE* in_file, int print_log = SILENT);
};

#endif