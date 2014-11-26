#include "lshtc_lib/lshtc_data.h"
#include "getneighbor/getneighbor.h"
#include "common/file_utility.h"
#include "common/common_basic.h"
#include "extractfeature/feature.h"
#include "metalabelnewtrain/metalabelnewtrain_lhtsc.h"
#include "getneighbor/ml_knn.h"
#include <fstream>
#include <iostream>
using namespace std;

int SaveNeighbor()
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
		rtn = featureneighbor.Build(allTrainFeatures.mFeatures, locFeatures.mFeatures, lshtcTrainFeatureID, locIds);
		CHECK_RTN(rtn);

		rtn = featureneighbor.SaveBin(filename, STATUS_ONLY);
		CHECK_RTN(rtn);
		clog << "Save bin completed" << endl;
	}

	return 0;
}

int NeighborTest()
{
	int rtn = 0;
	string neighborFile = "../data/lshtc_neighbor1.bin";
	string lshtcFile = "../data/loc_test_merge01.bin";

	LhtcDocumentSet lshtcSet;
//	rtn = lshtcSet.LoadBin(lshtcFile);
	CHECK_RTN(rtn);

	FeatureNeighbor neighbor;
	rtn = neighbor.LoadBin(neighborFile);
	CHECK_RTN(rtn);

	vector<int> docIds = { 708827, 738745, 768394, 797724, 827088, 856201, 885858, 915454 };
	//for (map<int, LhtcDocument>::iterator it = lshtcSet.mLhtcDocuments.begin(); it != lshtcSet.mLhtcDocuments.end(); ++it)
		//docIds.push_back(it->first);

	FILE* outFile = fopen("neighbor_tmp1.txt", "w");
	int cur = 0;
	for (size_t i = 0; i < docIds.size(); ++i)
	{
		vector<int> neighbors;
		vector<double> similarities;
		rtn = neighbor.GetNeighbor(docIds[i], 100, neighbors, similarities);
		CHECK_RTN(rtn);
		
		if (cur % 1 == 0)
		{
			fprintf(outFile, "%d", docIds[i]);
			for (int j = 0; j < neighbors.size(); ++j)
			{
				fprintf(outFile, " %d:%lf", neighbors[j], similarities[j]);
			}
			fprintf(outFile, "\n");
		}
		++cur;
	}
	fclose(outFile);

	/*
	FeatureNeighbor featureneighbor;
	rtn = featureneighbor.LoadBin(neighborFile, STATUS_ONLY);
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
	clog << "Test complete" << endl;
	return 0;
}

int Merge()
{
	int rtn = 0;
	FeatureNeighbor neighbor1;
	FeatureNeighbor neighbor2;
	FeatureNeighbor neighbor3;
	rtn = neighbor1.LoadBin("../data/lshtc_neighbor2.bin");
	CHECK_RTN(rtn);
	rtn = neighbor2.LoadBin("../data/lshtc_neighbor3.bin");
	CHECK_RTN(rtn);
	rtn = neighbor3.LoadBin("../data/lshtc_neighbor4.bin");
	CHECK_RTN(rtn);

	neighbor1.Merge(neighbor2);
	neighbor1.Merge(neighbor3);
	rtn = neighbor1.SaveBin("../data/lshtc_neighbor_merge234.bin");
	CHECK_RTN(rtn);

	LhtcDocumentSet locTest;
	rtn = locTest.LoadBin("../data/loc_test.bin", STATUS_ONLY);
	CHECK_RTN(rtn);

	const int FIRST_NUM = 160000;
	LhtcDocumentSet first2;
	int cnt = 0;
	for (map<int, LhtcDocument>::iterator it = locTest.mLhtcDocuments.begin(); it != locTest.mLhtcDocuments.end(); ++it, ++cnt)
	{
		if (cnt < FIRST_NUM)
			continue;
		first2.mLhtcDocuments[it->first] = it->second;
	}

	rtn = first2.SaveBin("../data/loc_test_merge234.bin");
	CHECK_RTN(rtn);
	
	clog << "Merge complete" << endl;
	return 0;
}

int main()
{
	int rtn = 0;
	//rtn = Merge();

	map<int, vector<int>> train_labels;
	map<int, vector<int>> test_labels;
	LhtcDocumentSet train_set;
	train_set.LoadBin("../data/loc_train.bin", STATUS_ONLY);
	for (auto it = train_set.mLhtcDocuments.begin(); it != train_set.mLhtcDocuments.end(); ++it)
		train_labels[it->first] = it->second.mLabels;
	LhtcDocumentSet test_set;
	test_set.LoadBin("../data/loc_test_merge234.bin");
	for (auto it = test_set.mLhtcDocuments.begin(); it != test_set.mLhtcDocuments.end(); ++it)
		test_labels[it->first] = it->second.mLabels;

	FeatureNeighbor neighbors;
	neighbors.LoadBin("../data/lshtc_neighbor_merge234.bin", FULL_LOG);

	MultiLabelKnn ml_knn;
	rtn = ml_knn.Initialize(train_labels, test_labels, neighbors);
	CHECK_RTN(rtn);

	rtn = ml_knn.Save("ml_knn_merge234_k3.bin", FULL_LOG);
	CHECK_RTN(rtn);
	//rtn = ml_knn.Load("ml_knn_merge234.bin", FULL_LOG);
	//ml_knn.Save("tmp.bin", FULL_LOG);
	clog << "Completed" << endl;
	return 0;
}
