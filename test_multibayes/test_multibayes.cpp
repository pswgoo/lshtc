#include "multinomial_naivebayes/multinomial_naivebayes.h"
#include "lshtc_lib/lshtc_data.h"
using namespace std;

int LoadTrainSetLabels(string fileName, vector<vector<int> >& trainSetLabels, int printLog)
{
	int rtn = 0;
	FILE *infile = fopen(fileName.c_str(), "rb");

	rtn = Read(infile, trainSetLabels);
	CHECK_RTN(rtn);

	fclose(infile);
	if (printLog != SILENT)
		std::clog << "Load Labels successful!" << std::endl;
	return 0;
}

int LoadTrainSetFeatures(string fileName, vector<std::map<int, double> >& trainSetFeatures, int printLog)
{
	int rtn = 0;
	FILE *infile = fopen(fileName.c_str(), "rb");

	rtn = Read(infile, trainSetFeatures);
	CHECK_RTN(rtn);

	fclose(infile);
	if (printLog != SILENT)
		std::clog << "Load Features successful!" << std::endl;
	return 0;
}

int SaveTrainSetLabels(string fileName, vector<vector<int> >& trainSetLabels, int printLog)
{
	int rtn = 0;
	FILE *outfile = fopen(fileName.c_str(), "wb");

	rtn = Write(outfile, trainSetLabels);
	CHECK_RTN(rtn);
	
	fclose(outfile);
	if (printLog != SILENT)
		std::clog << "Save Labels successful!" << std::endl;
	return 0;
}

int SaveTrainSetFeatures(string fileName, vector<std::map<int, double> >& trainSetFeatures, int printLog)
{
	int rtn = 0;
	FILE *outfile = fopen(fileName.c_str(), "wb");

	rtn = Write(outfile, trainSetFeatures);
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

	//rtn = testLhtcSet.LoadBin("F:\\lshtc\\data\\loc_test_merge01.bin", 1);
	//CHECK_RTN(rtn);

	vector<map<int, double> > trainSet;
	vector<vector<int> > trainLabels;
	trainLabels.clear();
	trainSet.clear();

	for (map<int, LhtcDocument>::iterator it = trainLhtcSet.mLhtcDocuments.begin(); it != trainLhtcSet.mLhtcDocuments.end(); ++it)
	{
		trainSet.push_back(it->second.mTf);
		trainLabels.push_back(it->second.mLabels);
	}//Deal trainSet

	rtn = SaveTrainSetLabels("loc_trainset_labels.bin", trainLabels, 1);
	CHECK_RTN(rtn);
	
	rtn = SaveTrainSetFeatures("loc_trainset_features.bin", trainSet, 1);
	CHECK_RTN(rtn);

	rtn = LoadTrainSetLabels("loc_trainset_labels.bin", trainLabels, 1);
	CHECK_RTN(rtn);

	rtn = LoadTrainSetFeatures("loc_trainset_features.bin", trainSet, 1);
	CHECK_RTN(rtn);

	printf("%d %d\n", trainLabels.size(), trainLabels[0].size());

	//printf("%d\n", clock());
	rtn = testMultinomial.Build(trainSet, trainLabels, 1);
	CHECK_RTN(rtn);
	//printf("%d\n", clock());

	rtn = testMultinomial.Save("Bayes_train_model.bin", 1);
	CHECK_RTN(rtn);

	rtn = testMultinomial.Load("Bayes_train_model.bin", 1);
	CHECK_RTN(rtn);

	//rtn = testMultinomial.Save("Bayes_train_model_tmp.bin", 1);
	//CHECK_RTN(rtn);

	printf("%d\n", testMultinomial.mInstanceSize);
	
	vector<vector<int> > labelID;
	vector<vector<int> > answerLabel;
	vector<vector<pair<int, double>>> labelScore;
	int cntTime = clock();
	int cnt = min(30, testLhtcSet.Size());
	labelID.clear();
	labelID.resize(cnt);
	labelScore.clear();
	labelScore.resize(cnt);
	answerLabel.resize(cnt);

	map<int, LhtcDocument>::iterator it = testLhtcSet.mLhtcDocuments.begin();

	int numThreads = omp_get_num_procs();
	clog << "CPU number: " << numThreads << endl;
	omp_set_num_threads(numThreads);
	clog << "Start Parallel Extract Features" << endl;
#pragma omp parallel for schedule(dynamic)
	for (int i = 0; i < cnt; i++)
	{
		++it;
		rtn = testMultinomial.Predict((it->second).mTf, labelScore[i], 10);
	}

	/*for (int i = 0; i < cnt; i++)
	{
		printf("%d\n", i);
		for (int j = 0; j < labelID[i].size(); j++) printf("%d ", labelID[i][j]);
	}*/

	it = testLhtcSet.mLhtcDocuments.begin();
	FILE* outfile = fopen("test.txt", "w");
	for (int i = 0; i < cnt; i++)
	{
		fprintf(outfile, "%d\n", i);
		for (int j = 0; j < labelScore[i].size(); j++) fprintf(outfile, "%d %lf\n", labelScore[i][j].first, labelScore[i][j].second);
		++it;
		for (int j = 0; j < it->second.mLabels.size(); j++)
			fprintf(outfile, "%d ", it->second.mLabels[j]);
		fprintf(outfile, "\n");
		for (map<int, double>::iterator iter = it->second.mTf.begin(); iter != it->second.mTf.end(); ++iter)
			fprintf(outfile, "%d %lf\n", iter->first, iter->second);
	}
	fclose(outfile);

	printf("%d\n", clock() - cntTime);
	system("pause");
	return 0;
}