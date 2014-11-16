#ifndef _CITATION_H
#define _CITATION_H
#include "basic.h"
#include <cstdio>
#include <string>
#include <vector>
#include <map>

struct Author
{
	char* mLastName;
	char* mForeName;
	char* mInitials;
	char* mSuffix;
	char* mCollectiveName;
};

struct MeshTerm
{
	bool mIsMajor;
	char* mText;
};

struct Mesh
{
	MeshTerm mDescriptorName;
	int mNumQualifierName;
	MeshTerm* mQualifierName;
};

struct Abstract
{
	char* mLabel;
	char* mText;
};

struct CommentsCorrections
{
	int mPmid;
	char* mRefSource;
	char* mRefType;
};

class Citation
{
public:
	int mPmid;
	int mDateCreated; //	yyyymmdd
	int mPubYear;
	bool mIsDeleted;  //	true for deleted
	char* mJournalTitle;
	char* mArticleTitle;
	int mNumberAbstract;
	struct Abstract* mAbstract;
	int mNumberAuthor;
	struct Author* mAuthorList;
	int mNumberMesh;
	struct Mesh* mMeshHeadingList;
	int mNumberComment;
	struct CommentsCorrections* mCommentsCorrectionsList;

public:
	Citation(int pmid = 0);
	~Citation();

	static int SkipCitation(FileBuffer& buffer, bool bPmid); //when "bPmid" is true,skip mPmid;otherwise not

	int DestroyCitation();
	
	int Save(FILE* binFile, int printLog = SILENT) const;
	
	int Load(FILE* binFile, int pmid = 0, int printLog = SILENT);
	
	int PrintText(FILE* binFile);
	
	bool FilterPmid(const int mod, const std::vector<int> &pmidList) const;
	
	bool FilterDate(const int startDate, const int endDate) const;
	
	bool FilterJournalTitle(const int mod, const std::vector<std::string> &wordList) const;
	
	bool FilterArticleTitle(const int mod, const std::vector<std::string> &wordList) const;
	
	bool FilterAbstract(const int mod, const std::vector<std::string> &wordList) const;
	
	bool FilterDescriptorMesh(const int mod, const std::vector<std::string> &wordList) const;
	
	bool FilterQualifierMesh(const int mod, const std::vector<std::string> &wordList) const;
	
	int GetMeshVector(std::vector<std::string> &meshVector) const;

	int GetMajorMeshVector(std::vector<std::string> &majorMeshVector) const;
	
	int GetAllAbstract(std::string &allAbstract) const;

	int SaveTrainJson(FILE* outFile) const;

	int SaveTestJson(FILE* outFile) const;

	int SaveTestJson2(FILE* outFile) const;

	int SaveTrainJson2(FILE* outFile) const;
};

class CitationSet
{
public:
	std::map<int, Citation*> mCitations;

	CitationSet();
	~CitationSet();
	
	static bool CmpDateCreated(const Citation* p1,const Citation* p2);

	Citation* operator[](int index);

	int Size();

	int Save(FILE* binFile, int printLog = SILENT);
	
	int Save(const char* fileName, int printLog = SILENT);
	
	int Load(FILE* binFile, int printLog = SILENT);
	
	int Load(const char* fileName, int printLog = SILENT);

	int Load(const char* fileName, std::set<int> &pmids, int printLog = SILENT);

	int Load(FILE* binFile, std::set<int> &pmidSet, int printLog = SILENT);

	int PrintText(FILE* outFile);

	int PrintText(const char* const filename);

	int SaveTrainJson(const char* const fileName);

	int SaveTrainJson(FILE* outFile);

	int SaveTestJson(const char* const fileName);

	int SaveTestJson(FILE* outFile);

	int LoadTestJson(const std::string& fileName, int printLog = SILENT);

	int LoadTestJson(FILE* inFile, int printLog = SILENT);

	int AddJournalTitle(const CitationSet& citationSet, int printLog = SILENT);

	int SortByDateCreated(std::vector<Citation*>& vecCita);

};

class Journal
{
public:
	int mJournalId;
	std::vector<char*>  mMedAbbr;
	std::vector<char*> mIsoAbbr;
	std::vector<char*> mJournalTitle;

	Journal(int id = 0);
	~Journal();

	int Initialize(int id, const std::string& medAbbr, const std::string& isoAbbr, const std::string& journalTitle);

	int InitializeJournal(int id, const std::string& journalTitle, const std::string& isoAbbr, const std::string& medAbbr);

	int addJournalTitle(const std::string& journalTitle);

	int addIsoAbbr(const std::string& isoAbbr);

	int addMedAbbr(const std::string& medAbbr);

	int PrintText(FILE* outFile);

	int LoadMedAbbr(FILE* inFile);

	int SaveMedAbbr(FILE* outFile);

	int LoadJournalTitle(FILE* inFile);

	int SaveJournalTitle(FILE* outFile);
};

class JournalSet
{
private:
public:
	std::map<std::string, Journal*> mMedAbbr;
	std::map<std::string, Journal*> mJournalTitle;
	std::map<std::string, Journal*> mIsoAbbr;
	std::map<int, Journal> mJournals;

	JournalSet();
	~JournalSet();

	static int SaveJournalMeshFreq(const std::map<int, std::map<int, double>>& freq, const std::string& fileName);

	static int LoadJournalMeshFreq(std::map<int, std::map<int, double>>& freq, const std::string& fileName);

	int Clear();

	int LoadRowData(const std::string medlineJournal, const std::string testJournal, int printLog = SILENT);

	int LoadRowData(const std::string medlineJournal, int printLog = SILENT);

	int Load(FILE* inFile, int printLog = SILENT);

	int Load(const std::string fileName, int printLog = SILENT);

	int Save(FILE* outFile, int printLog = SILENT);

	int Save(const std::string fileName, int printLog = SILENT);

	int LoadJournalSet(FILE* inFile, int printLog = SILENT);

	int LoadJournalSet(const std::string fileName, int printLog = SILENT);

	int SaveJournalSet(FILE* outFile, int printLog = SILENT);

	int SaveJournalSet(const std::string fileName, int printLog = SILENT);

	Journal* SearchMedAbbr(const std::string medAbbr);

	Journal* SearchIsoAbbr(const std::string isoAbbr);

	Journal* SearchJournalTitle(const std::string journalTitle);

	std::string GetFirstJournalTitle(const std::string& medAbbr);

	Journal* operator[](int index);

	Journal* operator[](const std::string& journalTitle);

	int PrintText(const std::string fileName);
};

class JournalLabelNum
{
private:
	//every journal vector begin with an empty element (-1,0)
	std::map<int, std::vector<std::pair<int, std::pair<int, int>>>> mJournalTotalLabelNum;
public:
	JournalLabelNum();
	~JournalLabelNum();

	int Load(const std::string& binaryFileName, int printLog = SILENT);

	int Save(const std::string& binaryFileName, int printLog = SILENT);

	int Initialize(const std::string& citationFileName, const std::string& journalSetFileName, int printLog = SILENT);

	//查询dateCreated当年，该杂志平均的label个数
	int GetAverageLabelNum(int journalId, int dateCreated, double& average);

	//查询journal离dateCreated最近的几篇文章的平均label个数
	int GetAverageLabelNum(int journalId, int dateCreated, int latestCitationNum, double& average);

	//查询杂志所有文章的平均label个数
	int GetAverageLabelNum(int journalId, double& average);

	//查询dateCreated当年，该杂志所有citation的label个数标准差
	int GetStandardDeviation(int journalId, int dateCreated, double& stdDev);

	//查询journal离dateCreated最近的几篇文章的label个数标准差
	int GetStandardDeviation(int journalId, int dateCreated, int latestCitationNum, double& stdDev);

	//查询杂志所有文章的label个数标准差
	int GetStandardDeviation(int journalId, double& stdDev);

	int GetLatestPmid(int journalId, int dateCreated, int latestCitationNum, std::vector<int>& latestPmids);

	//计算journal创建时间区间在 [startTime,endTime] 的文章的label个数的标准差
	int GetStdDevByTime(int journalId, int startTime, int endTime, double& stdDev);

	//计算journal下标区间在 [startIndex,endIndex] 的文章的label个数的标准差
	int GetStdDevByIndex(int journalId, int startIndex, int endIndex, double& stdDev);

	//查询杂志创建时间在dateCreated（包括dateCreated本身）之前的文章总数，和这些文章的label个数的和
	int SearchByTime(int journalId, int dateCreated, int& totalLabelNum, int& index);
};

struct TrainCitation
{
	int mPmid;
	int year;
	std::string mAbstract;
	std::string mJournal;
	std::string mArticle;
	std::vector<std::string> mMeshs;
};

struct TestCitation
{
	int mPmid;
	std::string mArticle;
	std::string mAbstract;
};

class TrainCitationSet
{
public:
	std::vector<TrainCitation> mTrainSet;

	int LoadTrainSet(const char* const fileName);

	int LoadTrainSet(FILE *pFile);
};

class TestCitationSet
{
public:
	std::vector<TestCitation> mTestSet;

	int LoadTestSet(const char* const fileName);

	int LoadTestSet(FILE *pFile);
};

#endif /* _CITATION_H */
