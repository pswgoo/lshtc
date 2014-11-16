#include "lshtc_lib/lshtc_data.h"
#include "common/file_utility.h"
#include "common/common_basic.h"
#include "extractfeature/feature.h"
#include "classifier/classifier.h"
#include "classifier/mylinear.h"
#include "extractfeature/feature.h"
#include "classifier/classifier.h"
#include "evaluation/tools.h"
#include "tokenization/tokenization.h"
#include <fstream>
#include <iostream>
using namespace std;

int GetTotalTokenNum()
{
	int rtn = 0;
	LhtcDocumentSet testLhtc;
	std::clog << "Load LhtcDocumentSet" << std::endl;
	rtn = testLhtc.Load("train.txt");
	//rtn = testLhtc.Load("train_top100.txt");
	//while (1);
	CHECK_RTN(rtn);
	std::clog << "Load LhtcDocumentSet Successful" << std::endl;
	std::set<int> appear;
	appear.clear();
	for (std::map<int, LhtcDocument>::iterator iter = testLhtc.mLhtcDocuments.begin(); iter != testLhtc.mLhtcDocuments.end(); ++iter)
	for (std::map<int, double>::iterator it = iter->second.mTf.begin(); it != iter->second.mTf.end(); ++it)
		appear.insert(it->first);
	printf("%d\n", appear.size());
	
	return 0;
}

int SplitTrainSet()
{
	int rtn = 0;
	const int LOC_TEST_NUM = 400000;

	LhtcDocumentSet testLhtc;
	std::clog << "Load LhtcDocumentSet" << std::endl;
	rtn = testLhtc.Load("../data/train.txt");
	CHECK_RTN(rtn);

	clog << "Load validtrain sorted pmid meshs" << endl;
	vector<pair<int, set<int>>> vecMeshs;
	FileBuffer buffer("lshtc_sorted_id_labels_file.bin");
	rtn = buffer.GetNextData(vecMeshs);
	CHECK_RTN(rtn);
	clog << "Total load " << vecMeshs.size() << " pmids" << endl;

	set<int> ids;
	LhtcDocumentSet locTrain;
	LhtcDocumentSet locTest;
	for (size_t i = 0; i < vecMeshs.size(); ++i)
	{
		ids.insert(vecMeshs[i].first);
		if (i < LOC_TEST_NUM)
			locTest.mLhtcDocuments[testLhtc.mLhtcDocuments[vecMeshs[i].first].mId] = testLhtc.mLhtcDocuments[vecMeshs[i].first];
		else
			locTrain.mLhtcDocuments[testLhtc.mLhtcDocuments[vecMeshs[i].first].mId] = testLhtc.mLhtcDocuments[vecMeshs[i].first];
	}
	clog << "Total " << ids.size() << " sample ids" << endl;

	rtn = testLhtc.SaveBin("../data/train.bin", FULL_LOG);
	CHECK_RTN(rtn);

	rtn = locTest.SaveBin("../data/loc_test.bin", FULL_LOG);;
	CHECK_RTN(rtn);
	clog << "Save " << locTest.Size() << " test instances" << endl;
	rtn = locTrain.SaveBin("../data/loc_train.bin", FULL_LOG);;
	CHECK_RTN(rtn);
	clog << "Save " << locTrain.Size() << " train instances" << endl;
	return 0;
}

int PrintFeature()
{
	int rtn = 0;

	clog << "Load features" << endl;
	size_t featureSize;
	feature_node** featureSpace = NULL;
	feature_node* nodeSpace = NULL;
	rtn = LinearMachine::LoadFeatureNode(featureSpace, nodeSpace, featureSize, "lshtc_sorted_loctrain_feature.bin");
	CHECK_RTN(rtn);
	clog << "Load " << featureSize << " train features" << endl;

	clog << "Load validtrain sorted pmid meshs" << endl;
	vector<pair<int, set<int>>> vecMeshs;
	FileBuffer buffer("lshtc_sorted_id_labels_file_loctrain.bin");
	rtn = buffer.GetNextData(vecMeshs);
	CHECK_RTN(rtn);
	clog << "Total load " << vecMeshs.size() << " pmids" << endl;

	LhtcDocumentSet lshtcSet;
	rtn = lshtcSet.LoadBin("../data/loc_train.bin",FULL_LOG);
	CHECK_RTN(rtn);

	FILE *outTF = fopen("ori_tf.txt", "w");
	FILE *outFeature = fopen("feature_tmp.txt", "w");
	for (int i = 0; i < 100; ++i)
	{
		for (set<int>::iterator it = vecMeshs[i].second.begin(); it != vecMeshs[i].second.end(); ++it)
		if (it == vecMeshs[i].second.begin())
			fprintf(outFeature, "%d", *it);
		else
			fprintf(outFeature, ",%d", *it);
		feature_node* ptr = featureSpace[i];
		while (ptr->index != -1)
		{
			fprintf(outFeature, " %d:%lf", ptr->index, ptr->value);
			++ptr;
		}
		fprintf(outFeature, "\n");

		rtn = lshtcSet[vecMeshs[i].first].Save(outTF);
		CHECK_RTN(rtn);
	}
	fclose(outFeature);
	fclose(outTF);

	rtn = SmartFree(featureSpace);
	CHECK_RTN(rtn);
	rtn = SmartFree(nodeSpace);
	return 0;
}

int PrintUnigramDict()
{
	int rtn = 0;
	UniGramFeature unigram;
	rtn = unigram.Load("lshtc_unigram_dictionary_loctrain.bin");
	CHECK_RTN(rtn);

	FILE *outFile = fopen("unigramdict.csv","w");
	for (map<int, UniGram>::iterator it = unigram.mDictionary.begin(); it != unigram.mDictionary.end(); ++it)
	{
		fprintf(outFile, "%d,%d,%d\n", it->first, it->second.mFirst, it->second.mAppear);
	}

	fclose(outFile);
	return 0;
}

int TestNumlabelModel()
{
	int rtn = 0;
	string tokenFile = "../data/loc_test.bin";
	string unigramFile = "lshtc_unigram_dictionary_loctrain.bin";
	string modelFile = "../models_1109/numlabel_1109.model";

	clog << "Loading Tokenization Result" << endl;
	LhtcDocumentSet tokenDocuments;
	rtn = tokenDocuments.LoadBin(tokenFile, FULL_LOG);
	CHECK_RTN(rtn);

	clog << "Load Unigram Dictionary" << endl;
	UniGramFeature uniGrams;
	rtn = uniGrams.Load(unigramFile.c_str());
	CHECK_RTN(rtn);
	clog << "Total " << uniGrams.mDictionary.size() << " unigrams" << endl;

	vector<int> meshNum;
	vector<LhtcDocument*> tokenDocVector;
	tokenDocVector.reserve(tokenDocuments.Size());
	for (map<int, LhtcDocument>::iterator it = tokenDocuments.mLhtcDocuments.begin(); it != tokenDocuments.mLhtcDocuments.end(); it++)
	{
		meshNum.push_back((int)it->second.mLabels.size());
		tokenDocVector.push_back(&(it->second));
	}

	clog << "Prepare for Extract Features" << endl;
	FeatureSet allFeatures;
	allFeatures.mFeatures.resize(tokenDocVector.size());
#pragma omp parallel for schedule(dynamic)
	for (int i = 0; i < (int)tokenDocVector.size(); i++)
	{
		uniGrams.ExtractLhtc(*tokenDocVector[i], allFeatures.mFeatures[i]);
	}
	allFeatures.Normalize();

	vector<double> goldLabels;
	goldLabels.reserve(meshNum.size());
	double goldSum = 0.0;
	for (size_t i = 0; i < meshNum.size(); i++)
	{
		goldLabels.push_back((double)meshNum[i]);
		goldSum += meshNum[i];
	}

	vector<double> labels;
	LinearMachine machine;
	clog << "Load Num Label Model to " + modelFile << endl;
	rtn = machine.Load(modelFile);
	CHECK_RTN(rtn);
	labels.resize(allFeatures.Size());
#pragma omp parallel for schedule(dynamic)
	for (int i = 0; i < allFeatures.Size(); ++i)
		rtn = machine.Predict(allFeatures.mFeatures[i], labels[i]);

	vector<double> predictLabels;
	double predictSum = 0.0;
	for (size_t i = 0; i < labels.size(); ++i)
	{
		double num = round(labels[i]);
		if (num < 1.0)
			num = 1.0;
		predictLabels.push_back(num);
		predictSum += num;
	}

	double pearson = 0.0;
	rtn = GetPearsonCorrelation(goldLabels, predictLabels, pearson);
	CHECK_RTN(rtn);

	double instanceNum = (double)predictLabels.size();
	cout << "Pearson = " << pearson << endl;
	cout << "Avg_gold = " << goldSum / instanceNum << ", Avg_predict = " << predictSum / instanceNum << endl;
	clog << "Completed" << endl;
	return 0;
}

int SaveScoreTableDataSet()
{
	const int DATASET_SIZE = 970000;
	const string pmidMeshFile = "lshtc_sorted_id_labels_file_loctrain.bin";

	int rtn = 0;
	clog << "Load validtrain sorted pmid meshs" << endl;
	vector<pair<int, set<int>>> vecMeshs;
	FileBuffer buffer(pmidMeshFile.c_str());
	rtn = buffer.GetNextData(vecMeshs);
	CHECK_RTN(rtn);
	clog << "Total load " << vecMeshs.size() << " pmids" << endl;

	LhtcDocumentSet lshtcSet;
	rtn = lshtcSet.LoadBin("../data/loc_train.bin", FULL_LOG);//load lhtcdocumentset
	CHECK_RTN(rtn);

	LhtcDocumentSet scoreTabDataSet;
	for (int i = 0; i < DATASET_SIZE; ++i)
	{
		if (i >= vecMeshs.size())
			break;
		int index = vecMeshs[vecMeshs.size() - i - 1].first;
		scoreTabDataSet.mLhtcDocuments[lshtcSet.mLhtcDocuments[index].mId] = lshtcSet.mLhtcDocuments[index];
	}

	rtn = scoreTabDataSet.SaveBin("../data/loc_normscore_set.bin", FULL_LOG);
	CHECK_RTN(rtn);

	clog << "Save norm score set completed" << endl;
	return 0;
}

int main()
{
	int rtn = 0;
	//rtn = SplitTrainSet();
	rtn = SaveScoreTableDataSet();
	CHECK_RTN(rtn);
	clog << "Complete" << endl;
	//system("pause");
	return 0;
}