#include "getneighbor.h"
#include "common/common_basic.h"
#include "common/file_utility.h"
using namespace std;

int FeatureNeighbor::Max_Remain_Neighbor = 300;

FeatureNeighbor::FeatureNeighbor()
{
	Clear();
}

FeatureNeighbor::~FeatureNeighbor()
{
}

int FeatureNeighbor::Clear()
{
	mTransID.clear();
	mNeighbor.clear();
	mSimilarity.clear();
	return 0;
}

double FeatureNeighbor::CalcSimilarity(const Feature& feature1, const Feature& feature2)
{
	double modX, modY, mulXY, temp;
	int tempX;
	modX = modY = mulXY = temp = 0.;

	for (Feature::const_iterator it = feature1.begin(); it != feature1.end(); ++it)
		modX += it->second * it->second;

	for (Feature::const_iterator it = feature2.begin(); it != feature2.end(); ++it)
		modY += it->second * it->second;

	modX = sqrt(modX), modY = sqrt(modY);// get |x|, |y|

	for (Feature::const_iterator it = feature1.begin(); it != feature1.end(); ++it)
	{
		tempX = it->first;
		Feature::const_iterator iter = feature2.find(tempX);
		if (iter != feature2.end())
			mulXY += it->second * iter->second;
	}//get x * y
	
	if (modX < 1e-6 || modY < 1e-6) temp = 0.;
	else temp = mulXY / modX / modY;
	return temp;
}

int FeatureNeighbor::Build(std::vector<std::map<int, double> > trainset, std::vector<std::map<int, double> > testset, std::vector<int> trainsetID, std::vector<int> testsetID, int printLog)
{
	std::vector<std::vector<double> > temptrainset, temptestset;
	std::vector<double> tempfeature;
	temptrainset.clear();
	temptestset.clear();
	
	if (printLog == FULL_LOG) clog << "Load trainset" << endl;
	int trainsetsize = (int)trainset.size();
	int testsetsize = (int)testset.size();
	mSimilarity.resize(testsetsize);
	mNeighbor.resize(testsetsize);

	int numThreads = omp_get_num_procs();
	clog << "CPU number: " << numThreads << endl;

	omp_set_num_threads(numThreads);
	clog << "Start Parallel Extract Features" << endl;
#pragma omp parallel for schedule(dynamic)
	for (int i = 0; i < testsetsize; i++)
	{
		if (i & 32767 == 0)
			clog << i << "th feature calc" << endl;
		vector<double> tempsimlar, tempcalc;
		vector<int> tempsimlarID;
		tempsimlar.clear();
		tempsimlarID.clear();
		tempcalc.clear();
		
		for (int j = 0; j < trainsetsize; j++)
			tempcalc.push_back(CalcSimilarity(trainset[j], testset[i]));//calc

		for (int j = 0; j < Max_Remain_Neighbor; j++)
			tempsimlar.push_back(-1.0), tempsimlarID.push_back(-1);
		for (int j = 0; j < trainsetsize; j++)
		{
			int nowPos = Max_Remain_Neighbor - 1;
			if (tempsimlar[nowPos] > tempcalc[j]) continue;
			while (nowPos > 0)
			{
				if (tempsimlar[nowPos - 1] > tempcalc[j]) break;
				tempsimlar[nowPos] = tempsimlar[nowPos - 1];
				tempsimlarID[nowPos] = tempsimlarID[nowPos - 1];
				nowPos--;
			}
			tempsimlar[nowPos] = tempcalc[j];
			tempsimlarID[nowPos] = trainsetID[j];
		}
		//if (i & 127 == 0) printf("\n%d testset feature calc", i);
		mSimilarity[i] = tempsimlar;
		mNeighbor[i] = tempsimlarID;
	}//get the topK
	return 0;
}

int FeatureNeighbor::GetNeighbor(int testID, int topK, std::vector<int>& neighborID)
{
	neighborID.clear();
	int temptestID = 0;
	map<int, int>::iterator it = mTransID.find(testID);
	if (it == mTransID.end())
	{
		clog << "Can't find the testID!";
		return -1;
	}
	temptestID = it->second;
	if (topK > Max_Remain_Neighbor)
	{
		clog << "The topK is exceed!";
		return -1;
	}
	for (int i = 0; i < topK; i++) neighborID.push_back(mNeighbor[temptestID][i]);
	return 0;
}

int FeatureNeighbor::GetNeighbor(int testID, int topK, std::vector<int>& neighborID, std::vector<double>& neighborSimilarity)
{
	neighborID.clear();
	neighborSimilarity.clear();
	int temptestID = 0;
	map<int, int>::iterator it = mTransID.find(testID);
	if (it == mTransID.end())
	{
		clog << "Can't find the testID!";
		return -1;
	}
	temptestID = it->second;
	if (topK > Max_Remain_Neighbor)
	{
		clog << "The topK is exceed!";
		return -1;
	}
	for (int i = 0; i < topK; i++)
		neighborID.push_back(mNeighbor[temptestID][i]);
	for (int i = 0; i < topK; i++)
		neighborSimilarity.push_back(mSimilarity[temptestID][i]);
	return 0;
}

int FeatureNeighbor::LoadBin(std::string fileName, int printLog)
{
	mTransID.clear();
	mNeighbor.clear();
	mSimilarity.clear();

	int rtn = 0;
	FILE *inFile = fopen(fileName.c_str(), "rb");
	
	Read(inFile, mTransID);
	CHECK_RTN(rtn);
	
	Read(inFile, mNeighbor);
	CHECK_RTN(rtn);

	Read(inFile, mSimilarity);
	CHECK_RTN(rtn);
	
	if (printLog != SILENT)
		clog << "Load successful!" << endl;

	return 0;
}

int FeatureNeighbor::SaveBin(std::string fileName, int printLog)
{
	int rtn = 0;
	FILE *outfile = fopen(fileName.c_str(), "wb");

	Write(outfile, mTransID);
	CHECK_RTN(rtn);

	Write(outfile, mNeighbor);
	CHECK_RTN(rtn);

	Write(outfile, mSimilarity);
	CHECK_RTN(rtn);

	if (printLog != SILENT)
		clog << "Save successful!" << endl;

	return 0;
}