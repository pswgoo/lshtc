#include "multinomial_naivebayes.h"
#include "common/file_utility.h"
#include <cmath>
#include <queue>
#include <functional>
#include <algorithm>
using namespace std;

MultinomialNaiveBayes::MultinomialNaiveBayes()
{
	Clear();
}

MultinomialNaiveBayes::~MultinomialNaiveBayes()
{
}

int MultinomialNaiveBayes::Clear()
{
	mInstanceSize = 0;
	mAppearLabels.clear();
	mAppearFeatures.clear();
	mTransLabels.clear();
	mTransFeatures.clear();
	mPossiblity.clear();
	mInverseTable.clear();
	mLabelsWordCnt.clear();
	return 0;
}

int MultinomialNaiveBayes::Load(std::string fileName, int printLog)
{
	int rtn = 0;
	FILE *infile = fopen(fileName.c_str(), "rb");

	rtn = Read(infile, mInstanceSize);
	CHECK_RTN(rtn);

	rtn = Read(infile, mAppearLabels);
	CHECK_RTN(rtn);

	rtn = Read(infile, mAppearFeatures);
	CHECK_RTN(rtn);

	rtn = Read(infile, mTransLabels);
	CHECK_RTN(rtn);

	rtn = Read(infile, mTransFeatures);
	CHECK_RTN(rtn);

	rtn = Read(infile, mPossiblity);
	CHECK_RTN(rtn);

	rtn = Read(infile, mInverseTable);
	CHECK_RTN(rtn);

	rtn = Read(infile, mLabelsWordCnt);
	CHECK_RTN(rtn);

	fclose(infile);

	if (printLog != SILENT)
		std::clog << "Load successful!" << std::endl;

	return 0;
}

int MultinomialNaiveBayes::Save(std::string fileName, int printLog)
{
	int rtn = 0;
	FILE *outfile = fopen(fileName.c_str(), "wb");

	rtn = Write(outfile, mInstanceSize);
	CHECK_RTN(rtn);

	rtn = Write(outfile, mAppearLabels);
	CHECK_RTN(rtn);

	rtn = Write(outfile, mAppearFeatures);
	CHECK_RTN(rtn);

	rtn = Write(outfile, mTransLabels);
	CHECK_RTN(rtn);

	rtn = Write(outfile, mTransFeatures);
	CHECK_RTN(rtn);

	rtn = Write(outfile, mPossiblity);
	CHECK_RTN(rtn);

	rtn = Write(outfile, mInverseTable);
	CHECK_RTN(rtn);

	rtn = Write(outfile, mLabelsWordCnt);
	CHECK_RTN(rtn);

	fclose(outfile);

	if (printLog != SILENT)
		std::clog << "Save successful!" << std::endl;

	return 0;
}

int MultinomialNaiveBayes::Build(const std::vector<std::map<int, double> >& trainSet, const std::vector<std::vector<int> >& trainLabels, int printLog)
{
	int tempFeatureID, tempLabelID;
	tempFeatureID = 0;
	tempLabelID = 0;

	mInstanceSize = (int)trainSet.size();
	for (int i = 0; i < mInstanceSize; i++)
	{
		for (std::map<int, double>::const_iterator it = trainSet[i].begin(); it != trainSet[i].end(); ++it)
		{
			int tempFeature;
			tempFeature = it->first;
			std::map<int, int>::iterator iter;
			iter = mTransFeatures.find(tempFeature);
			if (iter == mTransFeatures.end())
			{
				mTransFeatures[tempFeature] = tempFeatureID++;
				mAppearFeatures.push_back(1);
			}
			else
				mAppearFeatures[iter->second]++;
		}
		if (printLog != SILENT)
		if (!((i + 1) & 131071)) printf("%d Instances Load Features\n", i);
	}//get TransFeaturesID & AppearFeatures
	if (printLog != SILENT)
		printf("%d Instances Load Features\n", mInstanceSize);

	for (int i = 0; i < mInstanceSize; i++)
	{
		int tempLabelSize = (int)trainLabels[i].size();
		for (int j = 0; j < tempLabelSize; j++)
		{
			int tempLabel = trainLabels[i][j];
			std::map<int, int>::iterator iter;
			iter = mTransLabels.find(tempLabel);
			if (iter == mTransLabels.end())
			{
				mTransLabels[tempLabel] = tempLabelID++;
				mAppearLabels.push_back(1);
			}
			else
				mAppearLabels[iter->second]++;
		}
		if (printLog != SILENT)
		if (!((i + 1) & 131071)) printf("%d Instances Load Labels\n", i);
	}//get TransLabelID & AppearLabels
	if (printLog != SILENT)
		printf("%d Instances Load Labels\n", mInstanceSize);

	std::vector<std::map<int, int> > tempPossibility;;
	tempPossibility.resize(tempLabelID);
	mLabelsWordCnt.resize(tempLabelID);
	mInverseTable.resize(tempFeatureID);
	for (int i = 0; i < mInstanceSize; i++)
	{
		int tempLabelSize = (int)trainLabels[i].size();
		for (int j = 0; j < tempLabelSize; j++)
		{
			int tempLabel = trainLabels[i][j];
			int tempLabelID = mTransLabels.find(tempLabel)->second;
			for (std::map<int, double>::const_iterator it = trainSet[i].begin(); it != trainSet[i].end(); ++it)
			{
				int tempFeature = it->first;
				int tempValue = (int)it->second;
				int tempFeatureID = mTransFeatures.find(tempFeature)->second;
				std::map<int, int>::iterator iter;
				iter = tempPossibility[tempLabelID].find(tempFeatureID);
				if (iter == tempPossibility[tempLabelID].end())
					tempPossibility[tempLabelID][tempFeatureID] = tempValue;
				else
					tempPossibility[tempLabelID][tempFeatureID] += tempValue;

				mInverseTable[tempFeatureID].insert(tempLabel);
				mLabelsWordCnt[tempLabelID] += tempValue;
			}
		}
		if (printLog != SILENT)
		if (!((i + 1) & 131071)) printf("%d Instances To Count\n", i);
	}//get tempPossiblity & InverseTable & LabelsWordCnt

	if (printLog != SILENT)
		printf("%d Instances To Count\n", mInstanceSize);

	mPossiblity.resize(tempLabelID);
	for (int i = 0; i < tempLabelID; i++)
		for (std::map<int, int>::iterator it = tempPossibility[i].begin(); it != tempPossibility[i].end(); ++it)
			mPossiblity[i].push_back(*it);//get Possiblity

	if (printLog != SILENT)
		printf("Get Possiblity!\n");

	return 0;
}

int MultinomialNaiveBayes::Predict(const Feature& testInstance, std::vector<int>& labelID, int topK)
{
	double Pm, Pmn;
	int featureSize;
	std::vector<double> Wn;
	std::vector<int> labelList;
	std::vector<std::pair<int, double> > featureList;
	typedef std::priority_queue<std::pair<double, int>, std::vector<std::pair<double, int> >, std::greater<std::pair<double, int>>> Priority_Queue;
	Priority_Queue heap;

	featureList.clear();
	Wn.clear();
	labelList.clear();
	labelID.clear();

	for (Feature::const_iterator it = testInstance.begin(); it != testInstance.end(); ++it)
	if (it->second > 1e-6)
	{
		if (mTransFeatures.find(it->first) == mTransFeatures.end()) continue;
		int tempFeatureID = mTransFeatures.find(it->first)->second;
		featureList.push_back(std::make_pair(tempFeatureID, it->second));
	}
	featureSize = featureList.size();//get S(Wu) & get featureList/featureValue
	std::sort(featureList.begin(), featureList.end());

	if (!featureSize)
	{
		clog << "Don't exist valid feature!" << endl;
		return 1;
	}

	for (int i = 0; i < featureSize; i++)
	{
		double temp = 0;
		int tempFeatureID = featureList[i].first;
		temp = log(std::max(1.0, double(mInstanceSize) / double(mAppearFeatures[tempFeatureID]) - 1.0));//log[max(1,D/Dn-1)]
		temp *= log(1. + featureList[i].second);
		temp /= featureSize;//log[1+Wnu]/S(Wu)
		Wn.push_back(temp);//get Wn

		for (std::set<int>::iterator iter = mInverseTable[tempFeatureID].begin(); iter != mInverseTable[tempFeatureID].end(); ++iter)
		{
			int tmp = *iter;
			labelList.push_back(tmp);
		}
	}

	int totFeatures = (int)mAppearFeatures.size();
	int labelListSize = (int)labelList.size();
	std::sort(labelList.begin(), labelList.end());
	for (int i = 0; i < labelListSize; i++)
	{
		double temp = 0;
		if (i > 0 && labelList[i] == labelList[i - 1]) continue;
		int tempLabel = labelList[i];
		int tempLabelID = mTransLabels.find(tempLabel)->second;
		Pm = log(double(mAppearLabels[tempLabelID]) / double(mInstanceSize));//get Pm
		int featurePos = 0;
		int possiblityPos = 0;
		while (featurePos < featureSize)
		{
			if (possiblityPos >= mPossiblity[tempLabelID].size() || featureList[featurePos].first < mPossiblity[tempLabelID][possiblityPos].first)
			{
				Pmn = 1.0 / (mLabelsWordCnt[tempLabelID] + totFeatures);
				temp += Wn[featurePos] * log(Pmn);//log(Pmn ^ Wn)
				featurePos++;
			}
			else if (featureList[featurePos].first > mPossiblity[tempLabelID][possiblityPos].first)
				possiblityPos++;
			else
			{
				Pmn = (1.0 + 1.0 * mPossiblity[tempLabelID][possiblityPos].second) / (mLabelsWordCnt[tempLabelID] + totFeatures);
				temp += Wn[featurePos] * log(Pmn);//log(Pmn ^ Wn)
				featurePos++;
				possiblityPos++;
			}
		}
		temp += Pm;//get Pwm

		if (heap.size() < topK || heap.top().first < temp)
		{
			heap.push(std::make_pair(temp, tempLabel));
			while (heap.size() > topK)
				heap.pop();
		}//heap
	}

	int cur = (int)heap.size() - 1;
	labelID.resize(cur + 1);
	while (!heap.empty())
	{
		labelID[cur] = heap.top().second;
		--cur;
		heap.pop();
	}//get labelID

	return 0;
}

int MultinomialNaiveBayes::Predict(const Feature& testInstance, std::vector<std::pair<int, double> >& labelScore, int topK)
{
	double Pm, Pmn;
	int featureSize;
	std::vector<double> Wn;
	std::vector<int> labelList;
	std::vector<std::pair<int, double> > featureList;
	typedef std::priority_queue<std::pair<double, int>, std::vector<std::pair<double, int> >, std::greater<std::pair<double, int>>> Priority_Queue;
	Priority_Queue heap;

	featureList.clear();
	Wn.clear();
	labelList.clear();
	labelScore.clear();

	for (Feature::const_iterator it = testInstance.begin(); it != testInstance.end(); ++it)
	if (it->second > 1e-6)
	{
		if (mTransFeatures.find(it->first) == mTransFeatures.end()) continue;
		int tempFeatureID = mTransFeatures.find(it->first)->second;
		featureList.push_back(std::make_pair(tempFeatureID, it->second));
	}
	featureSize = featureList.size();//get S(Wu) & get featureList/featureValue
	std::sort(featureList.begin(), featureList.end());

	if (!featureSize)
	{
		clog << "Don't exist valid feature!" << endl;
		return 1;
	}

	for (int i = 0; i < featureSize; i++)
	{
		double temp = 0;
		int tempFeatureID = featureList[i].first;
		temp = log(std::max(1.0, double(mInstanceSize) / double(mAppearFeatures[tempFeatureID]) - 1.0));//log[max(1,D/Dn-1)]
		temp *= log(1. + featureList[i].second);
		temp /= featureSize;//log[1+Wnu]/S(Wu)
		Wn.push_back(temp);//get Wn

		for (std::set<int>::iterator iter = mInverseTable[tempFeatureID].begin(); iter != mInverseTable[tempFeatureID].end(); ++iter)
		{
			int tmp = *iter;
			labelList.push_back(tmp);
		}
	}

	int totFeatures = (int)mAppearFeatures.size();
	int labelListSize = (int)labelList.size();
	std::sort(labelList.begin(), labelList.end());
	for (int i = 0; i < labelListSize; i++)
	{
		double temp = 0;
		if (i > 0 && labelList[i] == labelList[i - 1]) continue;
		int tempLabel = labelList[i];
		int tempLabelID = mTransLabels.find(tempLabel)->second;
		Pm = log(double(mAppearLabels[tempLabelID]) / double(mInstanceSize));//get Pm
		int featurePos = 0;
		int possiblityPos = 0;
		while (featurePos < featureSize)
		{
			if (possiblityPos >= mPossiblity[tempLabelID].size() || featureList[featurePos].first < mPossiblity[tempLabelID][possiblityPos].first)
			{
				Pmn = 1.0 / (mLabelsWordCnt[tempLabelID] + totFeatures);
				temp += Wn[featurePos] * log(Pmn);//log(Pmn ^ Wn)
				featurePos++;
			}
			else if (featureList[featurePos].first > mPossiblity[tempLabelID][possiblityPos].first)
				possiblityPos++;
			else
			{
				Pmn = (1.0 + 1.0 * mPossiblity[tempLabelID][possiblityPos].second) / (mLabelsWordCnt[tempLabelID] + totFeatures);
				temp += Wn[featurePos] * log(Pmn);//log(Pmn ^ Wn)
				featurePos++;
				possiblityPos++;
			}
		}
		temp += Pm;//get Pwm

		if (heap.size() < topK || heap.top().first < temp)
		{
			heap.push(std::make_pair(temp, tempLabel));
			while (heap.size() > topK)
				heap.pop();
		}//heap
	}

	int cur = (int)heap.size() - 1;
	labelScore.resize(cur + 1);
	while (!heap.empty())
	{
		labelScore[cur].first = heap.top().second;
		labelScore[cur].second = heap.top().first;
		--cur;
		heap.pop();
	}//get labelScore
	
	return 0;
}