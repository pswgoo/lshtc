#include "common/common_basic.h"
#include "lshtc_lib/lshtc_data.h""
#include "extractfeature/feature.h"
#include "classifier/classifier.h"
#include "evaluation/evaluator.h"
#include "evaluation/tools.h"
#include "medlinetools/medlinetoolfunction.h"
#include <omp.h>
using namespace std;

int gMode = 1;
int gOption = 0;

//only unigram feature,predictscore saved by model, scores predicted by one model save in one line
int SavePredictScore(string tokenPath, string uniGramFile, string labelFreqFile, string modelPath, const int modelNum, string scoreFilePath)
{
	int rtn = 0;

	clog << "Loading Tokenization Result" << endl;
	LhtcDocumentSet tokenDocuments;
	rtn = tokenDocuments.LoadBin(tokenPath.c_str(), STATUS_ONLY);//"pratest_6020.bin"
	CHECK_RTN(rtn);

	clog << "Load Unigram Dictionary" << endl;
	UniGramFeature uniGrams;
	rtn = uniGrams.Load(uniGramFile.c_str());
	CHECK_RTN(rtn);
	clog << "Total " << uniGrams.mDictionary.size() << " unigrams" << endl;

	clog << "Load Label Frequence" << endl;
	map<int, double> labelFreq;
	rtn = LoadLabelFreq(labelFreqFile.c_str(), labelFreq);
	CHECK_RTN(rtn);

	vector<pair<int, double> > meshSort;
	for (map<int, double>::iterator it = labelFreq.begin(); it != labelFreq.end(); ++it)
		meshSort.push_back(make_pair(it->first, it->second));
	sort(meshSort.begin(), meshSort.end(), CmpScore);

	vector<int> modelIds;
	modelIds.clear();
	for (size_t i = 0; i < (size_t)modelNum && i < meshSort.size(); ++i)
	{
		string modelFile = modelPath + "/" + intToString(meshSort[i].first) + ".model";
		if (FileExist(modelFile))
		{
			modelIds.push_back(meshSort[i].first);
		}
	}
	clog << modelIds.size() << " Models Available" << endl;

	vector<LhtcDocument*> tokenDocVector;
	tokenDocVector.reserve(tokenDocuments.Size());
	for (map<int, LhtcDocument>::iterator it = tokenDocuments.mLhtcDocuments.begin(); it != tokenDocuments.mLhtcDocuments.end(); it++)
	{
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

	vector<int> pmids;
	for (map<int, LhtcDocument>::iterator it = tokenDocuments.mLhtcDocuments.begin(); it != tokenDocuments.mLhtcDocuments.end(); ++it)
		pmids.push_back(it->first);

	if (pmids.size() != (size_t)allFeatures.Size())
	{
		clog << "Error: pmids.size != allFeatures.size" << endl;
		return -1;
	}

	//clog << "Free Memory" << endl;
	//tokenCitations.~TokenCitationSet();

	FILE * outScoreFile = fopen(scoreFilePath.c_str(), "wb");
	if (outScoreFile == NULL)
		return -1;
	clog << "Start Predict" << endl;
	int numThreads = omp_get_num_procs();
	omp_set_num_threads(numThreads);
	rtn = Write(outScoreFile, modelIds.size());//(size_t)modelIds.size()
	CHECK_RTN(rtn);
	for (unsigned int k = 0; k < modelIds.size(); k++)
	{
		if ((k & 255) == 0)
		{
			clog << "LOG : Working for model " << modelIds[k] << endl;
		}
		string modelFile = modelPath + "/" + intToString(modelIds[k]) + ".model";
		LinearMachine linearMachine;
		rtn = linearMachine.Load(modelFile);
		CHECK_RTN(rtn);
		pair<int, vector<pair<int, double>>> modelScore;
		modelScore.first = modelIds[k];
		modelScore.second.resize(allFeatures.Size());
#pragma omp parallel for schedule(dynamic)
		for (int i = 0; i < allFeatures.Size(); i++)
		{
			double tmpScore;
			modelScore.second[i].first = pmids[i];
			linearMachine.Predict(allFeatures[i], modelScore.second[i].second);
		}
		rtn = Write(outScoreFile, modelScore);
		CHECK_RTN(rtn);
	}
	fclose(outScoreFile);
	outScoreFile = NULL;
	clog << "Save Complete" << endl;
	return 0;
}

int main(char argc, char** argv)
{
	int rtn = 0;
	//rtn = SavePredictScore("../data/loc_normscore_set.bin", "lshtc_unigram_dictionary_loctrain.bin", "lshtc_label_freqence.bin", "../models_1109", 25000, "../store_tables/loc_normscore_predictscores_97w.bin");
	rtn = InitializePrecisionScoreTableSet("../store_tables/loc_normscore_predictscores_97w.bin", "../data/loc_normscore_set.bin", "../store_tables/scoretable_precision_1109.bin");
	CHECK_RTN(rtn);
	return 0;
}
