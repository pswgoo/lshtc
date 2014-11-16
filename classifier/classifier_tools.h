#ifndef CLASSIFIER_TOOLS_H
#define CLASSIFIER_TOOLS_H

#include "common/common_basic.h"
#include "common/file_utility.h"
#include "common/data_struct.h"
#include "extractfeature/feature.h"
#include "medline/mesh.h"
#include "mylinear.h"

class ScoreTableSet;

int SaveLabelFreq(std::string fileName, std::map<int, int>& labelCount, int totInstance);

int SaveLabelFreq(std::string fileName, std::map<int, double>& labelFreq);

int LoadLabelFreq(std::string fileName, std::map<int, double>& labelFreq);

bool CmpPair(const std::pair<double, bool> &p1, const std::pair<double, bool> &p2);

int SaveThreshold(std::map<int, double>& threshold, const char* const fileName);

int LoadThreshold(std::map<int, double>& threshold, const char* const fileName);

int NormalizePredictScore(std::vector<std::vector<std::pair<int, double> > >& tarScore, std::map<int, double>& threshold);

int NormalizePredictScore(std::vector<std::vector<std::pair<int, double> > >& tarScore, ScoreTableSet& scoreTableSet);

double NormalizeScore(double origScore, double threshold);

class ScoreTableSet
{
private:
	//�����������ʣ����ڵ���1.0
	double mSamplingRate;

	//�洢ÿ��mesh��Ӧģ�͵ķ�����ѯ��
	std::map<int, QueryTable> mTableSet;

public:
	ScoreTableSet(const double samplingRate = 1000.0);

	//��ĳ�����Լ���predictscore�ı��ʼ������ģ�͵ķ�����ѯ��exampleScore��score��goldSetÿά��Ӧscore����Ӧά�ı�׼mesh��,���캯����ʼ��Ϊprecision��
	ScoreTableSet(const std::vector<std::vector<std::pair<int, double>>>& exampleScore, const std::vector<std::set<int>>& goldSet, const double samplingRate = 1000.0);

	//��ÿ��mesh�������������µĴ������ʼ��������ѯ��,�������labelScore����,���캯����ʼ��Ϊprecision��
	ScoreTableSet(std::map<int, std::vector<std::pair<double, bool>>>& labelScore, const double samplingRate = 1000.0);

	~ScoreTableSet();

	//���precision��ѯ��
	int AddPrecisionTable(std::pair<int, std::vector<std::pair<double, bool>>> labelScore);

	//���recall��ѯ��
	int AddRecallTable(std::pair<int, std::vector<std::pair<double, bool>>> labelScore);

	//��ĳ�����Լ���predictscore�ı��ʼ������ģ�͵ķ�����ѯ��exampleScore��score��goldSetÿά��Ӧscore����Ӧά�ı�׼mesh��,��ʼ��precision��ѯ��
	int InitializePrecisionTable(const std::vector<std::vector<std::pair<int, double>>>& exampleScore, const std::vector<std::set<int>>& goldSet);

	//��ÿ��mesh�������������µĴ������ʼ��������ѯ��,��ʼ��precision��ѯ��
	int InitializePrecisionTable(std::map<int, std::vector<std::pair<double, bool>>>& labelScore);

	//��ĳ�����Լ���predictscore�ı��ʼ������ģ�͵ķ�����ѯ��exampleScore��score��goldSetÿά��Ӧscore����Ӧά�ı�׼mesh��,��ʼ��recall��ѯ��
	int InitializeRecallTable(const std::vector<std::vector<std::pair<int, double>>>& exampleScore, const std::vector<std::set<int>>& goldSet);

	//��ÿ��mesh�������������µĴ������ʼ��������ѯ��,��ʼ��recall��ѯ��
	int InitializeRecallTable(std::map<int, std::vector<std::pair<double, bool>>>& labelScore);

	//��meshId,score����ѯ��Ӧģ�ͺ�score��precision
	int Query(int labelId, double threshold, double& value);

	//���ض�Ӧmeshģ�͵ķ�����ѯ��
	QueryTable* operator[](const int labelId);

	int Size();

	int Count(int labelId);

	int Save(const std::string& fileName, int printLog = SILENT);

	int Save(FILE* outFile, int printLog = SILENT);

	int Load(const std::string& fileName, int printLog = SILENT);

	//�������溯��Loadʱ�������Ǵ��ļ���ʼ����ȡ��
	int Load(FILE* inFile, int printLog = SILENT);

	int Load(FileBuffer& buffer, int printLog = SILENT);

};

class Threshold
{
private:
	std::map<int, double> mThreshold;

public:
	static int GetThreshold(LinearMachine& queryModel, FeatureSet& featureSet, std::vector<bool>& trueLabels, double& bestThres);

	static int GetThreshold(LinearMachine& queryModel, feature_node** featureSet, std::vector<bool>& trueLabels, double& bestThres);

	static int GetThreshold(std::vector<std::pair<double, bool> > &modelScores, double& bestThres);

public:
	Threshold();

	Threshold(const std::map<int, double>& threshold);

	~Threshold();

	int AddInstanceByRank(std::pair<int, std::vector<std::pair<double, bool>>> labelScore);

	int AddInstanceByMaxFscore(std::pair<int, std::vector<std::pair<double, bool>>> labelScore);

	//����һ�����Լ���predictscore��Ͷ�Ӧ��gold������ÿ��meshģ���ڸò��Լ���gold��������������ģ�͵���ֵ
	int InitializeByRank(const std::vector<std::vector<std::pair<int, double>>>& exampleScore, const std::vector<std::set<int>>& goldSet);

	//����ÿ��mesh�������������µĴ�֣���mesh��������ݼ��ϵ�gold����������ģ����ֵ
	int InitializeByRank(std::map<int, std::vector<std::pair<double, bool>>>& labelScore);

	//����һ�����Լ���predictscore��Ͷ�Ӧ��gold������ʹ�ø�meshģ����������Լ���ȡ���fscore��threshold��Ϊ��ֵ�����precision���ƣ�
	int InitializeByMaxFscore(const std::vector<std::vector<std::pair<int, double>>>& exampleScore, const std::vector<std::set<int>>& goldSet);

	//����ÿ��mesh�������������µĴ�֣���ʹ�ø�meshģ����������Լ���ȡ���fscore��threshold��Ϊ��ֵ�����precision���ƣ�
	int InitializeByMaxFscore(std::map<int, std::vector<std::pair<double, bool>>>& labelScore);

	//��ѯ��Ӧģ�͵���ֵ
	int Query(const int labelId, double& value);

	//��ѯ��Ӧģ�͵���ֵ
	double operator[](const int labelId);

	std::map<int, double> GetThreshold();

	int Size();

	int Save(const std::string& fileName, int printLog = SILENT);

	int Save(FILE* outFile, int printLog = SILENT);

	int Load(const std::string& fileName, int printLog = SILENT);

	int Load(FILE* inFile, int printLog = SILENT);

	int Load(FileBuffer& buffer, int printLog = SILENT);
};

#endif