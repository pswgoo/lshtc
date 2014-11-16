#include "evaluator.h"
#include "medline/basic.h"
#include "jsoncpp/json.h"
using namespace std;

MultiLabelAnswer::MultiLabelAnswer()
{
	Clear();
}

MultiLabelAnswer::MultiLabelAnswer(const int pmid)
{
	mPmid = pmid;
	mLabels.clear();
}

MultiLabelAnswer::MultiLabelAnswer(const int pmid, const std::vector<std::string> &labels)
{
	mPmid = pmid;
	mLabels = labels;
}

MultiLabelAnswer::MultiLabelAnswer(const Citation* const citation)
{
	mPmid = citation->mPmid;
	citation->GetMeshVector(mLabels);
}

MultiLabelAnswer::MultiLabelAnswer(const Json::Value &root)
{
	if (!root.isMember("pmid") || !root.isMember("labels"))
		return;

	mPmid = root["pmid"].asInt();
	const Json::Value &arrLab = root["labels"];
	for (unsigned int i = 0; i < arrLab.size(); ++i)
		mLabels.push_back(arrLab[i].asString());
}

MultiLabelAnswer::MultiLabelAnswer(const Json::Value &root, MeshRecordSet &meshSet)
{
	if (!root.isMember("pmid") || !root.isMember("labels"))
		return;

	mPmid = root["pmid"].asInt();
	const Json::Value &arrLab = root["labels"];
	for (unsigned int i = 0; i < arrLab.size(); ++i)
	{
		int labelId;
		char ch;
		sscanf(arrLab[i].asString().c_str(), "%c%d", &ch, &labelId);
		if (ch != 'D')
			continue;
		if (meshSet[labelId] == NULL)
		{
			cerr << "Error: Can't find mesh uid \"" << labelId << "\" in MeshRecordSet!" << endl;
			return;
		}
		mLabels.push_back(meshSet[labelId]->mName);
	}
}

bool MultiLabelAnswerSet::CmpPmid(const MultiLabelAnswer& answer1, const MultiLabelAnswer &answer2)
{
	return answer1.mPmid < answer2.mPmid;
}

MultiLabelAnswer::~MultiLabelAnswer()
{
}

std::string MultiLabelAnswer::operator[](int index)
{
	return mLabels[index];
}

int MultiLabelAnswer::Clear()
{
	mPmid = 0;
	mLabels.clear();
	return 0;
}

int MultiLabelAnswer::Size()
{
	return (int)mLabels.size();
}

int MultiLabelAnswer::SaveJson(FILE* outFile)
{
	if (outFile == NULL)
		return -1;

	int rtn = 0;
	fprintf(outFile, "{");

	fprintf(outFile, "\"labels\":[");
	for (unsigned i = 0; i < mLabels.size(); ++i)
	{
		if (i>0)
			fprintf(outFile, ",");
		fprintf(outFile, "\"");
		rtn = PutJsonEscapes(outFile, mLabels[i].c_str());
		CHECK_RTN(rtn);
		fprintf(outFile, "\"");
	}
	fprintf(outFile, "]");

	fprintf(outFile, ",");
	fprintf(outFile, "\"pmid\":%d", mPmid);
	fprintf(outFile, "}");
	return 0;
}

int MultiLabelAnswer::SaveUidJson(FILE* outFile, MeshRecordSet& meshSet)
{
	int rtn = 0;
	fprintf(outFile, "{");

	fprintf(outFile, "\"labels\":[");
	for (unsigned i = 0; i < mLabels.size(); ++i)
	{
		if (i>0)
			fprintf(outFile, ",");
		fprintf(outFile, "\"");
		if (meshSet[mLabels[i]] == NULL)
		{
			cerr << "Error: Can't find mesh \"mLabels[i]\"" << endl;
			return -1;
		}
		fprintf(outFile, "D%06d", meshSet[mLabels[i]]->mUid);
		fprintf(outFile, "\"");
	}
	fprintf(outFile, "]");
	
	fprintf(outFile, ",");
	fprintf(outFile, "\"pmid\":%d", mPmid);
	fprintf(outFile, "}");
	return 0;
}

int MultiLabelAnswerSet::SavePredictScores(const std::vector <std::vector<std::pair<int, double> > >& predictScores, const char* const fileName)
{
	if (fileName == NULL)
		return -1;
	FILE* outFile = fopen(fileName, "wb");
	Write(outFile, (int)predictScores.size());
	for (size_t i = 0; i < predictScores.size(); ++i)
	{
		Write(outFile, (int)predictScores[i].size());
		for (size_t j = 0; j < predictScores[i].size(); ++j)
		{
			Write(outFile, predictScores[i][j].first);
			Write(outFile, predictScores[i][j].second);
		}
	}
	fclose(outFile);
	return 0;
}

int MultiLabelAnswerSet::LoadPredictScores(std::vector <std::vector<std::pair<int, double> > >& predictScores, const char* const fileName)
{
	if (fileName == NULL)
		return -1;
	predictScores.clear();
	FileBuffer buffer(fileName);
	int rowLen = 0;
	buffer.GetNextData(rowLen);
	for (int i = 0; i < rowLen; ++i)
	{
		int colLen = 0;
		buffer.GetNextData(colLen);
		std::vector<std::pair<int, double> > scores;
		for (int j = 0; j < colLen; ++j)
		{
			int first;
			double second;
			buffer.GetNextData(first);
			buffer.GetNextData(second);
			scores.push_back(make_pair(first, second));
		}
		predictScores.push_back(scores);
	}
	return 0;
}

int MultiLabelAnswerSet::LoadPredictScores(std::vector < std::vector<std::pair<int, double> > >& predictScores, double threshold, const std::string& fileName, int printLog)
{
	predictScores.clear();
	FILE *inFile = fopen(fileName.c_str(), "rb");
	if (inFile == NULL)
		return -1;
	int rtn = 0;
	size_t len;
	rtn = Read(inFile, len);
	CHECK_RTN(rtn);
	if (printLog != SILENT)
		clog << "LOG: Read len = " << len << endl;
	std::set<int> pmids;
	std::map<int, std::vector<std::pair<int, double>>> exampleScore;
	for (size_t i = 0; i < len; ++i)
	{
		if (printLog == FULL_LOG)
		{
			if (((int)i & 255) == 0)
				clog << "Log: " << i << " line loading" << endl;
		}
		std::pair<int, std::vector<std::pair<int, double>>> modelScore;
		rtn = Read(inFile, modelScore);
		CHECK_RTN(rtn);
		for (size_t j = 0; j < modelScore.second.size(); ++j)
		{
			pmids.insert(modelScore.second[j].first);
			if (modelScore.second[j].second >= threshold)
			{
				exampleScore[modelScore.second[j].first].push_back(make_pair(modelScore.first, modelScore.second[j].second));
			}
		}
	}
	if (printLog == FULL_LOG)
		clog << endl;

	predictScores.resize(pmids.size());
	int k = 0;
	for (std::set<int>::iterator it = pmids.begin(); it != pmids.end(); ++it, ++k)
	{
		if (exampleScore.count(*it) > 0)
		{
			predictScores[k] = exampleScore[*it];
		}
		else
			predictScores[k].clear();
	}
	fclose(inFile);
	if (printLog != SILENT)
		clog << "LOG: Load " << predictScores.size() << " citations" << endl;
	return 0;
}

int MultiLabelAnswerSet::SaveNumLabel(std::map<int, int>& numLabels, const char* const fileName)
{
	if (fileName == NULL)
		return -1;
	int rtn = 0;
	FILE *outFile = fopen(fileName, "wb");
	if (outFile == NULL)
		return -1;
	rtn = Write(outFile, (int)numLabels.size());
	CHECK_RTN(rtn);
	for (std::map<int, int>::iterator it = numLabels.begin(); it != numLabels.end(); ++it)
	{
		rtn = Write(outFile, it->first);
		CHECK_RTN(rtn);
		rtn = Write(outFile, it->second);
		CHECK_RTN(rtn);
	}
	fclose(outFile);
	return 0;
}

int MultiLabelAnswerSet::LoadNumLabel(std::map<int, int>& numLabels, const char* const fileName)
{
	if (fileName == NULL)
		return -1;
	int rtn = 0;
	numLabels.clear();
	int len = 0;
	FileBuffer buffer(fileName);
	rtn = buffer.GetNextData(len);
	CHECK_RTN(rtn);
	for (int i = 0; i < len; ++i)
	{
		int index, value;
		rtn = buffer.GetNextData(index);
		CHECK_RTN(rtn);
		rtn = buffer.GetNextData(value);
		CHECK_RTN(rtn);
		numLabels[index] = value;
	}
	return 0;
}

MultiLabelAnswerSet::MultiLabelAnswerSet()
{
	mAnswers.clear();
	mLabelSet.clear();
}

MultiLabelAnswerSet::~MultiLabelAnswerSet()
{
}

MultiLabelAnswer& MultiLabelAnswerSet::operator[](int index)
{
	return mAnswers[index];
}

int MultiLabelAnswerSet::AddAnswer(MultiLabelAnswer answer)
{
	mAnswers.push_back(answer);
	for (int i = 0; i < answer.Size(); i++)
		mLabelSet.insert(answer[i]);
	return 0;
}

int MultiLabelAnswerSet::Size()
{
	return (int)mAnswers.size();
}

int MultiLabelAnswerSet::LabelSize()
{
	return (int)mLabelSet.size();
}

int MultiLabelAnswerSet::SortByPmid()
{
	sort(mAnswers.begin(), mAnswers.end(), CmpPmid);
	return 0;
}

int MultiLabelAnswerSet::LoadJsonSet(const char* const fileName)
{
	if (fileName == NULL)
		return -1;
	mAnswers.clear();
	mLabelSet.clear();
	FILE *inFile = fopen(fileName, "rb");
	int rtn = 0;
	rtn = LoadJsonSet(inFile);
	CHECK_RTN(rtn);
	fclose(inFile);
	return 0;
}

int MultiLabelAnswerSet::LoadJsonSet(FILE* inFile)
{
	if (inFile == NULL)
		return -1;
	mAnswers.clear();
	mLabelSet.clear();
	int rtn;
	FileBuffer buffer(inFile);
	char ch;

	while (!buffer.Eof())
	{
		rtn = buffer.GetNextData(ch);
		CHECK_RTN(rtn);
		if (ch == '{')
			break;
	}
	if (buffer.Eof())
		return 1;

	std::string line;
	Json::Reader reader;
	while (NextBracket(buffer, '{', '}', line) >= 0)
	{
		Json::Value record;
		reader.parse(line, record);
		if (record.isMember("pmid") && record.isMember("labels"))
		{
			MultiLabelAnswer answer(record);
			rtn = AddAnswer(answer);
			CHECK_RTN(rtn);
		}
	}
	return 0;
}

int MultiLabelAnswerSet::LoadUidJsonSet(const char* const fileName, const char* const meshPath)
{
	if (fileName == NULL)
		return -1;
	FILE *inFile = fopen(fileName, "rb");
	FILE *meshFile = fopen(meshPath, "rb");
	if (meshFile == NULL)
	{
		cerr << "Error: Can't open mesh file in MultiLabelAnswerSet::LoadUidJsonSet()" << endl;
		return -1;
	}
	mAnswers.clear();
	mLabelSet.clear();
	int rtn = 0;
	rtn = LoadUidJsonSet(inFile, meshFile);
	CHECK_RTN(rtn);
	fclose(inFile);
	return 0;
}

int MultiLabelAnswerSet::LoadUidJsonSet(FILE* inFile, FILE* meshFile)
{
	if (inFile == NULL || meshFile == NULL)
		return -1;

	mAnswers.clear();
	mLabelSet.clear();
	int rtn;
	FileBuffer buffer(inFile);
	char ch;

	MeshRecordSet meshSet;
	rtn = meshSet.Load(meshFile);
	CHECK_RTN(rtn);

	while (!buffer.Eof())
	{
		rtn = buffer.GetNextData(ch);
		CHECK_RTN(rtn);
		if (ch == '{')
			break;
	}
	if (buffer.Eof())
		return 1;

	std::string line;
	Json::Reader reader;
	while (NextBracket(buffer, '{', '}', line) >= 0)
	{
		Json::Value record;
		reader.parse(line, record);
		if (record.isMember("pmid") && record.isMember("labels"))
		{
			MultiLabelAnswer answer(record, meshSet);
			rtn = AddAnswer(answer);
			CHECK_RTN(rtn);
		}
	}
	return 0;
}

int MultiLabelAnswerSet::AddUidJsonSet(const char* const fileName, const char* const meshPath)
{
	if (fileName == NULL)
		return -1;
	FILE *inFile = fopen(fileName, "rb");
	FILE *meshFile = fopen(meshPath, "rb");
	if (meshFile == NULL)
	{
		cerr << "Error: Can't open mesh file in MultiLabelAnswerSet::AddUidJsonSet()" << endl;
		return -1;
	}
	int rtn = 0;
	rtn = AddUidJsonSet(inFile, meshFile);
	CHECK_RTN(rtn);
	fclose(inFile);
	return 0;
}

int MultiLabelAnswerSet::AddUidJsonSet(FILE* inFile, FILE* meshFile)
{
	if (inFile == NULL || meshFile == NULL)
		return -1;

	int rtn;
	FileBuffer buffer(inFile);
	char ch;

	MeshRecordSet meshSet;
	rtn = meshSet.Load(meshFile);
	CHECK_RTN(rtn);

	while (!buffer.Eof())
	{
		rtn = buffer.GetNextData(ch);
		CHECK_RTN(rtn);
		if (ch == '{')
			break;
	}
	if (buffer.Eof())
		return 1;

	std::string line;
	Json::Reader reader;
	while (NextBracket(buffer, '{', '}', line) >= 0)
	{
		Json::Value record;
		reader.parse(line, record);
		if (record.isMember("pmid") && record.isMember("labels"))
		{
			MultiLabelAnswer answer(record, meshSet);
			rtn = AddAnswer(answer);
			CHECK_RTN(rtn);
		}
	}
	return 0;
}

int MultiLabelAnswerSet::SaveJsonSet(const char* const fileName)
{
	if (fileName == NULL)
		return -1;
	FILE *outFile = fopen(fileName, "w");
	int rtn = 0;
	rtn = SaveJsonSet(outFile);
	CHECK_RTN(rtn);
	fclose(outFile);
	return 0;
}

int MultiLabelAnswerSet::SaveJsonSet(FILE* outFile)
{
	if (outFile == NULL)
		return -1;

	int rtn = 0;
	fprintf(outFile, "{\"documents\": [\n");
	for (unsigned i = 0; i < mAnswers.size(); ++i)
	{
		if (i>0)
			fprintf(outFile, ",\n");
		rtn = mAnswers[i].SaveJson(outFile);
		CHECK_RTN(rtn);
	}
	fprintf(outFile, "]}\n");
	return 0;
}

int MultiLabelAnswerSet::SaveUidJsonSet(const char* const fileName, const char* const meshPath)
{
	if (fileName == NULL)
		return -1;
	FILE *outFile = fopen(fileName, "w");
	FILE *meshFile = fopen(meshPath, "rb");
	if (meshFile == NULL)
	{
		cerr << "Error: Can't open mesh file in MultiLabelAnswerSet::SaveUidJsonSet()" << endl;
		return -1;
	}
	int rtn = 0;
	rtn = SaveUidJsonSet(outFile, meshFile);
	CHECK_RTN(rtn);
	fclose(outFile);
	return 0;
}

int MultiLabelAnswerSet::SaveUidJsonSet(FILE* outFile, FILE* meshFile)
{
	if (outFile == NULL || meshFile == NULL)
		return -1;

	int rtn = 0;
	MeshRecordSet meshSet;
	rtn = meshSet.Load(meshFile);
	CHECK_RTN(rtn);

	fprintf(outFile, "{\"documents\": [\n");
	for (unsigned i = 0; i < mAnswers.size(); ++i)
	{
		if (i>0)
			fprintf(outFile, ",\n");
		rtn = mAnswers[i].SaveUidJson(outFile, meshSet);
		CHECK_RTN(rtn);
	}
	fprintf(outFile, "]}\n");
	return 0;
}

int MultiLabelAnswerSet::SaveMatlabMatrix(FILE *outFile, MeshRecordSet& allMesh)
{
	if (outFile == NULL)
		return -1;

	for (size_t i = 0; i < mAnswers.size(); ++i)
		for (size_t j = 0; j < mAnswers[i].mLabels.size(); ++j )
		{
			if (allMesh[ mAnswers[i].mLabels[j] ])
			{
				fprintf(outFile, "%d %d %d\n", i+1, allMesh[ mAnswers[i].mLabels[j] ]->mUid, 1);
			}
			else
			{
				cerr << "WARNING : String " << mAnswers[i].mLabels[j] << " is Not a Mesh Term !" << endl;
			}
		}
	return 0;
}

int MultiLabelAnswerSet::SaveMatlabMatrix(const char* const fileName, MeshRecordSet& allMesh)
{
	if (fileName == NULL)
		return -1;
	FILE* outFile = fopen(fileName, "w");
	int rtn = 0;
	rtn = SaveMatlabMatrix(outFile, allMesh);
	CHECK_RTN(rtn);
	fclose(outFile);
	return 0;
}

LabelCounter::LabelCounter()
{
	mTruePostive = mTrue = mPostive = 0;
}

int LabelCounter::AddCorrect()
{
	mTruePostive++;
	mTrue++;
	mPostive++;
	return 0;
}

int LabelCounter::AddMissing()
{
	mTrue++;
	return 0;
}

int LabelCounter::AddWrong()
{
	mPostive++;
	return 0;
}

double LabelCounter::Recall()
{
	if (mTrue == 0)
		return 0;
	return (double)mTruePostive / (double)mTrue;
}

double LabelCounter::Precision()
{
	if (mPostive == 0)
		return 0;
	return (double)mTruePostive / (double)mPostive;
}

double LabelCounter::F1()
{
	double precision = Precision();
	double recall = Recall();
	if (precision + recall < EPS)
	{
		return 0;
	}
	return (2 * precision * recall / (precision + recall));
}

bool Evaluator::CmpPr(const std::pair<double, double>& p1, const std::pair<double, double>& p2)
{
	if (p1.first == p2.first)
		return p1.second > p2.second;
	return p1 < p2;
}

bool Evaluator::CmpPair(const std::pair<double, bool> &p1, const std::pair<double, bool> &p2)
{
	return p1.first > p2.first;
}

int Evaluator::GetAupr(const std::vector<double>& recall, const std::vector<double>& precision, double &aupr)
{
	aupr = 0.0;
	int rtn = 0;
	rtn = (recall.size() != precision.size());
	CHECK_RTN(rtn);
	std::vector<std::pair<double, double> > pr;
	pr.resize(recall.size());
	for (size_t i = 0; i < recall.size(); ++i)
		pr[i] = make_pair(recall[i], precision[i]);
	sort(pr.begin(), pr.end(), CmpPr);

	for (size_t i = 1; i < pr.size(); ++i)
		aupr += (pr[i - 1].second + pr[i].second)*(pr[i].first - pr[i - 1].first) / 2;

	return 0;
}

int Evaluator::GetAupr(std::vector <std::pair<double, bool> >& predictScores, double &aupr)
{
	aupr = 0.0;
	sort(predictScores.begin(), predictScores.end(), CmpPair);
	int rtn = 0;

	int trueNum = 0;
	for (size_t i = 0; i < predictScores.size(); ++i)
	if (predictScores[i].second)
		++trueNum;

	int tpNum = 0;

	std::vector<double> recall;
	std::vector<double> precision;
	recall.push_back(0.0);
	precision.push_back(1.0);
	unsigned j = 0;
	while (j < predictScores.size())
	{
		double thre = predictScores[j].first;
		for (; j < predictScores.size(); ++j)
		{
			if (predictScores[j].first + EPS < thre)
				break;

			if (predictScores[j].second)
				tpNum++;
		}

		int posNum = j;
		double rec = trueNum > 0.0 ? (double)tpNum / (double)trueNum : 0.0;
		double pre = posNum > 0.0 ? (double)tpNum / (double)posNum : 0.0;

		recall.push_back(rec);
		precision.push_back(pre);
	}

	rtn = GetAupr(recall, precision, aupr);
	CHECK_RTN(rtn);
	return 0;
}

int Evaluator::GetAuc(std::vector<std::pair<double, bool> >& predictScores, double &auc)
{
	sort(predictScores.begin(), predictScores.end(), CmpPair);
	int negative = 0;
	int positive = 0;
	int trurePositive = 0;
	int falsePositive = 0;
	double tpr = 0;
	double fpr = 0;
	double x0, y0, x1, y1;
	double area = 0;
	for (int i = 0; i < predictScores.size(); i++)
	if (predictScores[i].second)
		positive++;
	else
		negative++;
	if (negative == 0 || positive == 0)
	{
		auc = 0;
		cerr << "Error When Calc AUC!" << endl;
		return 1;
	}
	x0 = y0 = 0;
	for (int i = 0; i< predictScores.size(); i++)
	{
		if (predictScores[i].second == 1)
			trurePositive++;
		else
			falsePositive++;
		tpr = (double)trurePositive / positive;	
		fpr = (double)falsePositive / negative;
		x1 = fpr;		y1 = tpr;
		area += (x1 - x0) * (y1 + y0) / 2;
		x0 = x1;		y0 = y1;
	}
	auc = area;
	return 0;
}


int Evaluator::SaveAupr(std::map<int, double>& aupr, std::string fileName)
{
	if (fileName.empty())
		return -1;
	int rtn = 0;
	FILE *outFile = fopen(fileName.c_str(), "wb");
	if (outFile == NULL)
		return -1;
	rtn = Write(outFile, (int)aupr.size());
	CHECK_RTN(rtn);
	for (std::map<int, double>::iterator it = aupr.begin(); it != aupr.end(); ++it)
	{
		rtn = Write(outFile, it->first);
		CHECK_RTN(rtn);
		rtn = Write(outFile, it->second);
		CHECK_RTN(rtn);
	}
	fclose(outFile);
	return 0;
}

int Evaluator::LoadAupr(std::map<int, double>& aupr, std::string fileName)
{
	aupr.clear();
	if (fileName.empty())
		return -1;
	FileBuffer buffer(fileName.c_str());
	int len;
	buffer.GetNextData(len);
	for (int i = 0; i < len; ++i)
	{
		int index;
		double value;
		if (buffer.Eof())
			return -1;
		buffer.GetNextData(index);
		buffer.GetNextData(value);
		aupr[index] = value;
	}
	return 0;
}

int Evaluator::GetMaxFscore(std::vector <std::pair<double, bool> >& predictScores, double& fscore, double& precision, double& recall)
{
	sort(predictScores.begin(), predictScores.end(), CmpPair);
	int rtn = 0;

	int trueNum = 0;
	for (size_t i = 0; i < predictScores.size(); ++i)
	if (predictScores[i].second)
		++trueNum;

	int tpNum = 0;
	fscore = 0.0;
	precision = 0.0;
	recall = 0.0;
	unsigned j = 0;
	while (j < predictScores.size())
	{
		double thre = predictScores[j].first;
		for (; j < predictScores.size(); ++j)
		{
			if (predictScores[j].first + EPS < thre)
				break;

			if (predictScores[j].second)
				tpNum++;
		}

		int posNum = j;
		double rec = trueNum > 0.0 ? (double)tpNum / (double)trueNum : 0.0;
		double pre = posNum > 0.0 ? (double)tpNum / (double)posNum : 0.0;
		double f1 = (pre + rec) > 0.0 ? 2 * pre*rec / (pre + rec) : 0.0;
		if (f1 > fscore)
		{
			fscore = f1;
			recall = rec;
			precision = pre;
		}
	}

	return 0;
}

int Evaluator::SaveMap(std::map<int, double>& mapPair, std::string fileName)
{
	if (fileName.empty())
		return -1;
	int rtn = 0;
	FILE *outFile = fopen(fileName.c_str(), "wb");
	if (outFile == NULL)
		return -1;
	rtn = Write(outFile, (int)mapPair.size());
	CHECK_RTN(rtn);
	for (std::map<int, double>::iterator it = mapPair.begin(); it != mapPair.end(); ++it)
	{
		rtn = Write(outFile, it->first);
		CHECK_RTN(rtn);
		rtn = Write(outFile, it->second);
		CHECK_RTN(rtn);
	}
	fclose(outFile);
	return 0;
}

int Evaluator::LoadMap(std::map<int, double>& mapPair, std::string fileName)
{
	mapPair.clear();
	if (fileName.empty())
		return -1;
	FileBuffer buffer(fileName.c_str());
	int len;
	buffer.GetNextData(len);
	for (int i = 0; i < len; ++i)
	{
		int index;
		double value;
		if (buffer.Eof())
			return -1;
		buffer.GetNextData(index);
		buffer.GetNextData(value);
		mapPair[index] = value;
	}
	return 0;
}

Evaluator::Evaluator()
{
	mGraphExist = false;
	mMeshExist = false;
}

Evaluator::Evaluator(FILE* meshBin)
{
	LoadMesh(meshBin);
}

Evaluator::Evaluator(const char* const MeshBinfileName)
{
	LoadMesh(MeshBinfileName);
}

Evaluator::Evaluator(const char* const meshBinFileName, const char* const graphFileName)
{
	LoadMesh(meshBinFileName);
	LoadGraph(graphFileName);
}

Evaluator::~Evaluator()
{

}

int Evaluator::LoadMesh(MeshRecordSet& meshSet)
{
	int rtn = 0;
	if (mMeshExist)
	{
		cerr << "ERROR : Mesh Profile Already Exist!" << endl;
		return 1;
	}
	mAllMesh = meshSet;
	mMeshExist = true;
	return 0;
}

int Evaluator::LoadMesh(FILE* meshBin)
{
	int rtn = 0;
	if (mMeshExist)
	{
		cerr << "ERROR : Mesh Profile Already Exist!" << endl;
		return 1;
	}
	rtn = mAllMesh.Load(meshBin);
	CHECK_RTN(rtn);
	mMeshExist = true;
	return 0;
}

int Evaluator::LoadMesh(const char* const fileName)
{
	int rtn = 0;
	FILE* binFile = fopen(fileName, "rb");
	rtn = LoadMesh(binFile);
	CHECK_RTN(rtn);
	return 0;
}

int Evaluator::LoadGraph(FILE* graphFile)
{
	int rtn = 0;
	std::string x, y;
	char s1[100], s2[100];

	din.resize(maxN);
	edge.reserve(maxN);

	n = 0;
	while (fscanf(graphFile, "%s%s", s1, s2) != EOF)
	{
		x = s1;
		y = s2;
		add(getSig(x), getSig(y));
	}
	++n;
	for (int i = 1; i <= n - 1; ++i)
		if (!din[i]) add(n, i);

	printf("%d\n", n);

	mGraphExist = true;

	return 0;
}

int Evaluator::LoadGraph(const char* const fileName)
{
	int rtn = 0;
	FILE* graphFile = fopen(fileName, "r");
	rtn = LoadGraph(graphFile);
	CHECK_RTN(rtn);
	fclose(graphFile);
	return 0;
}

int Evaluator::MeshToInt(MultiLabelAnswer &mesh, std::set<int> &uid)
{
	int rtn = 0;
	uid.clear();
	for (int i = 0; i < mesh.Size(); i++)
	{
		int id;
		rtn = StringToData(mesh.mLabels[i], id);
		CHECK_RTN(rtn);
		uid.insert(id);
	}
	/*if (!mMeshExist)
	{
		cerr << "ERROR : No Mesh Profile Loaded for Evaluation !" << endl;
		return 1;
	}
	uid.clear();
	for (int i = 0; i < mesh.Size(); i++)
	if (mAllMesh[mesh[i]])
	{
		uid.insert(mAllMesh[mesh[i]]->mUid);
	}
	else
	{
		cerr << "WARNING : String " << mesh[i] << " is Not a Mesh Term !" << endl;
		//return 1;
	}*/
	return 0;
}

int Evaluator::MeshToDuId(std::string &mesh, std::string &DuId)
{
	if (!mMeshExist)
	{
		cerr << "ERROR : No Mesh Profile Loaded for Evaluation !" << endl;
		return 1;
	}
	if (mAllMesh[mesh])
	{
		DuId = intToString(mAllMesh[mesh]->mUid);
		while (DuId.size() < 6) DuId = "0" + DuId;
		DuId = "D" + DuId;
	}
	return 0;
}

template<typename T>
int Evaluator::Evaluate(std::set<T>& predictLabel, std::set<T>& truthLabel, double& precision, double& recall, double &f1)
{
	int interCount = 0;
	for (std::set<int>::iterator it = predictLabel.begin(); it != predictLabel.end(); it++)
	if (truthLabel.count(*it))
		interCount++;
	if (predictLabel.size() > 0)
		precision = (double)interCount / (double)predictLabel.size();
	else
		precision = 0;
	if (truthLabel.size() > 0)
		recall = (double)interCount / (double)truthLabel.size();
	else
		recall = 0;
	if (precision > EPS || recall > EPS)
		f1 = 2 * precision * recall / (precision + recall);
	else
		f1 = 0;
	return 0;
}

int Evaluator::FlatEvaluate(MultiLabelAnswer &predict, MultiLabelAnswer &truth, double &precision, double& recall, double &f1)
{
	int rtn = 0;
	std::set<int> predictLabel;
	std::set<int> truthLabel;
	rtn = MeshToInt(predict, predictLabel);
	CHECK_RTN(rtn);
	rtn = MeshToInt(truth, truthLabel);
	CHECK_RTN(rtn);
	rtn = Evaluate(predictLabel, truthLabel, precision, recall, f1);
	CHECK_RTN(rtn);
	return 0;
}

bool Evaluator::CheckValid(MultiLabelAnswerSet &predict, MultiLabelAnswerSet &truth)
{
	if (predict.Size() != truth.Size())
	{
		cerr << "Predict / GoldStandard size not match" << endl;
		return false;
	}
	predict.SortByPmid();
	truth.SortByPmid();
	for (int i = 0; i < predict.Size(); i++)
	if (predict[i].mPmid != truth[i].mPmid)
	{
		cerr << "Predict / GoldStandard pmid not match" << endl;
		return false;
	}
	return true;
}

int Evaluator::LabelBasedMacroEvaluate(MultiLabelAnswerSet &predict, MultiLabelAnswerSet &truth, double &precision, double& recall, double &f1, std::string labelAnalyseFileName)
{
	int rtn = 0;

	if (!CheckValid(predict, truth))
	{
		return 1;
	}

	std::map<int, LabelCounter> labels;
	std::set<int> inTrueList;
	labels.clear();
	inTrueList.clear();

	for (int i = 0; i < predict.Size(); i++)
	{
		std::set<int> predictLabel;
		std::set<int> trueLabel;
		rtn = MeshToInt(predict[i], predictLabel);
		CHECK_RTN(rtn);
		rtn = MeshToInt(truth[i], trueLabel);
		CHECK_RTN(rtn);
		for (std::set<int>::iterator it = predictLabel.begin(); it != predictLabel.end(); it++)
		if (trueLabel.count(*it) > 0)
		{
			rtn = labels[*it].AddCorrect();
			CHECK_RTN(rtn);
		}
		else
		{
			rtn = labels[*it].AddWrong();
			CHECK_RTN(rtn);
		}
		for (std::set<int>::iterator it = trueLabel.begin(); it != trueLabel.end(); it++)
		{
			inTrueList.insert(*it);
			if (predictLabel.count(*it) == 0)
			{
				rtn = labels[*it].AddMissing();
				CHECK_RTN(rtn);
			}
		}
	}

	precision = recall = f1 = 0;
	for (std::map<int, LabelCounter>::iterator it = labels.begin(); it != labels.end(); it++)
	if (inTrueList.count(it->first) > 0)
	{
		precision += it->second.Precision();
		recall += it->second.Recall();
		f1 += it->second.F1();
	}
	if (inTrueList.size() == 0)
	{
		cerr << "ERROR : error while evaluation, truelist is empty !" << endl;
		return 1;
	}
	precision /= inTrueList.size();
	recall /= inTrueList.size();
	f1 /= inTrueList.size();

	if (labelAnalyseFileName != "")
	{
		FILE* analyseFile = fopen(labelAnalyseFileName.c_str(), "w");
		fprintf(analyseFile, "MeshID,MeshName,TruePostive,True,Postive,Precision,Recall,FMeasure\n");
		for (std::map<int, LabelCounter>::iterator it = labels.begin(); it != labels.end(); it++)
		{
			fprintf(analyseFile, "%d,noname,%d,%d,%d,%.7f,%.7f,%.7f\n",it->first, /*mAllMesh[it->first]->mName,*/ it->second.mTruePostive, it->second.mTrue, it->second.mPostive, it->second.Precision(), it->second.Recall(), it->second.F1());
		}
		fclose(analyseFile);
	}

	return 0;
}

int Evaluator::LabelBasedMicroEvaluate(MultiLabelAnswerSet &predict, MultiLabelAnswerSet &truth, double &precision, double& recall, double &f1)
{
	int rtn = 0;
	if (!CheckValid(predict, truth))
	{
		return 1;
	}
	LabelCounter microEvaltor;
	for (int i = 0; i < predict.Size(); i++)
	{
		std::set<int> predictLabel;
		std::set<int> trueLabel;
		rtn = MeshToInt(predict[i], predictLabel);
		CHECK_RTN(rtn);
		rtn = MeshToInt(truth[i], trueLabel);
		CHECK_RTN(rtn);
		for (std::set<int>::iterator it = predictLabel.begin(); it != predictLabel.end(); it++)
		if (trueLabel.count(*it) > 0)
		{
			rtn = microEvaltor.AddCorrect();
			CHECK_RTN(rtn);
		}
		else
		{
			rtn = microEvaltor.AddWrong();
			CHECK_RTN(rtn);
		}
		for (std::set<int>::iterator it = trueLabel.begin(); it != trueLabel.end(); it++)
		if (predictLabel.count(*it) == 0)
		{
			rtn = microEvaltor.AddMissing();
			CHECK_RTN(rtn);
		}
	}
	precision = microEvaltor.Precision();
	recall = microEvaltor.Recall();
	f1 = microEvaltor.F1();
	return 0;
}

int Evaluator::ExampleBasedEvaluate(MultiLabelAnswerSet &predict, MultiLabelAnswerSet &truth, double &precision, double& recall, double &f1, std::string exampleAnalyseFileName)
{
	int rtn = 0;
	if (!CheckValid(predict, truth))
	{
		return 1;
	}
	precision = recall = f1 = 0;

	FILE *outFile = NULL;
	if (exampleAnalyseFileName != "")
	{
		outFile = fopen(exampleAnalyseFileName.c_str(), "w");
	}
	if (outFile)
	{
		fprintf(outFile, "PMID,Precision,Recall,F-Score,Prediction\n");
	}
	for (int i = 0; i < predict.Size(); i++)
	{
		double examplePrecision;
		double exampleRecall;
		double exampleF1;
		rtn = FlatEvaluate(predict[i], truth[i], examplePrecision, exampleRecall, exampleF1);
		CHECK_RTN(rtn);
		precision += examplePrecision;
		recall += exampleRecall;
		f1 += exampleF1;

		if (outFile)
		{
			fprintf(outFile, "%d,%.6f,%.6f,%.6f", predict[i].mPmid, examplePrecision, exampleRecall, exampleF1);
			fprintf(outFile, ",\"");
			for (int j = 0; j < predict[i].mLabels.size(); j++)
			{
				fprintf(outFile, "%s|", predict[i].mLabels[j].c_str());
			}
			fprintf(outFile, "\"\n");
		}
	}
	if (outFile)
		fclose(outFile);
	precision /= predict.Size();
	recall /= predict.Size();
	f1 /= predict.Size();
	return 0;
}


int Evaluator::HierarchicalEvaluate(MultiLabelAnswer &predict, MultiLabelAnswer &truth, double &precision, double& recall, double &f1, int runid)
{
	if (!mGraphExist)
	{
		cerr << "ERROR : No Hierarchical Profile for Evaluate !" << endl;
		return 1;
	}

	
	
	std::set <int> tmps;
	setT[runid] = tmps;
	setP[runid] = tmps; 
	setLCA[runid] = tmps; 
	nowGra[runid] = tmps;
	vis[runid] = tmps;

	std::vector<int> tmpv;
	disS[runid] = tmpv;
	disT[runid] = tmpv;

	disS[runid].resize(maxN);
	disT[runid].resize(maxN);
	
	int rtn = 0;
	std::vector<int> tru, pre;
	std::string DuId;

	for (int i = 0; i < truth.mLabels.size(); ++i)
	{
		if (strstr(truth.mLabels[i].c_str(), "_") != NULL)
			DuId = truth.mLabels[i];
		else
		{
			rtn = MeshToDuId(truth.mLabels[i], DuId);
			CHECK_RTN(rtn);
		}
		tru.push_back(getSig(DuId));
	}

	for (int i = 0; i < predict.mLabels.size(); ++i)
	{
		if (strstr(predict.mLabels[i].c_str(), "_") != NULL)
			DuId = predict.mLabels[i];
		else
		{
			rtn = MeshToDuId(predict.mLabels[i], DuId);
			CHECK_RTN(rtn);
		}
		pre.push_back(getSig(DuId));

	}
	
	setT[runid].clear();
	setP[runid].clear();

	for (int i = 0; i < tru.size(); ++i)
		setT[runid].insert(tru[i]);
	for (int i = 0; i < pre.size(); ++i)
		setP[runid].insert(pre[i]);

	std::vector<int> tmp;
	std::vector<int> sLCAt[maxMesh], sLCAp[maxMesh];
	std::vector<int> sBestt[maxMesh], sBestp[maxMesh];
	setLCA[runid].clear();

	for (int i = 0; i < tru.size(); ++i)
	{
		sBestt[i] = getSBest(tru[i], 0, runid);
		for (int j = 0; j < sBestt[i].size(); ++j)
		{
			tmp = LCA(tru[i], sBestt[i][j]);
			for (int k = 0; k < tmp.size(); ++k)
				sLCAt[i].push_back(tmp[k]);
		}

		myUnique(sLCAt[i]);
		for (int j = 0; j < sLCAt[i].size(); ++j)
			setLCA[runid].insert(sLCAt[i][j]);
	}

	for (int i = 0; i < pre.size(); ++i)
	{
		sBestp[i] = getSBest(pre[i], 1, runid);
		for (int j = 0; j < sBestp[i].size(); ++j)
		{
			tmp = LCA(pre[i], sBestp[i][j]);
			for (int k = 0; k < tmp.size(); ++k)
				sLCAp[i].push_back(tmp[k]);
		}

		myUnique(sLCAp[i]);
		for (int j = 0; j < sLCAp[i].size(); ++j)
			setLCA[runid].insert(sLCAp[i][j]);
	}

	std::vector<int> gt, gp;

	gt = getGraph(tru, pre, sBestt, sBestp, sLCAt, sLCAp, runid);
	gp = getGraph(pre, tru, sBestp, sBestt, sLCAp, sLCAt, runid);

	int p1, p2, cnt;
	p1 = 0; p2 = 0; cnt = 0;
	while (p1 < gt.size() && p2 < gp.size())
	{
		if (gt[p1] < gp[p2]) ++p1;
		else if (gt[p1] > gp[p2]) ++p2;
		else
		{
			++p1; ++p2;
			++cnt;
		}
	}

	double p, r, f;
	p = gp.size() > 0.0 ? double(cnt) / gp.size() : 0.0;
	r = gt.size() > 0.0 ? double(cnt) / gt.size() : 0.0;
	f = (p + r) > 0.0 ? 2 * p * r / (p + r) : 0.0;

	precision = p;
	recall = r;
	f1 = f;

	return 0;
}

int Evaluator::HierarchicalEvaluate(MultiLabelAnswerSet &predict, MultiLabelAnswerSet &truth, double &precision, double& recall, double &f1)
{
	if (!mGraphExist)
	{
		cerr << "ERROR : No Hierarchical Profile for Evaluate !" << endl;
		return 1;
	}
	int rtn = 0;
	if (!CheckValid(predict, truth))
	{
		return 1;
	}
	precision = recall = f1 = 0;

	std::vector<double> examplePrecision;
	std::vector<double> exampleRecall;
	std::vector<double> exampleF1;
	examplePrecision.resize(predict.Size());
	exampleRecall.resize(predict.Size());
	exampleF1.resize(predict.Size());

	int numThreads = omp_get_num_procs();
	omp_set_num_threads(numThreads);

	setT.resize(predict.Size());
	setP.resize(predict.Size());
	setLCA.resize(predict.Size());
	nowGra.resize(predict.Size());
	vis.resize(predict.Size());
	disS.resize(predict.Size());
	disT.resize(predict.Size());

#pragma omp parallel for schedule(dynamic)
	for (int i = 0; i < predict.Size(); i++)
	{
		int rtn = 0;
		rtn = HierarchicalEvaluate(predict[i], truth[i], examplePrecision[i], exampleRecall[i], exampleF1[i], i);
		//CHECK_RTN(rtn);
		if (rtn != 0) clog << "ERR at predict id = " << i << endl;

		erasemem(i);
	}

	for (size_t i = 0; i < exampleF1.size(); ++i)
	{
		precision += examplePrecision[i];
		recall += exampleRecall[i];
		f1 += exampleF1[i];
	}

	precision /= predict.Size();
	recall /= predict.Size();
	f1 /= predict.Size();

	return 0;
}
int Evaluator::NumlabelEvaluate(std::map<int, int>& predictNum, std::map<int, int>& goldNum, double& result, std::string numAnalyseFileName)
{
	result = -1.0;
	if (predictNum.size() != goldNum.size())
		return -1;

	FILE *outFile = NULL;
	if (numAnalyseFileName != "")
	{
		outFile = fopen(numAnalyseFileName.c_str(), "w");
		fprintf(outFile, "pmid,predictNum,goldNum,distance\n");
	}
	double sum = 0;
	for (std::map<int, int>::iterator it = predictNum.begin(), it2 = goldNum.begin(); it != predictNum.end() && it2 != goldNum.end(); ++it, ++it2)
	{
		if (it->first != it2->first)
			return -1;
		sum += (it->second - it2->second)*(it->second - it2->second);
		fprintf(outFile, "%d,%d,%d,%d\n", it->first, it->second, it2->second, (int)abs(it->second - it2->second));
	}
	fclose(outFile);
	result = sqrt(sum / predictNum.size());
	return 0;
}

void Evaluator::myUnique(std::vector<int> &a)
{
	sort(a.begin(), a.end());
	std::vector<int>::iterator it = unique(a.begin(), a.end());
	a.erase(it, a.end());
}

int Evaluator::getSig(std::string s)
{
	if (sig.count(s)) return sig[s];
	din[++n] = 0;
	return sig[s] = n;
}

void Evaluator::add(int x, int y)
{
	++din[y];
	edge[x].push_back(make_pair(y, false));
	edge[y].push_back(make_pair(x, true));
}

std::vector<int> Evaluator::LCA(int x, int y)
{
	std::queue<int> Q;
	std::set<int> S, T, del;
	std::vector<int> ans;

	int tmp;
	Q.push(x); S.insert(x);
	while (!Q.empty())
	{
		x = Q.front();
		for (std::vector<std::pair<int, bool> >::iterator p = edge[x].begin(); p != edge[x].end(); ++p)
			if ((*p).second == 1)
			{
				tmp = (*p).first;
				if (S.find(tmp) == S.end())
				{
					Q.push(tmp);
					S.insert(tmp);
				}
			}
		Q.pop();
	}

	Q.push(y); T.insert(y);
	while (!Q.empty())
	{
		x = Q.front();
		if (S.find(x) != S.end())
		{
			ans.push_back(x);
			Q.pop();
			continue;
		}
		for (std::vector<std::pair<int, bool> >::iterator p = edge[x].begin(); p != edge[x].end(); ++p)
			if ((*p).second == 1)
			{
				tmp = (*p).first;
				if (T.find(tmp) == T.end())
				{
					Q.push(tmp);
					T.insert(tmp);
				}
			}
		Q.pop();
	}

	for (int i = 0; i < ans.size(); ++i)
		Q.push(ans[i]);

	while (!Q.empty())
	{
		x = Q.front();
		for (std::vector<std::pair<int, bool> >::iterator p = edge[x].begin(); p != edge[x].end(); ++p)
			if ((*p).second == 1)
			{
				tmp = (*p).first;
				if (del.find(tmp) == del.end())
				{
					Q.push(tmp);
					del.insert(tmp);
				}
			}
		Q.pop();
	}

	std::vector<int> ret;

	for (int i = 0; i < ans.size(); ++i)
		if (del.find(ans[i]) == del.end())
			ret.push_back(ans[i]);

	return ret;
}

std::vector<int> Evaluator::LCA(int x, std::vector<int> S)
{
	std::vector<int> LCAs, tmp;
	for (int i = 0; i < S.size(); ++i)
	{
		tmp = LCA(x, S[i]);
		for (int j = 0; j < tmp.size(); ++j)
			LCAs.push_back(tmp[j]);
	}
	myUnique(LCAs);
	return LCAs;
}


std::vector<int> Evaluator::getSBest(int x, bool bo, int runid)
{
	std::queue<std::pair<int, int> > Q;
	std::set<int> S;
	std::vector<int> ans;
	int minPath = 19950108;
	int y;

	Q.push(make_pair(x, 0));
	S.insert(x);
	while (!Q.empty() && Q.front().second <= minPath)
	{
		x = Q.front().first;
		if (bo == 0 && setP[runid].find(x) != setP[runid].end())
			minPath = Q.front().second;
		if (bo == 1 && setT[runid].find(x) != setT[runid].end())
			minPath = Q.front().second;
		if (Q.front().second == minPath)
		{
			if (bo == 0 && setP[runid].find(x) != setP[runid].end())
				ans.push_back(x);
			if (bo == 1 && setT[runid].find(x) != setT[runid].end())
				ans.push_back(x);
		}
		for (std::vector<std::pair<int, bool> >::iterator p = edge[x].begin(); p != edge[x].end(); ++p)
		{
			y = (*p).first;
			if (S.find(y) != S.end()) continue;
			S.insert(y);
			Q.push(make_pair(y, Q.front().second + 1));
		}
		Q.pop();
	}

	return ans;
}


std::vector<int> Evaluator::getPath(int s, int t, int runid)
{
	std::queue<int> Q;
	std::set<int> boS, boT;
	std::vector<int> ans;
	int x, y, minPath;

	Q.push(s); disS[runid][s] = 0; boS.insert(s);
	while (!Q.empty())
	{
		x = Q.front();
		if (x == t) break;
		for (std::vector<std::pair<int, bool> >::iterator p = edge[x].begin(); p != edge[x].end(); ++p)
		{
			y = (*p).first;
			if (boS.find(y) == boS.end())
			{
				Q.push(y);
				disS[runid][y] = disS[runid][x] + 1;
				boS.insert(y);
			}
		}
		Q.pop();
	}

	while (!Q.empty()) Q.pop();

	Q.push(t); disT[runid][t] = 0; boT.insert(t);
	while (!Q.empty())
	{
		x = Q.front();
		if (x == s) break;
		for (std::vector<std::pair<int, bool> >::iterator p = edge[x].begin(); p != edge[x].end(); ++p)
		{
			y = (*p).first;
			if (boT.find(y) == boT.end())
			{
				Q.push(y);
				disT[runid][y] = disT[runid][x] + 1;
				boT.insert(y);
			}
		}
		Q.pop();
	}
	minPath = disS[runid][t];
	for (std::set<int>::iterator p = boS.begin(); p != boS.end(); ++p)
	{
		if (boT.find(*p) != boT.end() && disS[runid][*p] + disT[runid][*p] == minPath)
			ans.push_back(*p);
	}

	return ans;
}


void Evaluator::dfs(int x, bool &f1, bool &f2, int runid)
{
	vis[runid].insert(x);
	if (setT[runid].find(x) != setT[runid].end()) f1 = 1;
	if (setP[runid].find(x) != setP[runid].end()) f1 = 1;
	if (setLCA[runid].find(x) != setLCA[runid].end()) f2 = 1;
	for (std::vector<std::pair<int, bool> >::iterator p = edge[x].begin(); p != edge[x].end(); ++p)
		if (vis[runid].find((*p).first) == vis[runid].end() && nowGra[runid].find((*p).first) != nowGra[runid].end())
			dfs((*p).first, f1, f2, runid);
}


bool Evaluator::check(int runid)
{
	bool f1, f2;
	vis[runid].clear();
	for (std::set<int>::iterator p = nowGra[runid].begin(); p != nowGra[runid].end(); ++p)
		if (vis[runid].find(*p) == vis[runid].end())
		{
			f1 = f2 = 0;
			dfs(*p, f1, f2, runid);
			//printf("%d %d %d\n", *p, f1, f2);
			if (!f1 || !f2) return 0;
		}
	return 1;
}


std::vector<int> Evaluator::getGraph(std::vector<int> X, std::vector<int> Y, std::vector<int> xBest[maxMesh], std::vector<int> yBest[maxMesh], std::vector<int> xLCA[maxMesh], std::vector<int> yLCA[maxMesh], int runid)
{
	std::vector<int> s, tmp;

	for (int i = 0; i < X.size(); ++i)
		for (int j = 0; j < xLCA[i].size(); ++j)
		{
			tmp = getPath(X[i], xLCA[i][j], runid);
			s.insert(s.end(), tmp.begin(), tmp.end());
		}

	for (int i = 0; i < Y.size(); ++i)
		for (int j = 0; j < yBest[i].size(); ++j)
			for (int k = 0; k < yLCA[i].size(); ++k)
			{
				tmp = getPath(yBest[i][j], yLCA[i][k], runid);
				s.insert(s.end(), tmp.begin(), tmp.end());
			}

	myUnique(s);
	nowGra[runid].clear();
	for (int i = 0; i < s.size(); ++i)
		nowGra[runid].insert(s[i]);

	for (int i = 0; i < s.size(); ++i)
	{
		if (setT[runid].find(s[i]) != setT[runid].end()) continue;
		if (setP[runid].find(s[i]) != setP[runid].end()) continue;
		nowGra[runid].erase(nowGra[runid].find(s[i]));
		if (!check(runid)) nowGra[runid].insert(s[i]);
	}

	s.clear();
	for (std::set<int>::iterator p = nowGra[runid].begin(); p != nowGra[runid].end(); ++p)
		s.push_back(*p);
	return s;
}

void Evaluator::erasemem(int runid)
{
	setT[runid].erase(setT[runid].begin(), setT[runid].end());
	setP[runid].erase(setP[runid].begin(), setP[runid].end());
	setLCA[runid].erase(setLCA[runid].begin(), setLCA[runid].end());
	nowGra[runid].erase(nowGra[runid].begin(), nowGra[runid].end());
	vis[runid].erase(vis[runid].begin(), vis[runid].end());;
	disS[runid].resize(0);
	disT[runid].resize(0);
}