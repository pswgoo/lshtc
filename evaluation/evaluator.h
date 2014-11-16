#ifndef _EVALUATOR_H
#define _EVALUATOR_H
#include "medline/basic.h"
#include "medline/citation.h"
#include "medline/mesh.h"
#include "jsoncpp/json.h"
#include "tools.h"
#include <string>
#include <map>
#include <set>
#include <queue>
const int MAX_UID = 10000000;
const int MAX_WORD_LENGTH = 1000;

class MultiLabelAnswer
{
public:
	int mPmid;
	std::vector<std::string> mLabels;

	MultiLabelAnswer();
	MultiLabelAnswer(const int pmid);
	MultiLabelAnswer(const int pmid, const std::vector<std::string> &labels);
	MultiLabelAnswer(const Citation* const citation);
	MultiLabelAnswer(const Json::Value &root);
	MultiLabelAnswer(const Json::Value &root, MeshRecordSet &meshSet);
	~MultiLabelAnswer();

	std::string operator[](int index);

	int Clear();

	int Size();

	int SaveJson(FILE* outFile);

	int SaveUidJson(FILE* outFile, MeshRecordSet& meshSet);
};

class MultiLabelAnswerSet
{
private:
	static bool CmpPmid(const MultiLabelAnswer& answer1, const MultiLabelAnswer &answer2);
	std::vector<MultiLabelAnswer> mAnswers;
public:

	static int SavePredictScores(const std::vector < std::vector<std::pair<int, double> > >& predictScores, const char* const fileName);

	static int LoadPredictScores(std::vector < std::vector<std::pair<int, double> > >& predictScores, const char* const fileName);

	static int LoadPredictScores(std::vector < std::vector<std::pair<int, double> > >& predictScores, double threshold, const std::string& fileName, int printLog = SILENT);

	static int SaveNumLabel(std::map<int, int>& numLabels, const char* const fileName);

	static int LoadNumLabel(std::map<int, int>& numLabels, const char* const fileName);

	std::set<std::string> mLabelSet;
	MultiLabelAnswerSet();
	~MultiLabelAnswerSet();
	MultiLabelAnswer& operator[](int index);

	int AddAnswer(MultiLabelAnswer answer);

	int Size();

	int LabelSize();
	
	int SortByPmid();

	int LoadJsonSet(const char* const fileName);

	int LoadJsonSet(FILE* inFile);

	int LoadUidJsonSet(const char* const fileName, const char* const meshPath);

	int LoadUidJsonSet(FILE* inFile, FILE* meshFile);

	int AddUidJsonSet(const char* const fileName, const char* const meshPath);

	int AddUidJsonSet(FILE* inFile, FILE* meshFile);

	int SaveJsonSet(const char* const fileName);

	int SaveJsonSet(FILE* outFile);

	int SaveUidJsonSet(const char* const fileName, const char* const meshPath);

	int SaveUidJsonSet(FILE* outFile, FILE* meshFile);

	int SaveMatlabMatrix(FILE *outFile, MeshRecordSet& allMesh);

	int SaveMatlabMatrix(const char* const fileName, MeshRecordSet& allMesh);
};

class LabelCounter
{
public:
	long long mTruePostive;
	long long mTrue;
	long long mPostive;

	LabelCounter();

	int AddCorrect();

	int AddMissing();

	int AddWrong();

	double Recall();
	
	double Precision();
	
	double F1();
};

//Modified by PengShengwen for LSHTC in 2014/11/12, modified function MeshToInt()
class Evaluator
{
private:
	/*graph*/
	const static int maxN = 80000; //max mesh id
	const static int maxMesh = 40; //max mesh a ciation will have.
	std::map<std::string, int> sig;
	std::vector<std::vector<std::pair<int, bool> >> edge;
	std::vector<std::set<int>> setT, setP, setLCA, nowGra, vis;
	int n;
	std::vector<int> din;
	std::vector<std::vector<int>> disS, disT;

	int getSig(std::string s);
	void add(int x, int y);
	void myUnique(std::vector<int> &a);
	void dfs(int x, bool &f1, bool &f2, int runid);
	void erasemem(int runid);

	bool check(int runid);

	std::vector<int> LCA(int x, int y);
	std::vector<int> LCA(int x, std::vector<int> S);
	std::vector<int> getSBest(int x, bool bo, int runid);
	std::vector<int> getGraph(std::vector<int> X, std::vector<int> Y, std::vector<int> xBest[maxMesh], std::vector<int> yBest[maxMesh], std::vector<int> xLCA[maxMesh], std::vector<int> yLCA[maxMesh],int runid);
	std::vector<int> getPath(int x, int y, int runid);
	/*graph end*/

public:
	bool mGraphExist;
	bool mMeshExist;
	MeshRecordSet mAllMesh;


	static bool CmpPair(const std::pair<double, bool> &p1, const std::pair<double, bool> &p2);

	static bool CmpPr(const std::pair<double, double>& p1, const std::pair<double, double>& p2);

	static int GetAupr(const std::vector<double>& recall, const std::vector<double>& precision, double &aupr);

	static int GetAupr(std::vector <std::pair<double, bool> >& predictScores, double &aupr);

	static int GetAuc(std::vector<std::pair<double, bool> >& predictScores, double &auc);

	static int SaveAupr(std::map<int, double>& aupr, std::string fileName);

	static int LoadAupr(std::map<int, double>& aupr, std::string fileName);

	static int GetMaxFscore(std::vector <std::pair<double, bool> >& predictScores, double& fscore, double& precision, double& recall);

	static int SaveMap(std::map<int, double>& mapPair, std::string fileName);

	static int LoadMap(std::map<int, double>& mapPair, std::string fileName);

	Evaluator();

	Evaluator(FILE* meshBin);

	Evaluator(const char* const meshBinFileName);

	Evaluator(const char* const meshBinFileName, const char* const graphFileName);

	~Evaluator();

	int LoadMesh(MeshRecordSet& meshSet);

	int LoadMesh(FILE* meshBin);

	int LoadMesh(const char* const fileName);

	int LoadGraph(FILE* graphFile);

	int LoadGraph(const char* const fileName);

	int MeshToInt(MultiLabelAnswer &mesh, std::set<int> &uid);

	int MeshToDuId(std::string &mesh, std::string &DuId);

	template<typename T>
	static int Evaluate(std::set<T>& predictLabel, std::set<T>& truthLabel, double& precision, double& recall, double &f1);

	int FlatEvaluate(MultiLabelAnswer &predict, MultiLabelAnswer &truth, double &precision, double& recall, double &f1);

	bool CheckValid(MultiLabelAnswerSet &predict, MultiLabelAnswerSet &truth);
	
	int LabelBasedMacroEvaluate(MultiLabelAnswerSet &predict, MultiLabelAnswerSet &truth, double &precision, double& recall, double &f1, std::string labelAnalyseFileName = "");

	int LabelBasedMicroEvaluate(MultiLabelAnswerSet &predict, MultiLabelAnswerSet &truth, double &precision, double& recall, double &f1);

	int ExampleBasedEvaluate(MultiLabelAnswerSet &predict, MultiLabelAnswerSet &truth, double &precision, double& recall, double &f1, std::string exampleAnalyseFileName = "");

	int HierarchicalEvaluate(MultiLabelAnswer &predict, MultiLabelAnswer &truth, double &precision, double& recall, double &f1, int runid);

	int HierarchicalEvaluate(MultiLabelAnswerSet &predict, MultiLabelAnswerSet &truth, double &precision, double& recall, double &f1);

	int NumlabelEvaluate(std::map<int, int>& predictNum, std::map<int, int>& goldNum, double& result, std::string numAnalyseFileName = "");
};


#endif /* _EVALUATOR_H */