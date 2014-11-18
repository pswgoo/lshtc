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

void neighborSorted(vector<pair<double, int> >::iterator neighborL, vector<pair<double, int> >::iterator neighborR, int Lpos, int Rpos, int topK)
{
	if (Lpos >= topK) return;
	vector<pair<double, int> >::iterator tempL, tempR;
	tempL = neighborL, tempR = neighborR;
	int i, j;
	double mid;
	i = Lpos, j = Rpos, mid = 0.5 * ((*neighborL).first + (*neighborR).first);
	while (i <= j)
	{
		while ((*tempL).first > mid) ++i, ++tempL;
		while ((*tempR).first < mid) --j, --tempR;
		if (i <= j)
		{
			swap((*tempL).first, (*tempR).first);
			swap((*tempL).second, (*tempR).second);
			++i, ++tempL;
			--j, --tempR;
		}
	}
	if (Lpos < j) neighborSorted(neighborL, tempR, Lpos, j, topK);
	if (i < Rpos) neighborSorted(tempL, neighborR, i, Rpos, topK);
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
		if ((i & 32767) == 0)
			clog << i << "th feature calc" << endl;
		vector<double> tempsimlar;
		vector<int> tempsimlarID;
		double tempcalc;
		vector<pair<double, int> >tempsorted;
		tempsimlar.clear();
		tempsimlarID.clear();
		tempsorted.clear();

		for (int j = 0; j < trainsetsize; j++)
		{
			tempcalc = CalcSimilarity(trainset[j], testset[i]);//calc
			tempsorted.push_back(pair<double, int>(tempcalc, trainsetID[j]));
		}

		vector<pair<double, int> >::iterator neighborl = tempsorted.begin();
		neighborSorted(tempsorted.begin(), --tempsorted.end(), 0, trainsetsize - 1, Max_Remain_Neighbor);

		int nowPos = 0;
		for (int j = 0; j < Max_Remain_Neighbor; j++)
		{
			tempsimlar.push_back(tempsorted[j].first);
			tempsimlarID.push_back(tempsorted[j].second);
		}
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