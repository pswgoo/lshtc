#include "common/common_basic.h"
#include <omp.h>

class Featureneighbor
{

public:
	static int Max_Remain_Neighbor;
	std::map<int, int> mTransID;
	std::vector<std::vector<int> > mNeighbor;
	std::vector<std::vector<double> > mSimilarity;

public:
	Featureneighbor();
	~Featureneighbor();

	int Clear();

	double CalcSimilarity(std::vector<double> traindata, std::vector<double> testdata);

	int Build(std::vector<std::map<int, double> > trainset, std::vector<std::map<int, double> > testset, std::vector<int> trainsetID, std::vector<int> testsetID, int printLog = SILENT);

	int GetNeighbor(int testID, int topK, std::vector<int>& neighborID);

	int GetNeighbor(int testID, int topK, std::vector<int>& neighborID, std::vector<double>& neighborSimilarity);

	int SaveBin(std::string fileName, int printLog = SILENT);

	int LoadBin(std::string fileName, int printLog = SILENT);

};
