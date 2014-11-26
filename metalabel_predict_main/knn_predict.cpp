#include "knn_predict.h"
#include <vector>
using namespace std;

int CosineKnnPredict(std::string trainFile, std::string testFile, std::string neighborFile, MultiLabelAnswerSet& goldStandard, std::vector<std::vector<LabelScore>>& predctLabelScore)
{
	const int NEIBO_NUM = 20;
	int rtn = 0;
	LhtcDocumentSet trainSet;
	rtn = trainSet.LoadBin(trainFile, STATUS_ONLY);
	CHECK_RTN(rtn);

	LhtcDocumentSet testSet;
	rtn = testSet.LoadBin(testFile, STATUS_ONLY);
	CHECK_RTN(rtn);

	FeatureNeighbor neighbor;
	neighbor.LoadBin(neighborFile);

	vector<int> testIds;
	for (map<int, LhtcDocument>::iterator it = testSet.mLhtcDocuments.begin(); it != testSet.mLhtcDocuments.end(); ++it)
	{
		testIds.push_back(it->first);
	}

	for (map<int, LhtcDocument>::iterator it = testSet.mLhtcDocuments.begin(); it != testSet.mLhtcDocuments.end(); ++it)
	{
		vector<int>& labels = it->second.mLabels;
		vector<string> strLabels;
		for (size_t k = 0; k < labels.size(); ++k)
		{
			strLabels.push_back(intToString(labels[k]));
		}
		goldStandard.AddAnswer(MultiLabelAnswer(it->first, strLabels));
	}

	int numThreads = omp_get_num_procs();
	clog << "CPU number: " << numThreads << endl;
	omp_set_num_threads(numThreads);

	predctLabelScore.clear();
	predctLabelScore.resize(testIds.size());
#pragma omp parallel for schedule(dynamic)
	for (int i = 0; i < (int)testIds.size(); ++i)
	{
		vector<int> neibos;
		vector<double> similarities;
		rtn = neighbor.GetNeighbor(testIds[i], NEIBO_NUM, neibos, similarities);

		map<int, double> labelFreq;
		for (size_t j = 0; j < neibos.size(); ++j)
		{
			vector<int>& labels = trainSet.mLhtcDocuments[neibos[j]].mLabels;
			for (size_t k = 0; k < labels.size(); ++k)
			{
				labelFreq[labels[k]] += 1.0;//similarities[k];
			}
		}
		for (auto it = labelFreq.begin(); it != labelFreq.end(); ++it)
			predctLabelScore[i].push_back(make_pair(it->first, it->second / double(neibos.size())));
		sort(predctLabelScore[i].begin(), predctLabelScore[i].end(), CmpPairByLagerSecond<int,double>);
	}

	return 0;
}

int CosineKnnEvaluate(std::string trainFile, std::string testFile, std::string neighborFile, std::string numlabelFile, std::string resultFile)
{
	int rtn = 0; 
	MultiLabelAnswerSet goldStandard;
	vector<vector<LabelScore>> predictScores;
	rtn = CosineKnnPredict(trainFile, testFile, neighborFile, goldStandard, predictScores);
	CHECK_RTN(rtn);

	map<int, int> numLabel;
	ReadFile(numlabelFile, numLabel);

	MultiLabelAnswerSet predictLabels;
	for (size_t i = 0; i < predictScores.size(); ++i)
	{
		int id = goldStandard[i].mPmid;
		if (numLabel.count(id) == 0)
			return -1;
		int num = numLabel[id];
		vector<string> strLabels;
		for (size_t j = 0; j < predictScores[i].size() && j < num; ++j)
		{
			strLabels.push_back(intToString(predictScores[i][j].first));
		}
		predictLabels.AddAnswer(MultiLabelAnswer(id, strLabels));
	}

	rtn = Evaluate(goldStandard, predictLabels, resultFile);
	CHECK_RTN(rtn);
	return 0;
}
