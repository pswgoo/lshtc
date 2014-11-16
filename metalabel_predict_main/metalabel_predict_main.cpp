#include "medline/basic.h"
#include "extractfeature/feature.h"
#include "classifier/classifier.h"
#include "evaluation/tools.h"
#include "tokenization/tokenization.h"
#include <iostream>
#include <omp.h>
#include <algorithm>
using namespace std;

int MetalabelPredict(string tokenFile, string unigramFile, string numlabelModelFile, string scoreFile)
{
	int rtn = 0;

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

	clog << "Extract gold standard" << endl;
	MultiLabelAnswerSet goldStandard;
	for (map<int, LhtcDocument>::iterator it = tokenDocuments.mLhtcDocuments.begin(); it != tokenDocuments.mLhtcDocuments.end(); it++)
	{
		vector<string> labels;
		for (int i = 0; i < it->second.mLabels.size(); ++i)
		{
			string strId;
			rtn = DataToString(it->second.mLabels[i], strId);
			CHECK_RTN(rtn);
			labels.push_back(strId);
		}
		goldStandard.AddAnswer(MultiLabelAnswer(it->first, labels));
	}

	vector<double> labels;
	LinearMachine machine;
	clog << "Load Num Label Model from " + numlabelModelFile << endl;
	rtn = machine.Load(numlabelModelFile);
	CHECK_RTN(rtn);
	labels.resize(allFeatures.Size());
#pragma omp parallel for schedule(dynamic)
	for (int i = 0; i < allFeatures.Size(); ++i)
		rtn = machine.Predict(allFeatures.mFeatures[i], labels[i]);

	vector<double> predictLabelNum;
	for (size_t i = 0; i < labels.size(); ++i)
	{
		double num = round(labels[i]);
		if (num < 1.0)
			num = 1.0;
		predictLabelNum.push_back(num);
	}

	clog << "Load predict scores and get predict label" << endl;
	vector<vector<pair<int, double>>> predictLabels;
	predictLabels.resize(predictLabelNum.size());
	for (int i = 0; i < predictLabels.size(); ++i)
		predictLabels.reserve(predictLabelNum[i]+1);

	FILE * inScoreFile = fopen(scoreFile.c_str(), "rb");
	size_t modelNum = 0;
	rtn = Read(inScoreFile, modelNum);
	CHECK_RTN(rtn);
	for (int i = 0; i < (int)modelNum; ++i)
	{
		if ((i & 255) == 0)
		{
			clog << "LOG : Working for the " << i <<" model"<< endl;
		}
		pair<int, vector<pair<int, double>>> modelScore;
		rtn = Read(inScoreFile, modelScore);
		CHECK_RTN(rtn);

#pragma omp parallel for schedule(dynamic)
		for (int j = 0; j < modelScore.second.size(); ++j)
		{
			if (modelScore.second[j].first != goldStandard[j].mPmid)
				cerr << "Error: the doc id is not matched, id in modelScore is " << modelScore.second[j].first << ", another in goldstandard is " << goldStandard[j].mPmid << endl;
			vector<pair<int, double>>& refData = predictLabels[j];
			if (refData.size() < (size_t)predictLabelNum[j])
				refData.push_back(make_pair(modelScore.first, modelScore.second[j].second));
			else if (refData.rbegin()->second < modelScore.second[j].second)
			{
				*(refData.rbegin()) = make_pair(modelScore.first, modelScore.second[j].second);
			}

			int p = (int)refData.size() - 1;
			auto tmp = refData[p];
			while (p > 0 && tmp.second > refData[p - 1].second)
			{
				refData[p] = refData[p - 1];
				--p;
			}
			refData[p] = tmp;
		}
	}
	fclose(inScoreFile);

	clog << "Generate predict answer" << endl;
	MultiLabelAnswerSet predictAnswers;
	for (size_t i = 0; i < predictLabels.size(); ++i)
	{
		vector<string> labels;
		for (size_t j = 0; j < predictLabels[i].size(); ++j)
		{
			string strId;
			rtn = DataToString(predictLabels[i][j].first, strId);
			CHECK_RTN(rtn);
			labels.push_back(strId);
		}
		predictAnswers.AddAnswer(MultiLabelAnswer(goldStandard[i].mPmid, labels));
	}

	rtn = goldStandard.SaveJsonSet("lshtc_loc_test_gold.json");
	CHECK_RTN(rtn);
	rtn = predictAnswers.SaveJsonSet("lshtc_loc_test_predict.json");
	CHECK_RTN(rtn);
	clog << "Begin Evaluation" << endl;
	double precision = 0.0;
	double recall = 0.0;
	double f1 = 0.0;

	FILE *outEvaluate = fopen("lshtc_result.txt", "w");
	Evaluator evaluator;
	evaluator.LabelBasedMicroEvaluate(predictAnswers, goldStandard, precision, recall, f1);
	printf("micro: MIP=%lf,MIR=%lf,MIF=%lf\n", precision, recall, f1);
	fprintf(outEvaluate, "micro: MIP=%lf,MIR=%lf,MIF=%lf\n", precision, recall, f1);

	evaluator.ExampleBasedEvaluate(predictAnswers, goldStandard, precision, recall, f1, "./lshtc_example_analyse.csv");
	printf("examp: EBP=%lf,EBR=%lf,EBF=%lf\n", precision, recall, f1);
	fprintf(outEvaluate, "examp: EBP=%lf,EBR=%lf,EBF=%lf\n", precision, recall, f1);

	evaluator.LabelBasedMacroEvaluate(predictAnswers, goldStandard, precision, recall, f1, "./lshtc_model_analyse.csv");//, "./ltr_model_analyse.csv"
	printf("MAP=%lf,MAR=%lf,MAF=%lf\n", precision, recall, f1);
	fprintf(outEvaluate, "MAP=%lf,MAR=%lf,MAF=%lf\n", precision, recall, f1);
	fclose(outEvaluate);

	clog << "Predict completed" << endl;
	return 0;
}

int main()
{
	int rtn = 0;
	rtn = MetalabelPredict("../data/loc_test.bin", "lshtc_unigram_dictionary_loctrain.bin", "../models_1109/numlabel_1109.model", "../store_tables/loc_test_predictscores.bin");
	CHECK_RTN(rtn);
	return 0;
}