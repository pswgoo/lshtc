#include "getneighbor.h"
#include "common/common_basic.h"
#include "common/file_utility.h"
using namespace std;

int Featureneighbor::Max_Remain_Neighbor = 300;

Featureneighbor::Featureneighbor()
{
	Clear();
}

Featureneighbor::~Featureneighbor()
{
}

int Featureneighbor::Clear()
{
	mTransID.clear();
	mNeighbor.clear();
	mSimilarity.clear();
	return 0;
}

double Featureneighbor::CalcSimilarity(std::vector<double> traindata, std::vector<double> testdata)
{
	double modX, modY, mulXY, temp;
	int dimX, dimY, dimAll, trainPos, testPos;
	modX = modY = mulXY = temp = 0.;
	dimX = (int)traindata.size(), dimY = (int)testdata.size();
	dimAll = max(dimX, dimY);
	
	for (int i = 1; i < dimX; i += 2) modX += traindata[i] * traindata[i];
	for (int i = 1; i < dimY; i += 2) modY += testdata[i] * testdata[i];
	modX = sqrt(modX), modY = sqrt(modY);// get |x|, |y|

	trainPos = testPos = 0;
	while (trainPos < dimX && testPos < dimY)//token1 < token2 < token3 < ... < tokenn
	{
		if (traindata[trainPos] > testdata[testPos] + 1e-6) testPos += 2;
		else if (traindata[trainPos] + 1e-6 < testdata[testPos]) trainPos += 2;
		else
		{
			mulXY += traindata[trainPos + 1] * testdata[testPos + 1];
			trainPos += 2;
			testPos += 2;
		}
	}//get x * y
	
	if (modX < 1e-6 || modY < 1e-6) temp = 0.;
	else temp = mulXY / modX / modY;
	return temp;
}

int Featureneighbor::Build(std::vector<std::map<int, double> > trainset, std::vector<std::map<int, double> > testset, std::vector<int> trainsetID, std::vector<int> testsetID, int printLog)
{
	std::vector<std::vector<double> > temptrainset, temptestset;
	std::vector<double> tempfeature;
	temptrainset.clear();
	temptestset.clear();
	
	if (printLog == FULL_LOG) clog << "Load trainset" << endl;
	int trainsetsize = (int)trainset.size();
	for (int i = 0; i < trainsetsize; i++)
	{
		tempfeature.clear();
		for (std::map<int, double>::iterator iter = trainset[i].begin(); iter != trainset[i].end(); ++iter)
		{
			tempfeature.push_back(1.0 * iter->first);
			tempfeature.push_back(iter->second);
		}
		temptrainset.push_back(tempfeature);
	}
	if (printLog == FULL_LOG) clog << "Load testset" << endl;
	int testsetsize = (int)testset.size();
	for (int i = 0; i < testsetsize; i++)
	{
		mTransID.insert(std::pair<int, int>(testsetID[i], i));
		tempfeature.clear();
		for (std::map<int, double>::iterator iter = testset[i].begin(); iter != testset[i].end(); ++iter)
		{
			tempfeature.push_back(1.0 * iter->first);
			tempfeature.push_back(iter->second);
		}
		temptestset.push_back(tempfeature);
	}//load featureset and save as vector<vector<double> >
	
	int numThreads = omp_get_num_procs();
	clog << "CPU number: " << numThreads << endl;

	omp_set_num_threads(numThreads);
	clog << "Start Parallel Extract Features" << endl;
	printf("%lf\n", CalcSimilarity(temptrainset[0], temptestset[0]));
	//while (1);
#pragma omp parallel for schedule(dynamic)
	for (int i = 0; i < testsetsize; i++)
	{
		vector<double> tempsimlar, tempcalc;
		vector<int> tempsimlarID;
		tempsimlar.clear();
		tempsimlarID.clear();
		tempcalc.clear();
		
		for (int j = 0; j < trainsetsize; j++)
			tempcalc.push_back(CalcSimilarity(temptrainset[j], temptestset[i]));//calc

		for (int j = 0; j < Max_Remain_Neighbor; j++) tempsimlar.push_back(-1.0), tempsimlarID.push_back(-1);
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
		mSimilarity.push_back(tempsimlar);
		mNeighbor.push_back(tempsimlarID);
	}//get the topK
	return 0;
}

int Featureneighbor::GetNeighbor(int testID, int topK, std::vector<int>& neighborID)
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

int Featureneighbor::GetNeighbor(int testID, int topK, std::vector<int>& neighborID, std::vector<double>& neighborSimilarity)
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
	for (int i = 0; i < topK; i++) neighborID.push_back(mNeighbor[temptestID][i]);
	for (int i = 0; i < topK; i++) neighborSimilarity.push_back(mSimilarity[temptestID][i]);
	return 0;
}

int Featureneighbor::LoadBin(std::string fileName, int printLog)
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

int Featureneighbor::SaveBin(std::string fileName, int printLog)
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