#include "common/common_basic.h"
#include "common/file_utility.h"
#include "classifier_tools.h"
#include "mylinear.h"
using namespace std;

int SaveLabelFreq(string fileName, map<int, int>& labelCount, int totInstance)
{
	int rtn = 0;
	FILE* freqFile = fopen(fileName.c_str(), "wb");
	rtn = Write(freqFile, (int)labelCount.size());
	CHECK_RTN(rtn);
	for (map<int, int>::iterator it = labelCount.begin(); it != labelCount.end(); it++)
	{
		double freq = (double)it->second / (double)totInstance;
		rtn = Write(freqFile, it->first);
		CHECK_RTN(rtn);
		rtn = Write(freqFile, freq);
		CHECK_RTN(rtn);
	}
	fclose(freqFile);
	return 0;
}

int SaveLabelFreq(string fileName, map<int, double>& labelFreq)
{
	int rtn = 0;
	FILE* freqFile = fopen(fileName.c_str(), "wb");
	rtn = Write(freqFile, (int)labelFreq.size());
	CHECK_RTN(rtn);
	for (map<int, double>::iterator it = labelFreq.begin(); it != labelFreq.end(); it++)
	{
		rtn = Write(freqFile, it->first);
		CHECK_RTN(rtn);
		rtn = Write(freqFile, it->second);
		CHECK_RTN(rtn);
	}
	fclose(freqFile);
	return 0;
}

int LoadLabelFreq(string fileName, map<int, double>& labelFreq)
{
	int rtn = 0;
	FileBuffer buffer(fileName.c_str());
	if (buffer.GetSize() < 1)
	{
		cerr << "File " << fileName << "Ivalid!" << endl;
		return 1;
	}
	labelFreq.clear();
	int labelCount;
	rtn = buffer.GetNextData(labelCount);
	CHECK_RTN(rtn);
	for (int i = 0; i < labelCount; i++)
	{
		int index = 0;
		double value = 0;
		rtn = buffer.GetNextData(index);
		CHECK_RTN(rtn);
		rtn = buffer.GetNextData(value);
		CHECK_RTN(rtn);
		labelFreq[index] = value;
	}
	return 0;
}

bool CmpPair(const pair<double, bool> &p1, const pair<double, bool> &p2)
{
	return p1.first > p2.first;
}

int SaveThreshold(std::map<int, double>& threshold, const char* const fileName)
{
	if (fileName == NULL)
		return -1;
	FILE* outFile = fopen(fileName, "wb");
	if (outFile == NULL)
		return -1;
	Write(outFile, (int)threshold.size());
	for (map<int, double>::iterator it = threshold.begin(); it != threshold.end(); ++it)
	{
		Write(outFile, it->first);
		Write(outFile, it->second);
	}
	fclose(outFile);
	return 0;
}

int LoadThreshold(std::map<int, double>& threshold, const char* const fileName)
{
	if (fileName == NULL)
		return -1;
	threshold.clear();
	FileBuffer buffer(fileName);
	int len = 0;
	buffer.GetNextData(len);
	for (int i = 0; i < len; ++i)
	{
		int index;
		double value;
		buffer.GetNextData(index);
		buffer.GetNextData(value);
		threshold[index] = value;
	}
	return 0;
}

int NormalizePredictScore(vector<vector<pair<int, double> > >& tarScore, map<int, double>& threshold)
{
	for (size_t i = 0; i < tarScore.size(); ++i)
	{
		for (size_t j = 0; j < tarScore[i].size(); ++j)
			tarScore[i][j].second = NormalizeScore(tarScore[i][j].second, threshold[tarScore[i][j].first]);
	}
	return 0;
}

int NormalizePredictScore(std::vector<std::vector<std::pair<int, double> > >& tarScore, ScoreTableSet& scoreTableSet)
{
	int rtn = 0;
	for (size_t i = 0; i < tarScore.size(); ++i)
	{
		for (size_t j = 0; j < tarScore[i].size(); ++j)
		{
			double value;
			rtn = scoreTableSet.Query(tarScore[i][j].first, tarScore[i][j].second, value);
			CHECK_RTN(rtn);
			tarScore[i][j].second = value;
		}
	}
	return 0;
}

double NormalizeScore(double origScore, double threshold)
{
	double c = (threshold - 1) / (2 * threshold - 1);
	return (c * origScore / (c + origScore - 1));
}

ScoreTableSet::ScoreTableSet(const double samplingRate)
{
	mSamplingRate = samplingRate;
	mTableSet.clear();
}

ScoreTableSet::ScoreTableSet(const std::vector<std::vector<std::pair<int, double>>>& exampleScore, const std::vector<std::set<int>>& goldSet, const double samplingRate)
{
	mTableSet.clear();
	mSamplingRate = samplingRate;
	if (mSamplingRate <= 1.0 + EPS)
		mSamplingRate = 1.0;
	InitializePrecisionTable(exampleScore, goldSet);
}

ScoreTableSet::ScoreTableSet(std::map<int, std::vector<std::pair<double, bool>>>& labelScore, const double samplingRate)
{
	mTableSet.clear();
	mSamplingRate = samplingRate;
	if (mSamplingRate <= 1.0 + EPS)
		mSamplingRate = 1.0;
	InitializePrecisionTable(labelScore);
}

ScoreTableSet::~ScoreTableSet()
{
	mTableSet.clear();
}

int ScoreTableSet::AddPrecisionTable(pair<int, vector<pair<double, bool>>> labelScore)
{
	int rtn = 0;
	int labelId = labelScore.first;
	vector<pair<double, bool>>& modelScores = labelScore.second;
	sort(modelScores.begin(), modelScores.end(), CmpPairByLagerFirst<double, bool>);

	int trueNum = 0;
	for (size_t i = 0; i < modelScores.size(); ++i)
	if (modelScores[i].second)
		++trueNum;


	int dis = trueNum / (int)mSamplingRate;
	if (dis < 1)
		dis = 1;

	vector<pair<double, double>> table;
	int tpNum = 0;
	for (size_t i = 0; i < modelScores.size(); ++i)
	{
		if (modelScores[i].second)
		{
			++tpNum;
			if (tpNum > 0 && tpNum % dis == 0)
			{
				double thres = modelScores[i].first;
				double pre = (double)(i + 1) > 0.0 ? (double)tpNum / (double)(i + 1) : 0.0;
				if (thres == 1.0)
					pre = 1.0;
				table.push_back(make_pair(thres, pre));
			}
		}
	}

	sort(table.begin(), table.end());
	bool bModified = false;
	if (table.empty())
	{
		bModified = true;
		table.push_back(make_pair(0.0, double(trueNum) / double(modelScores.size())));
		table.push_back(make_pair(1.0, 1.0));
	}
	if (table.size() > 0 && table.begin()->first != 0.0)
	{
		bModified = true;
		table.push_back(make_pair(0.0, double(trueNum) / double(modelScores.size())));
	}
	if (table.size() > 0 && table.rbegin()->first != 1.0)
	{
		bModified = true;
		table.push_back(make_pair(1.0, 1.0));
	}
	if (bModified)
		sort(table.begin(), table.end());

	rtn = mTableSet[labelId].Initialize(table);
	CHECK_RTN(rtn);
	return 0;
}

int ScoreTableSet::AddRecallTable(pair<int, vector<pair<double, bool>>> labelScore)
{
	int rtn = 0;
	int labelId = labelScore.first;
	vector<pair<double, bool>>& modelScores = labelScore.second;
	sort(modelScores.begin(), modelScores.end(), CmpPairByLagerFirst<double, bool>);

	int trueNum = 0;
	for (size_t i = 0; i < modelScores.size(); ++i)
	if (modelScores[i].second)
		++trueNum;

	int dis = trueNum / (int)mSamplingRate;
	if (dis < 1)
		dis = 1;

	vector<pair<double, double>> table;
	int tpNum = 0;
	for (size_t i = 0; i < modelScores.size(); ++i)
	{
		if (modelScores[i].second)
		{
			++tpNum;
			if (tpNum > 0 && tpNum % dis == 0)
			{
				double thres = modelScores[i].first;
				double rec = (double)trueNum > 0.0 ? (double)tpNum / (double)trueNum : 0.0;
				if (thres == 1.0)
					rec = 0.0;
				table.push_back(make_pair(thres, rec));
			}
		}
	}

	sort(table.begin(), table.end());
	bool bModified = false;
	if (table.empty())
	{
		bModified = true;
		table.push_back(make_pair(0.0, 1.0));
		table.push_back(make_pair(1.0, 0.0));
	}
	if (table.size() > 0 && table.begin()->first != 0.0)
	{
		bModified = true;
		table.push_back(make_pair(0.0, 1.0));
	}
	if (table.size() > 0 && table.rbegin()->first != 1.0)
	{
		bModified = true;
		table.push_back(make_pair(1.0, 0.0));
	}
	if (bModified)
		sort(table.begin(), table.end());

	rtn = mTableSet[labelId].Initialize(table);
	CHECK_RTN(rtn);
	return 0;
}

int ScoreTableSet::InitializePrecisionTable(const std::vector<std::vector<std::pair<int, double>>>& exampleScore, const std::vector<std::set<int>>& goldSet)
{
	if (exampleScore.size() != goldSet.size())
		return -1;
	int rtn = 0;
	mTableSet.clear();

	map<int, vector<pair<double, bool> > > modelScores;
	for (size_t i = 0; i < exampleScore.size(); ++i)
	{
		const set<int>& gs = goldSet[i];

		for (size_t j = 0; j < exampleScore[i].size(); ++j)
		{
			if (gs.count(exampleScore[i][j].first) > 0)
				modelScores[exampleScore[i][j].first].push_back(make_pair(exampleScore[i][j].second, true));
			else
				modelScores[exampleScore[i][j].first].push_back(make_pair(exampleScore[i][j].second, false));
		}
	}

	rtn = InitializePrecisionTable(modelScores);
	CHECK_RTN(rtn);
	return 0;
}

int ScoreTableSet::InitializePrecisionTable(std::map<int, std::vector<std::pair<double, bool>>>& labelScore)
{
	int rtn = 0;

	mTableSet.clear();
	for (map<int, vector<pair<double, bool>>>::iterator it = labelScore.begin(); it != labelScore.end(); ++it)
		rtn = AddPrecisionTable(*it);

	return 0;
}

int ScoreTableSet::InitializeRecallTable(std::map<int, std::vector<std::pair<double, bool>>>& labelScore)
{
	int rtn = 0;

	mTableSet.clear();
	for (map<int, vector<pair<double, bool>>>::iterator it = labelScore.begin(); it != labelScore.end(); ++it)
		rtn = AddRecallTable(*it);

	return 0;
}

int ScoreTableSet::InitializeRecallTable(const std::vector<std::vector<std::pair<int, double>>>& exampleScore, const std::vector<std::set<int>>& goldSet)
{
	if (exampleScore.size() != goldSet.size())
		return -1;
	int rtn = 0;
	mTableSet.clear();

	map<int, vector<pair<double, bool> > > modelScores;
	for (size_t i = 0; i < exampleScore.size(); ++i)
	{
		const set<int>& gs = goldSet[i];

		for (size_t j = 0; j < exampleScore[i].size(); ++j)
		{
			if (gs.count(exampleScore[i][j].first) > 0)
				modelScores[exampleScore[i][j].first].push_back(make_pair(exampleScore[i][j].second, true));
			else
				modelScores[exampleScore[i][j].first].push_back(make_pair(exampleScore[i][j].second, false));
		}
	}

	rtn = InitializeRecallTable(modelScores);
	CHECK_RTN(rtn);
	return 0;
}

int ScoreTableSet::Query(int labelId, double threshold, double& value)
{
	value = -1.0;
	if (mTableSet.count(labelId) == 0)
	{
		cerr << "Error: can't find the labelId in ScoreTableSet" << endl;
		return -1;
	}

	if (threshold < 0.0 || threshold > 1.0)
	{
		cerr << "Error: threshold out of range when query precision" << endl;
		return -1;
	}

	int rtn = 0;
	rtn = mTableSet[labelId].Query(threshold, value);
	CHECK_RTN(rtn);
	return 0;
}

QueryTable* ScoreTableSet::operator[](const int labelId)
{
	if (mTableSet.count(labelId) == 0)
		return NULL;
	return &mTableSet[labelId];
}

int ScoreTableSet::Size()
{
	return (int)mTableSet.size();
}

int ScoreTableSet::Count(int labelId)
{
	return (int)mTableSet.count(labelId);
}

int ScoreTableSet::Save(const std::string& fileName, int printLog)
{
	FILE *outFile = fopen(fileName.c_str(), "wb");
	if (outFile == NULL)
		return -1;
	int rtn = 0;
	rtn = Save(outFile, printLog);
	fclose(outFile);
	CHECK_RTN(rtn);
	return 0;
}

int ScoreTableSet::Save(FILE* outFile, int printLog)
{
	if (outFile == NULL)
		return -1;
	int  rtn = 0;
	rtn = Write(outFile, mSamplingRate);
	CHECK_RTN(rtn);
	rtn = Write(outFile, (size_t)mTableSet.size());
	int cnt = 0;
	for (map<int, QueryTable>::iterator it = mTableSet.begin(); it != mTableSet.end(); ++it)
	{
		rtn = Write(outFile, it->first);
		CHECK_RTN(rtn);
		rtn = it->second.Save(outFile);
		CHECK_RTN(rtn);
		++cnt;
	}
	if (printLog != SILENT)
		clog << "Total saved " << cnt << " scoreTable instances" << endl;
	return 0;
}

int ScoreTableSet::Load(const std::string& fileName, int printLog)
{
	mTableSet.clear();
	FILE* inFile = fopen(fileName.c_str(), "rb");
	if (inFile == NULL)
		return -1;
	int rtn = 0;
	rtn = Load(inFile, printLog);
	fclose(inFile);
	CHECK_RTN(rtn);
	return 0;
}

int ScoreTableSet::Load(FILE* inFile, int printLog)
{
	mTableSet.clear();
	if (inFile == NULL)
		return -1;
	int rtn = 0;
	FileBuffer buffer(inFile);
	rtn = Load(buffer, printLog);
	CHECK_RTN(rtn);
	return 0;
}

int ScoreTableSet::Load(FileBuffer& buffer, int printLog)
{
	mTableSet.clear();
	if (buffer.Eof())
		return -1;
	int rtn = 0;
	rtn = buffer.GetNextData(mSamplingRate);
	CHECK_RTN(rtn);
	size_t len = 0;
	rtn = buffer.GetNextData(len);
	CHECK_RTN(rtn);
	int cnt = 0;
	for (size_t i = 0; i < len; ++i)
	{
		if (buffer.Eof())
			return -1;
		int labelId;
		rtn = buffer.GetNextData(labelId);
		CHECK_RTN(rtn);
		rtn = mTableSet[labelId].Load(buffer);
		CHECK_RTN(rtn);
		++cnt;
	}
	if (printLog != SILENT)
		clog << "Total loaded " << cnt << " scoreTable instances" << endl;
	return 0;
}

int Threshold::GetThreshold(LinearMachine& queryModel, FeatureSet& featureSet, std::vector<bool>& trueLabels, double& bestThres)
{
	vector<pair<double, bool> > modelScores;
	modelScores.resize(featureSet.Size());

#pragma omp parallel for schedule(dynamic)
	for (int i = 0; i < featureSet.Size(); i++)
	{
		queryModel.Predict(featureSet[i], modelScores[i].first);
		modelScores[i].second = trueLabels[i];
	}
	int rtn = GetThreshold(modelScores, bestThres);
	CHECK_RTN(rtn);
	return 0;
}

int Threshold::GetThreshold(LinearMachine& queryModel, feature_node** featureSet, std::vector<bool>& trueLabels, double& bestThres)
{
	vector<pair<double, bool> > modelScores;
	modelScores.resize(trueLabels.size());

#pragma omp parallel for schedule(dynamic)
	for (int i = 0; i < (int)trueLabels.size(); i++)
	{
		queryModel.Predict(featureSet[i], modelScores[i].first);
		modelScores[i].second = trueLabels[i];
	}

	int rtn = GetThreshold(modelScores, bestThres);
	CHECK_RTN(rtn);
	return 0;
}

int Threshold::GetThreshold(std::vector<std::pair<double, bool> > &modelScores, double& bestThres)
{
	const double DEFAULT_THRESHOLD = 0.9;
	sort(modelScores.begin(), modelScores.end(), CmpPairByLagerFirst<double, bool>);
	bestThres = 1.0;

	int trueNum = 0;
	for (size_t i = 0; i < modelScores.size(); ++i)
	if (modelScores[i].second)
		++trueNum;

	int tpNum;
	tpNum = 0;

	double preThres = 0.2;
	double maxF1 = 0.0;
	unsigned j = 0;
	while (j < modelScores.size())
	{
		double thre = modelScores[j].first;
		for (; j < modelScores.size(); ++j)
		{
			if (modelScores[j].first + EPS < thre)
				break;

			if (modelScores[j].second)
				tpNum++;
		}

		int posNum = j;
		double pre = posNum > 0.0 ? (double)tpNum / (double)posNum : 0.0;
		double rec = trueNum > 0.0 ? (double)tpNum / (double)trueNum : 0.0;
		double f1 = (pre + rec) > 0.0 ? 2 * pre*rec / (pre + rec) : 0.0;

		if (pre >= preThres)// || (posNum - tpNum) + EPS < falseAlarm
		{
			if (maxF1 < f1)
			{
				maxF1 = f1;
				bestThres = thre;
			}
		}
	}
	if (bestThres == 1.0 || bestThres == 0.0)
		bestThres = DEFAULT_THRESHOLD;
	return 0;
}

Threshold::Threshold()
{
	mThreshold.clear();
}

Threshold::Threshold(const std::map<int, double>& threshold)
{
	mThreshold = threshold;
}

Threshold::~Threshold()
{
	mThreshold.clear();
}

int Threshold::AddInstanceByRank(std::pair<int, std::vector<std::pair<double, bool>>> labelScore)
{
	const double DEFAULT_THRESHOLD = 0.9;
	int rtn = 0;
	int labelId = labelScore.first;
	vector<pair<double, bool>>& modelScores = labelScore.second;
	sort(modelScores.begin(), modelScores.end(), CmpPairByLagerFirst<double, bool>);

	int trueNum = 0;
	for (int i = 0; i < (int)modelScores.size(); ++i)
	if (modelScores[i].second)
		++trueNum;

	double thres = DEFAULT_THRESHOLD;
	if (trueNum > 0 && trueNum < modelScores.size())
		thres = (modelScores[trueNum - 1].first + modelScores[trueNum].first) / 2.0;
	else if (trueNum == modelScores.size())
		thres = (modelScores[trueNum - 1].first + 0.0) / 2.0;

	mThreshold[labelId] = thres;
	return 0;
}

int Threshold::AddInstanceByMaxFscore(std::pair<int, std::vector<std::pair<double, bool>>> labelScore)
{
	int rtn = 0;
	int labelId = labelScore.first;
	vector<pair<double, bool>>& modelScores = labelScore.second;
	sort(modelScores.begin(), modelScores.end(), CmpPairByLagerFirst<double, bool>);
	double thres = 0.0;
	rtn = GetThreshold(modelScores, thres);
	CHECK_RTN(rtn);
	mThreshold[labelId] = thres;
	return 0;
}

int Threshold::InitializeByRank(const std::vector<std::vector<std::pair<int, double>>>& exampleScore, const std::vector<std::set<int>>& goldSet)
{
	if (exampleScore.size() != goldSet.size())
		return -1;
	int rtn = 0;
	mThreshold.clear();
	map<int, vector<pair<double, bool> > > modelScores;
	for (int i = 0; i < (int)exampleScore.size(); ++i)
	{
		const set<int>& gs = goldSet[i];

		for (int j = 0; j < (int)exampleScore[i].size(); ++j)
		{
			if (gs.count(exampleScore[i][j].first) > 0)
				modelScores[exampleScore[i][j].first].push_back(make_pair(exampleScore[i][j].second, true));
			else
				modelScores[exampleScore[i][j].first].push_back(make_pair(exampleScore[i][j].second, false));
		}
	}

	rtn = InitializeByRank(modelScores);
	CHECK_RTN(rtn);
	return 0;
}

int Threshold::InitializeByRank(std::map<int, std::vector<std::pair<double, bool>>>& labelScore)
{
	int rtn = 0;
	mThreshold.clear();
	for (map<int, vector<pair<double, bool>>>::iterator it = labelScore.begin(); it != labelScore.end(); ++it)
	{
		rtn = AddInstanceByRank(*it);
		CHECK_RTN(rtn);
	}
	return 0;
}

int Threshold::InitializeByMaxFscore(const std::vector<std::vector<std::pair<int, double>>>& exampleScore, const std::vector<std::set<int>>& goldSet)
{
	if (exampleScore.size() != goldSet.size())
		return -1;
	int rtn = 0;
	mThreshold.clear();
	map<int, vector<pair<double, bool> > > modelScores;
	for (size_t i = 0; i < exampleScore.size(); ++i)
	{
		const set<int>& gs = goldSet[i];

		for (size_t j = 0; j < exampleScore[i].size(); ++j)
		{
			if (gs.count(exampleScore[i][j].first) > 0)
				modelScores[exampleScore[i][j].first].push_back(make_pair(exampleScore[i][j].second, true));
			else
				modelScores[exampleScore[i][j].first].push_back(make_pair(exampleScore[i][j].second, false));
		}
	}

	rtn = InitializeByMaxFscore(modelScores);
	CHECK_RTN(rtn);
	return 0;
}

int Threshold::InitializeByMaxFscore(std::map<int, std::vector<std::pair<double, bool>>>& labelScore)
{
	int rtn = 0;
	mThreshold.clear();
	for (map<int, vector<pair<double, bool>>>::iterator it = labelScore.begin(); it != labelScore.end(); ++it)
	{
		rtn = AddInstanceByMaxFscore(*it);
		CHECK_RTN(rtn);
	}

	return 0;
}

int Threshold::Query(const int labelId, double& value)
{
	value = -1.0;
	if (mThreshold.count(labelId) == 0)
		return -1;
	value = mThreshold[labelId];
	return 0;
}

double Threshold::operator[](const int labelId)
{
	if (mThreshold.count(labelId) == 0)
		return 2.0;
	return mThreshold[labelId];
}

map<int, double> Threshold::GetThreshold()
{
	return mThreshold;
}

int Threshold::Size()
{
	return mThreshold.size();
}

int Threshold::Save(const std::string& fileName, int printLog)
{
	FILE *outFile = fopen(fileName.c_str(), "wb");
	if (outFile == NULL)
		return -1;
	int rtn = 0;
	rtn = Save(outFile, printLog);
	fclose(outFile);
	CHECK_RTN(rtn);
	return 0;
}

int Threshold::Save(FILE* outFile, int printLog)
{
	if (outFile == NULL)
		return -1;
	int rtn = 0;
	rtn = Write(outFile, mThreshold);
	CHECK_RTN(rtn);
	return 0;
}

int Threshold::Load(const std::string& fileName, int printLog)
{
	mThreshold.clear();
	FILE* inFile = fopen(fileName.c_str(), "rb");
	if (inFile == NULL)
		return -1;
	int rtn = 0;
	rtn = Load(inFile, printLog);
	fclose(inFile);
	CHECK_RTN(rtn);
	return 0;
}

int Threshold::Load(FILE* inFile, int printLog)
{
	mThreshold.clear();
	if (inFile == NULL)
		return -1;
	int rtn = 0;
	rtn = Read(inFile, mThreshold);
	CHECK_RTN(rtn);
	if (printLog != SILENT)
		clog << "Loaded " << mThreshold.size() << " threshold intances" << endl;
	return 0;
}

int Threshold::Load(FileBuffer& buffer, int printLog)
{
	mThreshold.clear();
	if (buffer.Eof())
		return -1;
	int rtn = 0;
	rtn = buffer.GetNextData(mThreshold);
	CHECK_RTN(rtn);
	if (printLog != SILENT)
		clog << "Loaded " << mThreshold.size() << " threshold intances" << endl;
	return 0;
}
