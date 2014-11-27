#include "multinomial_naivebayes/multinomial_naivebayes.h"
#include "lshtc_lib/lshtc_data.h"
using namespace std;

int main()
{
	int rtn = 0;
	LhtcDocumentSet trainLhtcSet, testLhtcSet;
	MultinomialNaiveBayes testMultinomial, testMultinomial_tmp;

	//rtn = trainLhtcSet.LoadBin("F:\\lshtc\\data\\loc_test_merge234.bin", 1);
	//CHECK_RTN(rtn);

	rtn = testLhtcSet.LoadBin("F:\\lshtc\\data\\loc_test_merge01.bin", 1);
	CHECK_RTN(rtn);

	/*vector<map<int, double>> trainSet;
	vector<vector<int> > trainLabels;
	trainLabels.clear();
	trainSet.clear();

	for (map<int, LhtcDocument>::iterator it = trainLhtcSet.mLhtcDocuments.begin(); it != trainLhtcSet.mLhtcDocuments.end(); ++it)
	{
	trainSet.push_back(it->second.mTf);
	trainLabels.push_back(it->second.mLabels);
	}//Deal trainSet

	//printf("%d\n", clock());
	rtn = testMultinomial.Build(trainSet, trainLabels);
	CHECK_RTN(rtn);
	//printf("%d\n", clock());

	rtn = testMultinomial.Save("Bayes_train_model.bin", 1);
	CHECK_RTN(rtn);*/

	rtn = testMultinomial.Load("Bayes_train_model.bin", 1);
	CHECK_RTN(rtn);

	rtn = testMultinomial.Save("Bayes_train_model_tmp.bin", 1);
	CHECK_RTN(rtn);

	printf("%d\n", testMultinomial.mInstanceSize);

	vector<vector<int> > labelID;
	vector<vector<pair<int, double>>> labelScore;
	int cntTime = clock();
	int cnt = min(30, testLhtcSet.Size());
	labelID.clear();
	labelID.resize(cnt);
	labelScore.clear();
	labelScore.resize(cnt);

	map<int, LhtcDocument>::iterator it = testLhtcSet.mLhtcDocuments.begin();

	int numThreads = omp_get_num_procs();
	clog << "CPU number: " << numThreads << endl;
	omp_set_num_threads(numThreads);
	clog << "Start Parallel Extract Features" << endl;
#pragma omp parallel for schedule(dynamic)
	for (int i = 0; i < cnt; i++)
	{
		++it;
		rtn = testMultinomial.Predict((it->second).mTf, labelID[i], 10);
	}

	for (int i = 0; i < cnt; i++)
	{
		printf("%d\n", i);
		for (int j = 0; j < labelID[i].size(); j++) printf("%d ", labelID[i][j]);
	}

	/*for (int i = 0; i < cnt; i++)
	{
	printf("%d\n", i);
	for (int j = 0; j < labelScore[i].size(); j++) printf("%d %lf\n", labelScore[i][j].first, labelScore[i][j].second);
	}*/

	printf("%d\n", clock() - cntTime);
	system("pause");
	return 0;
}