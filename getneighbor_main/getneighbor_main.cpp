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
	UniGramFeature uniGrams;
	string trainsetFile = "../data/loc_train.bin";
	string testsetFile = "../data/loc_test.bin";
	vector<Feature> lshtcTrainFeatureSet, lshtcTestFeatureSet;
	vector<int> lshtcTrainFeatureID, lshtcTestFeatureID;
	Feature tempFeature;
	lshtcTrainFeatureSet.clear();
	lshtcTestFeatureSet.clear();
	lshtcTrainFeatureID.clear();
	lshtcTestFeatureID.clear();

	clog << "Load Unigram Dictionary" << endl;
	rtn = uniGrams.Load("lshtc_unigram_dictionary_loctrain.bin");
	CHECK_RTN(rtn);
	clog << "Total " << uniGrams.mDictionary.size() << " unigrams" << endl;
	
	rtn = lshtcTrainSet.LoadBin(trainsetFile, FULL_LOG);
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
		uniGrams.ExtractLhtc(*vecTrainDocument[i], allTrainFeatures.mFeatures[i]);
		if (allTrainFeatures.mFeatures[i].size() == 0) printf("%d Warning!!\n", i);
	}
	allTrainFeatures.Normalize();//get traindata feature

	rtn = lshtcTestSet.LoadBin(testsetFile, FULL_LOG);
	CHECK_RTN(rtn);

	int testSize = (int)lshtcTestSet.Size();
	for (std::map<int, LhtcDocument>::iterator it = lshtcTestSet.mLhtcDocuments.begin(); it != lshtcTestSet.mLhtcDocuments.end(); ++it)
		lshtcTestFeatureID.push_back(it->first);

	vector<LhtcDocument*> vecTestDocument;
	vecTestDocument.reserve(lshtcTestSet.Size());
	for (map<int, LhtcDocument>::iterator it = lshtcTestSet.mLhtcDocuments.begin(); it != lshtcTestSet.mLhtcDocuments.end(); ++it)
		vecTestDocument.push_back(&(it->second));

	clog << "Prepare for Extract Features" << endl;
	FeatureSet allTestFeatures;
	allTestFeatures.mFeatures.resize(vecTestDocument.size());
#pragma omp parallel for schedule(dynamic)
	for (int i = 0; i < (int)vecTestDocument.size(); i++)
	{
		uniGrams.ExtractLhtc(*vecTestDocument[i], allTestFeatures.mFeatures[i]);
		if (allTestFeatures.mFeatures[i].size() == 0) printf("%d Warning!!\n", i);
	}
	allTestFeatures.Normalize();//get testdata feature

	int sigSize = allTestFeatures.Size() / 5;
	for (int i = 0; i < 5; ++i)
	{
		string filename = "../data/lshtc_neighbor" + intToString(i) + ".bin";
		if (FileExist(filename))
			continue;
		clog << i << "th, sigSize = " << sigSize << endl;
		FeatureSet locFeatures;
		vector<int> locIds;
		for (int j = sigSize*i; j < sigSize*(i + 1); ++j)
		{
			locFeatures.AddInstance(allTestFeatures[j]);
			locIds.push_back(lshtcTestFeatureID[j]);
		}
		FeatureNeighbor featureneighbor;
		rtn = featureneighbor.Build(allTrainFeatures.mFeatures, allTestFeatures.mFeatures, lshtcTrainFeatureID, lshtcTestFeatureID);
		CHECK_RTN(rtn);

		rtn = featureneighbor.SaveBin(filename, STATUS_ONLY);
		CHECK_RTN(rtn);
		clog << "Save bin completed" << endl;
	}
	/*//Ä£¿é²âÊÔ
	rtn = featureneighbor.LoadBin("lshtc_neighbor.bin", STATUS_ONLY);
	CHECK_RTN(rtn);
	clog << "Load bin completed" << endl;

	vector<int> testNeighbor;
	vector<double> testSimilarity;
	int testtopK = 300;
	int testNum = 50;
	FILE* outFile = fopen("neighbor.txt", "w");
	for (int i = 0; i < testNum; ++i)
	{
		rtn = featureneighbor.GetNeighbor(lshtcTestFeatureID[i], testtopK, testNeighbor, testSimilarity);
		CHECK_RTN(rtn);
		fprintf(outFile, "%d", lshtcTestFeatureID[i]);
		for (int j = 0; j < testtopK; j++)
			fprintf(outFile, " %d:%lf", testNeighbor[j], testSimilarity[j]);
		fprintf(outFile, "\n");
	}
	fclose(outFile);

	map<int, int> indexs;
	int cur = 0;
	for (auto it = lshtcTrainSet.mLhtcDocuments.begin(); it != lshtcTrainSet.mLhtcDocuments.end(); ++it)
		indexs[it->first] = cur++;
	map<int, int> indexs2;
	cur = 0;
	for (auto it = lshtcTestSet.mLhtcDocuments.begin(); it != lshtcTestSet.mLhtcDocuments.end(); ++it)
		indexs2[it->first] = cur++;
	outFile = fopen("feature_tmp.txt", "w");
	vector<int> list = { 147, 238 };
	for (int i = 0; i < list.size(); ++i)
	{
		fprintf(outFile, "%d", list[i]);
		for (auto it = allTestFeatures.mFeatures[indexs2[list[i]]].begin(); it != allTestFeatures.mFeatures[indexs2[list[i]]].end(); ++it)
			fprintf(outFile, " %d:%lf", it->first, it->second);
		fprintf(outFile, "\n");
		rtn = featureneighbor.GetNeighbor(list[i], 10, testNeighbor);
		CHECK_RTN(rtn);
		for (int j = 0; j < testNeighbor.size(); ++j)
		{
			fprintf(outFile, "%d", testNeighbor[j]);
			for (auto it = allTrainFeatures.mFeatures[indexs[testNeighbor[j]]].begin(); it != allTrainFeatures.mFeatures[indexs[testNeighbor[j]]].end(); ++it)
				fprintf(outFile, " %d:%lf", it->first, it->second);
			fprintf(outFile, "\n");
		}
		fprintf(outFile, "\n\n");
	};
	fclose(outFile);
	*/
	clog << "Completed" << endl;
	return 0;
}
