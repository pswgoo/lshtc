#ifndef MEDLINE_TOOL_CLASS_H
#define MEDLINE_TOOL_CLASS_H

#include "medline/basic.h"
#include "medline/mesh.h"
#include "medline/citation.h"
#include "classifier/classifier.h"
#include "extractfeature/feature.h"

class MetalabelFeature
{
public:
	int ExtractFeature(const std::vector<TokenCitation*>& tokenVector, CitationSet& citationSet, UniGramFeature& uniGrams, BiGramFeature& biGrams, JournalSet& journalSet, feature_node** &featureSpace, int printLog = SILENT);

	int ExtractFeature(const std::vector<TokenCitation*>& tokenVector, CitationSet& citationSet, UniGramFeature& uniGrams, BiGramFeature& biGrams, JournalSet& journalSet, FeatureSet& allFeatures, int printLog = SILENT);

	int ExtractFeature(const std::vector<TokenCitation*>& tokenVector, UniGramFeature& uniGrams, BiGramFeature& biGrams, feature_node** &featureSpace, int printLog = SILENT);

	int ExtractFeature(const std::vector<TokenCitation*>& tokenVector, UniGramFeature& uniGrams, BiGramFeature& biGrams, FeatureSet& allFeatures, int printLog = SILENT);

	int ExtractFeature(const std::vector<TokenCitation*>& tokenVector, UniGramFeature& uniGrams, feature_node** &featureSpace, int printLog = SILENT);

	int ExtractFeature(const std::vector<TokenCitation*>& tokenVector, UniGramFeature& uniGrams, FeatureSet& allFeatures, int printLog = SILENT);

};

class MeshInfoSet
{
private:
	typedef std::pair<int, int> MeshPair;											//规定 MeshPair.first < MeshPair.second

	int mTotalCitationNum;															//数据集中的citation总数
	int mUniqueMeshNum;																//数据集中出现的不同的mesh总数
	int mUniqueMeshPairNum;															//数据集中出现的不同的meshPair总数
	int mSumMeshOcurNum;															//数据集中所有citation的mesh的个数的和

	std::map<int, int> mMeshOcurNum;												//数据集中，每个mesh出现的次数
	std::map<MeshPair, int> mMeshPairOcurNum;										//数据集中，每个mesh pair出现的次数，规定 pair.first < pair.second

	std::map<int, std::vector<std::pair<int, int>>> mMeshNeiboNum;					//数据集中，记录每个mesh和与该mesh共同出现的meshId和共同出现次数，按照共同出现次数从高到低排序

	std::map<int, int> mTitleMeshEntryOcurNum;										//数据集中，每个mesh entry在title中出现的次数
	std::map<MeshPair, int> mTitleMeshEntryPairOcurNum;								//数据集中，mesh entry pair在title中出现的次数，规定 pair.first < pair.second

	std::map<int, int> mAbstractMeshEntryOcurNum;									//数据集中，每个mesh entry在abstract中出现的次数
	std::map<MeshPair, int> mAbstractMeshEntryPairOcurNum;							//数据集中，mesh entry pair在abstract中出现的次数，规定 pair.first < pair.second

	//std::map<int, std::vector<int>> mMeshPmids;										//(待定)记录数据集中，goldstandard包含该mesh的pmid集

private:
	int AddMeshNum(int meshId, std::map<int, int>& meshCnt);

	int AddMeshPairNum(int meshId1, int meshId2, std::map<MeshPair, int>& meshPairCnt);

	int AddMeshNeiboNum(int meshId, int meshNeiboId, int occurNum, std::map<int, std::vector<std::pair<int, int>>>& meshNeiboNum);

	int GetPairNum(int meshId1, int meshId2, std::map<MeshPair, int>& meshPairCnt, int& num);

	int InitializeMeshNeiboNum(const std::map<MeshPair, int>& meshPair, std::map<int, std::vector<std::pair<int, int>>>& meshNeiboNum);

	int InitializeMesh(CitationSet& citationSet, MeshRecordSet& meshRecords, int printLog = SILENT);										//只初始化mesh本身在数据集中，与entrymap无关的信息

	int InitializeMeshEntry(CitationSet& citationSet, MeshRecordSet& meshRecords, EntryMapFeature& entryMap, int printLog = SILENT);		//只初始化mesh entry在数据集中的相关信息

public:
	MeshInfoSet();
	MeshInfoSet(CitationSet& citationSet, MeshRecordSet& meshRecords, EntryMapFeature& entryMap);
	~MeshInfoSet();

	int Initialize(const std::string& citationFileName, const std::string& meshRecordFileName, const std::string& entryMapFileName, int printLog = SILENT);

	int Initialize(CitationSet& citationSet, MeshRecordSet& meshRecords, EntryMapFeature& entryMap, int printLog = SILENT);

	int QueryTotalCitationNum(int& totalCitationNum);													//查询数据集citation总数

	int QueryMeshOcurProb(int keyMesh, double& prob);													//查询meshId在数据集中出现的概率

	int QueryMeshPairOcurProb(int mesh1, int mesh2, double& prob);										//查询mesh pair(mesh1,mesh2)同时出现的概率

	int QueryTitleEntryOcurProb(int keyMesh, double& prob);												//查询meshId的entry在title中出现的概率

	int QueryAbstractEntryOcurProb(int keyMesh, double& prob);											//查询meshId的entry在abstract中出现的概率

	int QueryMeshNeiboOcurProb(int keyMesh, int neighborMesh, double& prob);							//查询meshId1出现时，meshId2同时出现在文章中的概率

	int QueryTitleEntryNeiboOcurProb(int keyMesh, int neighborMesh, double& prob);						//查询meshId1的entry出现在title中时，meshId2的entry同时出现的概率

	int QueryAbstractEntryNeiboOcurProb(int keyMesh, int neighborMesh, double& prob);					//查询meshId1的entry出现在abstract中时，meshId2的entry同时出现的概率

	int QueryMeshTopNeibo(int meshId, int rank, std::vector<int>& neiboIds);							//查询与meshId共同出现次数最多的mesh

	int Clear();

	int Save(const std::string fileName, int printLog = SILENT);

	int Save(FILE* outFile, int printLog = SILENT);

	int Load(const std::string fileName, int printLog = SILENT);

	int Load(FILE* inFile, int printLog = SILENT);

	int Load(FileBuffer& buffer, int printLog = SILENT);
};

#endif