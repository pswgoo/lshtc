#include "getneighbor.h"
#include "common/common_basic.h"
#include "common/file_utility.h"
#include <queue>
#include <functional>
using namespace std;

int FeatureNeighbor::Max_Remain_Neighbor = 300;

FeatureNeighbor::FeatureNeighbor()
{
	Clear();
}

FeatureNeighbor::~FeatureNeighbor()
{
}

bool FeatureNeighbor::CheckValid() const
{
	if (mTransID.size() != mSimilarity.size())
		return false;
	if (mSimilarity.size() != mNeighbor.size())
		return false;
	return true;
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
	else temp = mulXY / (modX * modY);
	return temp;
}

int FeatureNeighbor::Merge(const FeatureNeighbor& neighbor)
{
	if (!CheckValid() || !neighbor.CheckValid())
	{
		cerr << "Error: check valid error, can't merge" << endl;
		return -1;
	}

	int start = (int)mTransID.size();
	for (map<int,int>::const_iterator it = neighbor.mTransID.cbegin(); it != neighbor.mTransID.cend(); ++it)
	{
		if (mTransID.count(it->first) > 0)
		{
			cerr << "Error: the id " << it->first << " has occured, can't merge" << endl;
			return 0;
		}
		mTransID[it->first] = start + it->second;
	}
	for (size_t i = 0; i < neighbor.mNeighbor.size(); ++i)
	{
		mNeighbor.push_back(neighbor.mNeighbor[i]);
		mSimilarity.push_back(neighbor.mSimilarity[i]);
	}
	return 0;
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

	for (int i = 0; i < testsetsize; i++)
		mTransID.insert(pair<int, int>(testsetID[i], i));

	int numThreads = omp_get_num_procs();
	clog << "CPU number: " << numThreads << endl;

	omp_set_num_threads(numThreads);
	clog << "Start Parallel Extract Features" << endl;

	typedef priority_queue<pair<double, int>, vector<pair<double, int>>, greater<pair<double, int>>> Priority_Queue;
#pragma omp parallel for schedule(dynamic)
	for (int i = 0; i < testsetsize; i++)
	{
		if ((i & 255) == 0)
			clog << i << "th feature calc" << endl;

		Priority_Queue heap;
		double tempcalc;
		for (int j = 0; j < trainsetsize; j++)
		{
			tempcalc = CalcSimilarity(trainset[j], testset[i]);//calc
			if (heap.size() < Max_Remain_Neighbor || heap.top().first < tempcalc)
			{
				heap.push(make_pair(tempcalc, trainsetID[j]));
				while (heap.size() > Max_Remain_Neighbor)
					heap.pop();
			}
		}
		
		mSimilarity[i].resize(heap.size());
		mNeighbor[i].resize(heap.size());
		int cur = (int)heap.size() - 1;
		while (!heap.empty())
		{
			mSimilarity[i][cur] = heap.top().first;
			mNeighbor[i][cur] = heap.top().second;
			--cur;
			heap.pop();
		}
	}//get the topK
	clog << "Build completed" << endl;
	return 0;
}

int FeatureNeighbor::GetNeighbor(int testID, int topK, std::vector<int>& neighborID) const
{
	neighborID.clear();
	int temptestID = 0;
	map<int, int>::const_iterator it = mTransID.find(testID);
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

int FeatureNeighbor::GetNeighbor(int testID, int topK, std::vector<int>& neighborID, std::vector<double>& neighborSimilarity) const
{
	neighborID.clear();
	neighborSimilarity.clear();
	int temptestID = 0;
	map<int, int>::const_iterator it = mTransID.find(testID);
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

	fclose(inFile);

	if (printLog != SILENT)
		clog << "Load successful!" << endl;

	return 0;
}

int FeatureNeighbor::SaveBin(std::string fileName, int printLog) const
{
	int rtn = 0;
	FILE *outfile = fopen(fileName.c_str(), "wb");

	Write(outfile, mTransID);
	CHECK_RTN(rtn);

	Write(outfile, mNeighbor);
	CHECK_RTN(rtn);

	Write(outfile, mSimilarity);
	CHECK_RTN(rtn);

	fclose(outfile);

	if (printLog != SILENT)
		clog << "Save successful!" << endl;

	return 0;
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
