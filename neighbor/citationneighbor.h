#ifndef CITATION_NEIGHBOR_H
#define CITATION_NEIGHBOR_H
#include "medline/citation.h"
#include <set>
#include <map>
#include <string>

class MeshNeighbor
{
public:
	int mMeshId;
	int mTotWeight;
	std::map<int, int> mNeighbors;

	static int SaveMeshNeighborBinary(std::map<int, MeshNeighbor>& gra, const char* fileName, int EdgeNum = -1);

	static int LoadMeshNeighborBinary(std::map<int, MeshNeighbor>& gra, const char* fileName);

	MeshNeighbor(int mId = 0);

	int AddNeighbor(int neiId);

	int AddEdge(int neiId, int weight);

	int GetNeighbor(std::vector<int>& meshs, int rank);
};

class CitationNeighbor
{
public:
	//static std::map<int,std::vector<std::string> > mCitationMeshs;
	static CitationSet mCitationSet;
	const static char mNeighborPath[];// = "/d:/bioasq1a/pmidlinks";
	const static char mUrlDownPath[];
	static std::map<int, int> mCntFindPmids;

	static int AddCitations(const char* const dir, int start, int end, std::string journalSetFile, CitationSet &tarSet = mCitationSet, std::set<int> delPmids = std::set<int>(), int printLog = SILENT);

	static int AddCitations(const char* const dir, int start, int end, std::string journalSetFile, std::set<int>& pmids, CitationSet &tarSet = mCitationSet, int printLog = SILENT);

	static int LoadCitations(const char* const fileName, CitationSet &tarSet = mCitationSet, int printLog = SILENT);

	static int LoadCitations(const char* const fileName, std::set<int>& pmids, CitationSet &tarSet = mCitationSet, int printLog = SILENT);

	static int LoadCitationMeshs(const char* const dir, int start, int end, std::set<int> testpmids = std::set<int>(), int printLog = SILENT);

	static int LoadCitationMeshs(CitationSet& citationSet, std::set<int> testpmids = std::set<int>(), int printLog = SILENT);

	static int DownloadLinkFiles(const std::set<int> &pmids, std::string linkPath = mNeighborPath);

	static int DownloadRowDataFiles(const std::set<int> &pmids, std::string urlPath = mUrlDownPath);

	static int StoreCntFindPmids(const char* const fileName);

	int mPmid;

	std::vector<std::pair<int, double> > mNeighborScores;

	std::map<int, Citation*> mNeighbors;

	CitationNeighbor(int pmid = 0);

	~CitationNeighbor();

	int GetNeighborNum(int& numNeighbor);

	int GetNeighborPmid(int& pmid, int rank);

	int GetNeighborScore(double& score, int rank);

	int GetValidNeighborCount(int topRank, int &count);

	int GetValidScoreCount(int topRank, double &sumScore);

	int GetAllNeighborPmid(std::vector<int>& pmids);

	int GetNeighborMesh(std::vector<std::string>& meshs, int rank);

	int GetNeighborMesh(std::map<int, std::vector<std::string> >& neighbors, int rank);

	int GetNeighborMesh(std::map<std::string, int>& meshCnt, int rank);

	int GetNeighborCitation(std::map<int, Citation*>& neighbors, int rank);

	int GetNeighborMeshWithScore(std::set<std::string>& meshs, double score);

	int GetNeighborMeshWithRank(std::set<std::string>& meshs, int rank);

	int GetNeighborPmidWithScore(std::vector<int>& pmids, double score);

	int GetNeighborPmidWithRank(std::vector<int>& pmids, int rank);

	int LoadNeighbor(std::string linkPath = mNeighborPath);

	int LoadRowNeighbor(const char* const fileName);

	int RemoveEmptyNeigbbor();
};

class CitationNeighborSet
{
public:
	std::map<int, CitationNeighbor> mNeighborSet;

	CitationNeighborSet();

	int Load(const char* const linkPath, const std::vector<int>& vecPmids);

	int Load(const char* const linkPath, const char* const testSetPath);

	int Load(const char* const linkPath, const char* const citationSetPath, const std::vector<int>& vecPmids);

	int Load(const char* const linkPath, const CitationSet& citationSet, const std::vector<int>& vecPmids);

	int GetNeighbor(int queryPmid, int queryRank, double& score, std::vector<std::string>& meshs);

	int GetNeighborNum(int queryPmid, int& numNeighbor);

	CitationNeighbor* operator[](int index);
};

int NextBracket(FILE *pFile, const char left, const char right, std::string& seq);

int NextBracketRange(const std::string& str, const int startPos, const int endPos, int& leftPos, int& rightPos, const char left = '{', const char right = '}', std::string* pStr = NULL);

int NextQuotePair(const std::string& bracket, int start, int end, std::string& tar);
#endif
