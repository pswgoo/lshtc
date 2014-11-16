#ifndef DATA_STRUCT_H
#define DATA_STRUCT_H
#include "common_basic.h"
#include "file_utility.h"
#include "string_utility.h"

typedef std::map<int, double> Feature;

class Instance
{
public:
	Feature mFeatures;
	std::set<int> mLabels;

	Instance();

	~Instance();

	static int SaveInstances(const char* const fileName, std::vector<Instance> &instances);

	static int LoadInstances(const char* const fileName, std::vector<Instance> &instances);

	int Load(FileBuffer &buffer);

	int Save(FILE *outFile);

	int PrintText(FILE* outFile);
};

class FeatureSet
{
public:
	int mMaxIndex;
	std::vector<Feature> mFeatures;
public:
	FeatureSet();
	~FeatureSet();

	FeatureSet(int maxIndex);

	Feature& operator[](int idx);

	int Size() const;

	int AddInstance(Feature &feature);

	int Merge(FeatureSet& anotherFeatureSet);

	int Normalize();

	int UpdateMaxIndex();

	int SaveFeature(const std::string& fileName, std::vector<std::string>* vecInfo = NULL);

	int SaveSvmFormat(const std::vector<double>& labels, std::string fileName, std::vector<std::string>* vecInfo = NULL);

	static int Normalize(Feature& feature);

	static int SaveFeature(Feature& feature, const char* const fileName);

	static int LoadFeature(const char* const fileName, Feature& feature);
	
	static int SaveMatrix(FILE *OutFile, std::vector<Feature>& features);

	static int SaveMatrix(const char* const fileName, std::vector<Feature>& features);

};

class QueryTable
{
private:
	//查询表的表值，pair.first为表的关键字，pair.second为对应关键字的值
	std::vector<std::pair<double, double>> mQueryTable;

public:
	QueryTable();

	//用给定的表初始化
	QueryTable(const std::vector<std::pair<double, double>>& table);

	~QueryTable();

	//查询在两个点front,rear之间key对应的值，用线性插值
	int Interpolate(const std::pair<double, double>& front, const std::pair<double, double>& rear, double key, double& value);

	int BinarySearch(double key, int &index);

	//用给定的表初始化
	int Initialize(const std::vector<std::pair<double, double>>& table);

	//查询一个键对应的值
	int Query(const double key, double& value);

	//查询一个键对应的值
	double operator[](const double key);

	int Size();

	int GetElement(int index, std::pair<double, double>& ele);

	int Save(const std::string& fileName, int printLog = SILENT);

	int Save(FILE* outFile, int printLog = SILENT);

	int Load(const std::string& fileName, int printLog = SILENT);

	int Load(FILE* inFile, int printLog = SILENT);

	int Load(FileBuffer& buffer, int printLog = SILENT);

};

struct UniGram
{
	int mId;      // the same id with tokens id
	int mFirst;
	int mAppear; // count only once in the same citation
	UniGram(int id = 0, int first = 0, int appear = 0)
	{
		mId = id;
		mFirst = first;
		mAppear = appear;
	}
};

struct BiGram
{
	int mId;
	int mFirst;
	int mSecond;
	int mAppear;
	BiGram()
	{
		mId = 0;
		mFirst = 0;
		mSecond = 0;
		mAppear = 0;
	}
	BiGram(int id, int first, int second, int appear = 0)
	{
		mId = id;
		mFirst = first;
		mSecond = second;
		mAppear = appear;
	}
};

struct TrieNode
{
	int mId;
	int mDepth;
	char mPath;
	bool mDanger;
	size_t mLink[PRINTABLE_CHAR];
	size_t mFather;
	size_t mSuffix;
	TrieNode()
	{
		mId = mDepth = 0;
		mPath = 0;
		mDanger = false;
		mFather = mSuffix = 0;
		memset(mLink, 0, sizeof(mLink));
	}
};

struct TrieDetected
{
	int mFoundId;
	size_t mStart;
	size_t mEnd;
};

class TrieGraph
{
public:
	std::vector<TrieNode> mNodes;
public:
	TrieGraph();

	~TrieGraph();

	TrieGraph(std::map<std::string, int> &stringMap);

	TrieGraph(const char* const fileName);

	int Clear();

	int Load(FILE* binFile);

	int Load(const char* const fileName);

	int Save(FILE* binFile);

	int Save(const char* const fileName);

	int AddInTrieTree(std::string word,int id);

	int CompleteGraph();

	int Build(std::map<std::string, int> &stringMap);

	//pair<id, pair<posStart, posEnd> >
	int PhaseText(std::string &origText, std::vector<TrieDetected>& matches);
};

class EntryMap
{
public:
	std::map<std::string, int> mStringIndex;

	TrieGraph mTrie;

	bool mHasTrie; 

	bool mMode = WORD_MODE;

public:
	EntryMap();

	EntryMap(bool mode);

	~EntryMap();

	EntryMap(std::map<std::string, int>& entryMap);

	EntryMap(std::map<std::string, int>& entryMap, bool mode = WORD_MODE);

	int SetMode(bool mode);

	int Clear();

	int Load(FILE* inFile);

	int Load(std::string fileName);

	int Save(FILE* outFile);

	int Save(std::string fileName);

	int BuildFromMeshXML(FILE* inFile);

	int BuildFromMeshXML(std::string fileName);

	int BuildTrie();

	int PhaseText(std::string origText, std::map<int, double> &termCount);
};

//only store float value [0.0,1.0], accuracy is 1 / 65535;
class ShortFloat
{
private:
	typedef unsigned short FloatType;
	static const FloatType C_MAX_INTEGER = 65535;
	static std::vector<double> mTable;
	FloatType mValue;

public:
	static std::vector<double> InitTable();

	ShortFloat();
	ShortFloat(double value);

	double Value() const;

	ShortFloat operator=(const double& value);
};
#endif