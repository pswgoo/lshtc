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
	//正样本采样率，大于等于1.0
	double mSamplingRate;

	//存储每个mesh对应模型的分数查询表
	std::map<int, QueryTable> mTableSet;

public:
	ScoreTableSet(const double samplingRate = 1000.0);

	//用某个测试集的predictscore的表初始化所有模型的分数查询表，exampleScore是score表，goldSet每维对应score表相应维的标准mesh集,构造函数初始化为precision表
	ScoreTableSet(const std::vector<std::vector<std::pair<int, double>>>& exampleScore, const std::vector<std::set<int>>& goldSet, const double samplingRate = 1000.0);

	//用每个mesh和它对所有文章的打分来初始化分数查询表,函数会对labelScore排序,构造函数初始化为precision表
	ScoreTableSet(std::map<int, std::vector<std::pair<double, bool>>>& labelScore, const double samplingRate = 1000.0);

	~ScoreTableSet();

	//添加precision查询表
	int AddPrecisionTable(std::pair<int, std::vector<std::pair<double, bool>>> labelScore);

	//添加recall查询表
	int AddRecallTable(std::pair<int, std::vector<std::pair<double, bool>>> labelScore);

	//用某个测试集的predictscore的表初始化所有模型的分数查询表，exampleScore是score表，goldSet每维对应score表相应维的标准mesh集,初始化precision查询表
	int InitializePrecisionTable(const std::vector<std::vector<std::pair<int, double>>>& exampleScore, const std::vector<std::set<int>>& goldSet);

	//用每个mesh和它对所有文章的打分来初始化分数查询表,初始化precision查询表
	int InitializePrecisionTable(std::map<int, std::vector<std::pair<double, bool>>>& labelScore);

	//用某个测试集的predictscore的表初始化所有模型的分数查询表，exampleScore是score表，goldSet每维对应score表相应维的标准mesh集,初始化recall查询表
	int InitializeRecallTable(const std::vector<std::vector<std::pair<int, double>>>& exampleScore, const std::vector<std::set<int>>& goldSet);

	//用每个mesh和它对所有文章的打分来初始化分数查询表,初始化recall查询表
	int InitializeRecallTable(std::map<int, std::vector<std::pair<double, bool>>>& labelScore);

	//用meshId,score来查询对应模型和score的precision
	int Query(int labelId, double threshold, double& value);

	//返回对应mesh模型的分数查询表
	QueryTable* operator[](const int labelId);

	int Size();

	int Count(int labelId);

	int Save(const std::string& fileName, int printLog = SILENT);

	int Save(FILE* outFile, int printLog = SILENT);

	int Load(const std::string& fileName, int printLog = SILENT);

	//调用下面函数Load时，数据是从文件开始处读取的
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

	//给定一个测试集的predictscore表和对应的gold集，用每个mesh模型在该测试集的gold个数来划定各个模型的阈值
	int InitializeByRank(const std::vector<std::vector<std::pair<int, double>>>& exampleScore, const std::vector<std::set<int>>& goldSet);

	//给定每个mesh和它对所有文章的打分，用mesh在这个数据集上的gold个数划定其模型阈值
	int InitializeByRank(std::map<int, std::vector<std::pair<double, bool>>>& labelScore);

	//给定一个测试集的predictscore表和对应的gold集，用使得该mesh模型在这个测试集上取最高fscore的threshold作为阈值（会加precision限制）
	int InitializeByMaxFscore(const std::vector<std::vector<std::pair<int, double>>>& exampleScore, const std::vector<std::set<int>>& goldSet);

	//给定每个mesh和它对所有文章的打分，用使得该mesh模型在这个测试集上取最高fscore的threshold作为阈值（会加precision限制）
	int InitializeByMaxFscore(std::map<int, std::vector<std::pair<double, bool>>>& labelScore);

	//查询对应模型的阈值
	int Query(const int labelId, double& value);

	//查询对应模型的阈值
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