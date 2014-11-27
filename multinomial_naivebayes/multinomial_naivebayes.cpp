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

	Read(infile, mInstanceSize);
	CHECK_RTN(rtn);

	Read(infile, mAppearLabels);
	CHECK_RTN(rtn);

	Read(infile, mAppearFeatures);
	CHECK_RTN(rtn);

	Read(infile, mTransLabels);
	CHECK_RTN(rtn);

	Read(infile, mTransFeatures);
	CHECK_RTN(rtn);

	Read(infile, mPossiblity);
	CHECK_RTN(rtn);

	Read(infile, mInverseTable);
	CHECK_RTN(rtn);

	Read(infile, mLabelsWordCnt);
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

	Write(outfile, mInstanceSize);
	CHECK_RTN(rtn);

	Write(outfile, mAppearLabels);
	CHECK_RTN(rtn);

	Write(outfile, mAppearFeatures);
	CHECK_RTN(rtn);

	Write(outfile, mTransLabels);
	CHECK_RTN(rtn);

	Write(outfile, mTransFeatures);
	CHECK_RTN(rtn);

	Write(outfile, mPossiblity);
	CHECK_RTN(rtn);

	Write(outfile, mInverseTable);
	CHECK_RTN(rtn);

	Write(outfile, mLabelsWordCnt);
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

	mPossiblity.resize(tempLabelID);
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
				iter = mPossiblity[tempLabelID].find(tempFeature);
				if (iter == mPossiblity[tempLabelID].end())
					mPossiblity[tempLabelID][tempFeature] = tempValue;
				else
					mPossiblity[tempLabelID][tempFeature] += tempValue;

				mInverseTable[tempFeatureID].insert(tempLabel);
				mLabelsWordCnt[tempLabelID] += tempValue;
			}
		}
		if (printLog != SILENT)
			if (!((i + 1) & 131071)) printf("%d Instances To Count\n", i);
	}//get Possiblity & InverseTable & LabelsWordCnt
	if (printLog != SILENT)
		printf("%d Instances To Count\n", mInstanceSize);
	return 0;
}

int MultinomialNaiveBayes::Predict(const Feature& testInstance, std::vector<int>& labelID, int topK)
{
	double Pm, Pmn;
	std::vector<double> Wn;
	std::vector<int> labelList;
	int featureSize;
	typedef std::priority_queue<std::pair<double, int>, std::vector<std::pair<double, int> >, std::greater<std::pair<double, int>>> Priority_Queue;
	Priority_Queue heap;

	featureSize = 0;
	Wn.clear();
	labelList.clear();
	labelID.clear();

	for (Feature::const_iterator it = testInstance.begin(); it != testInstance.end(); ++it)
		if (it->second > 1e-6)
			featureSize++;//get S(Wu)

	for (Feature::const_iterator it = testInstance.begin(); it != testInstance.end(); ++it)
	{
		if (mTransFeatures.find(it->first) == mTransFeatures.end())
			continue;//valid
		double temp = 0;
		int tempFeatureID = mTransFeatures.find(it->first)->second;
		temp = log(std::max(1.0, double(mInstanceSize) / double(mAppearFeatures[tempFeatureID]) - 1.0));//log[max(1,D/Dn-1)]
		temp *= log(1. + it->second);
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
		for (Feature::const_iterator iter = testInstance.begin(); iter != testInstance.end(); ++iter)
		{
			int tempFeature = iter->first;
			if (mTransFeatures.find(tempFeature) == mTransFeatures.end())
				continue;//valid
			if (mPossiblity[tempLabelID].find(tempFeature) != mPossiblity[tempLabelID].end())//get Pmn
				Pmn = (1.0 + 1.0 * mPossiblity[tempLabelID].find(tempFeature)->second) / (mLabelsWordCnt[tempLabelID] + totFeatures);
			else
				Pmn = 1.0 / (mLabelsWordCnt[tempLabelID] + totFeatures);
			temp += Wn[featurePos++] * log(Pmn);//log(Pmn ^ Wn)
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
	std::vector<double> Wn;
	std::vector<int> labelList;
	int featureSize;
	typedef std::priority_queue<std::pair<double, int>, std::vector<std::pair<double, int> >, std::greater<std::pair<double, int>>> Priority_Queue;
	Priority_Queue heap;

	featureSize = 0;
	Wn.clear();
	labelList.clear();
	labelScore.clear();

	for (Feature::const_iterator it = testInstance.begin(); it != testInstance.end(); ++it)
		if (it->second > 1e-6)
			featureSize++;//get S(Wu)

	for (Feature::const_iterator it = testInstance.begin(); it != testInstance.end(); ++it)
	{
		if (mTransFeatures.find(it->first) == mTransFeatures.end())
			continue;//valid
		int tempFeatureID = mTransFeatures.find(it->first)->second;
		double temp = 0;
		temp = log(std::max(1.0, double(mInstanceSize) / double(mAppearFeatures[tempFeatureID]) - 1.0));//log[max(1,D/Dn-1)]
		temp *= log(1 + it->second);
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
		for (Feature::const_iterator iter = testInstance.begin(); iter != testInstance.end(); ++iter)
		{
			int tempFeature = iter->first;
			if (mTransFeatures.find(tempFeature) == mTransFeatures.end())
				continue;//valid
			if (mPossiblity[tempLabelID].find(tempFeature) != mPossiblity[tempLabelID].end())//get Pmn
				Pmn = (1.0 + 1.0 * mPossiblity[tempLabelID].find(tempFeature)->second) / (mLabelsWordCnt[tempLabelID] + totFeatures);
			else
				Pmn = 1.0 / (mLabelsWordCnt[tempLabelID] + totFeatures);
			temp += Wn[featurePos++] * log(Pmn);//log(Pmn ^ Wn)
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