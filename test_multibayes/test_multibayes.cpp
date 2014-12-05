#include "multinomial_naivebayes/multinomial_naivebayes.h"
#include "lshtc_lib/lshtc_data.h"
using namespace std;

int LoadLabels(string fileName, vector<vector<int> >& labelSet, int printLog)
{
	int rtn = 0;
	FILE *infile = fopen(fileName.c_str(), "rb");

	rtn = Read(infile, labelSet);
	CHECK_RTN(rtn);

	fclose(infile);
	if (printLog != SILENT)
		std::clog << "Load Labels successful!" << std::endl;
	return 0;
}

int LoadFeatures(string fileName, vector<std::map<int, double> >& FeatureSet, int printLog)
{
	int rtn = 0;
	FILE *infile = fopen(fileName.c_str(), "rb");

	rtn = Read(infile, FeatureSet);
	CHECK_RTN(rtn);

	fclose(infile);
	if (printLog != SILENT)
		std::clog << "Load Features successful!" << std::endl;
	return 0;
}

int SaveLabels(string fileName, vector<vector<int> >& labelSet, int printLog)
{
	int rtn = 0;
	FILE *outfile = fopen(fileName.c_str(), "wb");

	rtn = Write(outfile, labelSet);
	CHECK_RTN(rtn);
	
	fclose(outfile);
	if (printLog != SILENT)
		std::clog << "Save Labels successful!" << std::endl;
	return 0;
}

int SaveFeatures(string fileName, vector<std::map<int, double> >& FeatureSet, int printLog)
{
	int rtn = 0;
	FILE *outfile = fopen(fileName.c_str(), "wb");

	rtn = Write(outfile, FeatureSet);
	CHECK_RTN(rtn);

	fclose(outfile);
	if (printLog != SILENT)
		std::clog << "Save Features successful!" << std::endl;
	return 0;
}

int main()
{
	int rtn = 0;
	LhtcDocumentSet trainLhtcSet, testLhtcSet;
	MultinomialNaiveBayes testMultinomial, testMultinomial_tmp;

	/*rtn = trainLhtcSet.LoadBin("F:\\lshtc\\data\\loc_test_merge234.bin", 1);
	rtn = trainLhtcSet.LoadBin("F:\\lshtc\\data\\loc_train.bin", 1);
	CHECK_RTN(rtn);*/

	/*rtn = testLhtcSet.LoadBin("F:\\lshtc\\data\\loc_test_merge01.bin", 1);
	CHECK_RTN(rtn);*/

	vector<map<int, double> > trainSet;
	vector<map<int, double> > testSet;
	vector<vector<int> > trainLabels;
	vector<vector<int> > testLabels;
	trainLabels.clear();
	trainSet.clear();
	testLabels.clear();
	testSet.clear();

	/*for (map<int, LhtcDocument>::iterator it = trainLhtcSet.mLhtcDocuments.begin(); it != trainLhtcSet.mLhtcDocuments.end(); ++it)
	{
		trainSet.push_back(it->second.mTf);
		trainLabels.push_back(it->second.mLabels);
	}//Deal trainSet*/

	/*for (map<int, LhtcDocument>::iterator it = testLhtcSet.mLhtcDocuments.begin(); it != testLhtcSet.mLhtcDocuments.end(); ++it)
	{
		testSet.push_back(it->second.mTf);
		testLabels.push_back(it->second.mLabels);
	}//Deal testSet*/

	/*rtn = SaveLabels("loc_trainset_labels.bin", trainLabels, 1);
	CHECK_RTN(rtn);
	
	rtn = SaveFeatures("loc_trainset_features.bin", trainSet, 1);
	CHECK_RTN(rtn);*/

	/*rtn = LoadLabels("loc_trainset_labels.bin", trainLabels, 1);
	CHECK_RTN(rtn);

	rtn = LoadFeatures("loc_trainset_features.bin", trainSet, 1);
	CHECK_RTN(rtn);*/

	/*rtn = SaveLabels("loc_testset_labels.bin", testLabels, 1);
	CHECK_RTN(rtn);

	rtn = SaveFeatures("loc_testset_features.bin", testSet, 1);
	CHECK_RTN(rtn);*/

	rtn = LoadLabels("loc_testset_labels.bin", testLabels, 1);
	CHECK_RTN(rtn);

	rtn = LoadFeatures("loc_testset_features.bin", testSet, 1);
	CHECK_RTN(rtn);

	/*printf("%d %d\n", clock(), trainLabels.size());
	rtn = testMultinomial.Build(trainSet, trainLabels, 1);
	CHECK_RTN(rtn);
	printf("%d\n", clock());

	rtn = testMultinomial.Save("Bayes_train_model_new.bin", 1);
	CHECK_RTN(rtn);*/

	rtn = testMultinomial.Load("Bayes_train_model_new.bin", 1);
	CHECK_RTN(rtn);

	//rtn = testMultinomial.Save("Bayes_train_model_tmp.bin", 1);
	//CHECK_RTN(rtn);

	printf("%d\n", testMultinomial.mInstanceSize);
	
	vector<vector<int> > labelID;
	vector<vector<int> > answerLabel;
	vector<vector<pair<int, double>>> labelScore;
	int cntTime = clock();
	int cnt = min(30, (int)testLabels.size());
	labelID.clear();
	labelScore.clear();
	labelID.resize(cnt);
	labelScore.resize(cnt);
	answerLabel.resize(cnt);

	int numThreads = omp_get_num_procs();
	clog << "CPU number: " << numThreads << endl;
	omp_set_num_threads(numThreads);
	clog << "Start Parallel Extract Features" << endl;
#pragma omp parallel for schedule(dynamic)
	for (int i = 0; i < cnt; i++)
		rtn = testMultinomial.Predict(testSet[i], labelScore[i], 10);

	/*for (int i = 0; i < cnt; i++)
	{
		printf("\n%d\n", i);
		for (int j = 0; j < labelID[i].size(); j++) printf("%d ", labelID[i][j]);
	}*/

	FILE* outfile = fopen("test.txt", "w");
	for (int i = 0; i < cnt; i++)
	{
		fprintf(outfile, "%d\n", i);
		for (int j = 0; j < labelScore[i].size(); j++)
			fprintf(outfile, "%d %lf\n", labelScore[i][j].first, labelScore[i][j].second);
		for (int j = 0; j < testLabels[i].size(); j++)
			fprintf(outfile, "%d ", testLabels[i][j]);
		fprintf(outfile, "\n");
		//for (map<int, double>::iterator iter = testSet[i].begin(); iter != testSet[i].end(); ++iter)
			//fprintf(outfile, "%d %lf\n", iter->first, iter->second);
	}
	fclose(outfile);

	printf("%d\n", clock() - cntTime);

	/*if (labelScore[0].size() > 0)//get answerLabel
	{
		for (int i = 0; i < cnt; i++)
			for (int j = 0; j < labelScore[i].size(); j++)
				answerLabel[i].push_back(labelScore[i][j].first);
	}
	else
	{
		for (int i = 0; i < cnt; i++)
			answerLabel[i] = labelID[i];
	}*/
	//rtn = SaveLabels("loc_answer_labels.bin", answerLabel, 1);
	//CHECK_RTN(rtn);

	system("pause");
	return 0;
}