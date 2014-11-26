#include "knn_predict.h"
#include "getneighbor/ml_knn.h"
using namespace std;

int MultilabelKnn(std::string mlknn_model, std::string test_file, std::string neighbor_file, MultiLabelAnswerSet& goldStandard, std::vector<std::vector<LabelScore>>& predctLabelScore)
{
	int rtn = 0;
	LhtcDocumentSet test_set;
	rtn = test_set.LoadBin(test_file, STATUS_ONLY);
	CHECK_RTN(rtn);

	FeatureNeighbor neighbor;
	neighbor.LoadBin(neighbor_file);

	MultiLabelKnn ml_knn;
	rtn = ml_knn.Load(mlknn_model, FULL_LOG);
	CHECK_RTN(rtn);

	vector<int> test_ids;
	goldStandard.mLabelSet.clear();
	for (map<int, LhtcDocument>::iterator it = test_set.mLhtcDocuments.begin(); it != test_set.mLhtcDocuments.end(); ++it)
	{
		test_ids.push_back(it->first);
		vector<string> strLabels;
		for (size_t i = 0; i < it->second.mLabels.size(); ++i)
			strLabels.push_back(intToString(it->second.mLabels[i]));
		goldStandard.AddAnswer(MultiLabelAnswer(it->first, strLabels));
	}

	int numThreads = omp_get_num_procs();
	clog << "CPU number: " << numThreads << endl;
	omp_set_num_threads(numThreads);

	predctLabelScore.clear();
	predctLabelScore.resize(test_ids.size());
#pragma omp parallel for schedule(dynamic)
	for (int i = 0; i < (int)test_ids.size(); ++i)
	{
		ml_knn.Predict(neighbor, test_ids[i], predctLabelScore[i]);
	}
	
	return 0;
}

int MultilabelKnnEvaluate(std::string mlknn_model, std::string test_file, std::string neighbor_file, std::string numlabel_file, std::string resultFile)
{
	int rtn = 0;
	MultiLabelAnswerSet goldStandard;
	vector<vector<LabelScore>> predictScores;
	rtn = MultilabelKnn(mlknn_model, test_file, neighbor_file, goldStandard, predictScores);
	CHECK_RTN(rtn);

	map<int, int> numLabel;
	ReadFile(numlabel_file, numLabel);

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
