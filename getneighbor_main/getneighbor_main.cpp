#include "lshtc_lib/lshtc_data.h"
#include "getneighbor/getneighbor.h"
#include "common/file_utility.h"
#include "common/common_basic.h"
#include "extractfeature/feature.h"
#include "metalabelnewtrain/metalabelnewtrain_lhtsc.h"
#include <fstream>
#include <iostream>
using namespace std;

int main()
{
	int rtn = 0;
	LhtcDocumentSet lshtcTrainSet, lshtcTestSet;
	UniGramFeature trainUniGrams, testUniGrams;
	string trainsetFile = "F:\\lshtc\\data\\loc_train.bin";
	string testsetFile = "F:\\lshtc\\data\\loc_test.bin";
	vector<Feature> lshtcTrainFeatureSet, lshtcTestFeatureSet;
	vector<int> lshtcTrainFeatureID, lshtcTestFeatureID;
	Feature tempFeature;
	lshtcTrainFeatureSet.clear();
	lshtcTestFeatureSet.clear();
	lshtcTrainFeatureID.clear();
	lshtcTestFeatureID.clear();
	
	rtn = lshtcTrainSet.LoadBin(trainsetFile, FULL_LOG);
	CHECK_RTN(rtn);

	rtn = trainUniGrams.BuildLhtc(lshtcTrainSet);
	CHECK_RTN(rtn);

	int trainSize = (int)lshtcTrainSet.Size();
	for (std::map<int, LhtcDocument>::iterator it = lshtcTrainSet.mLhtcDocuments.begin(); it != lshtcTrainSet.mLhtcDocuments.end(); ++it)
		lshtcTrainFeatureID.push_back(it->first);

	vector<LhtcDocument*> vecTrainDocument;
	vecTrainDocument.reserve(lshtcTrainSet.Size());
	for (map<int, LhtcDocument>::iterator it = lshtcTrainSet.mLhtcDocuments.begin(); it != lshtcTrainSet.mLhtcDocuments.end(); ++it)
		vecTrainDocument.push_back(&(it->second));

	clog << "Prepare for Extract Features" << endl;
	FeatureSet allTrainFeatures;
	allTrainFeatures.mFeatures.resize(vecTrainDocument.size());
#pragma omp parallel for schedule(dynamic)
	for (int i = 0; i < (int)vecTrainDocument.size(); i++)
	{
		trainUniGrams.ExtractLhtc(*vecTrainDocument[i], allTrainFeatures.mFeatures[i]);
		if (allTrainFeatures.mFeatures[i].size() == 0) printf("%d Warning!!\n", i);
	}
	allTrainFeatures.Normalize();//get traindata feature

	rtn = lshtcTestSet.LoadBin(testsetFile, FULL_LOG);
	CHECK_RTN(rtn);

	rtn = testUniGrams.BuildLhtc(lshtcTestSet);
	CHECK_RTN(rtn);

	int testSize = (int)lshtcTestSet.Size();
	for (std::map<int, LhtcDocument>::iterator it = lshtcTestSet.mLhtcDocuments.begin(); it != lshtcTestSet.mLhtcDocuments.end(); ++it)
		lshtcTestFeatureID.push_back(it->first);

	vector<LhtcDocument*> vecTestDocument;
	vecTrainDocument.reserve(lshtcTestSet.Size());
	for (map<int, LhtcDocument>::iterator it = lshtcTestSet.mLhtcDocuments.begin(); it != lshtcTestSet.mLhtcDocuments.end(); ++it)
		vecTestDocument.push_back(&(it->second));

	clog << "Prepare for Extract Features" << endl;
	FeatureSet allTestFeatures;
	allTestFeatures.mFeatures.resize(vecTestDocument.size());
#pragma omp parallel for schedule(dynamic)
	for (int i = 0; i < (int)vecTestDocument.size(); i++)
	{
		testUniGrams.ExtractLhtc(*vecTestDocument[i], allTestFeatures.mFeatures[i]);
		if (allTestFeatures.mFeatures[i].size() == 0) printf("%d Warning!!\n", i);
	}
	allTestFeatures.Normalize();//get testdata feature

	Featureneighbor featureneighbor;
	rtn = featureneighbor.Build(allTrainFeatures.mFeatures, allTestFeatures.mFeatures, lshtcTrainFeatureID, lshtcTestFeatureID);
	CHECK_RTN(rtn);

	vector<int> testNeighbor;
	vector<double> testSimilarity;
	int testtopK = 10;
	rtn = featureneighbor.GetNeighbor(lshtcTestFeatureID[0], testtopK, testNeighbor, testSimilarity);
	for (int i = 0; i < testtopK; i++) printf("\n%d %lf", testNeighbor[i], testSimilarity[i]);
	CHECK_RTN(rtn);
	
	rtn = featureneighbor.SaveBin("lshtc_neighbor.bin");
	CHECK_RTN(rtn);

	return 0;
}