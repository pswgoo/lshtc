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
	typedef std::pair<int, int> MeshPair;											//�涨 MeshPair.first < MeshPair.second

	int mTotalCitationNum;															//���ݼ��е�citation����
	int mUniqueMeshNum;																//���ݼ��г��ֵĲ�ͬ��mesh����
	int mUniqueMeshPairNum;															//���ݼ��г��ֵĲ�ͬ��meshPair����
	int mSumMeshOcurNum;															//���ݼ�������citation��mesh�ĸ����ĺ�

	std::map<int, int> mMeshOcurNum;												//���ݼ��У�ÿ��mesh���ֵĴ���
	std::map<MeshPair, int> mMeshPairOcurNum;										//���ݼ��У�ÿ��mesh pair���ֵĴ������涨 pair.first < pair.second

	std::map<int, std::vector<std::pair<int, int>>> mMeshNeiboNum;					//���ݼ��У���¼ÿ��mesh�����mesh��ͬ���ֵ�meshId�͹�ͬ���ִ��������չ�ͬ���ִ����Ӹߵ�������

	std::map<int, int> mTitleMeshEntryOcurNum;										//���ݼ��У�ÿ��mesh entry��title�г��ֵĴ���
	std::map<MeshPair, int> mTitleMeshEntryPairOcurNum;								//���ݼ��У�mesh entry pair��title�г��ֵĴ������涨 pair.first < pair.second

	std::map<int, int> mAbstractMeshEntryOcurNum;									//���ݼ��У�ÿ��mesh entry��abstract�г��ֵĴ���
	std::map<MeshPair, int> mAbstractMeshEntryPairOcurNum;							//���ݼ��У�mesh entry pair��abstract�г��ֵĴ������涨 pair.first < pair.second

	//std::map<int, std::vector<int>> mMeshPmids;										//(����)��¼���ݼ��У�goldstandard������mesh��pmid��

private:
	int AddMeshNum(int meshId, std::map<int, int>& meshCnt);

	int AddMeshPairNum(int meshId1, int meshId2, std::map<MeshPair, int>& meshPairCnt);

	int AddMeshNeiboNum(int meshId, int meshNeiboId, int occurNum, std::map<int, std::vector<std::pair<int, int>>>& meshNeiboNum);

	int GetPairNum(int meshId1, int meshId2, std::map<MeshPair, int>& meshPairCnt, int& num);

	int InitializeMeshNeiboNum(const std::map<MeshPair, int>& meshPair, std::map<int, std::vector<std::pair<int, int>>>& meshNeiboNum);

	int InitializeMesh(CitationSet& citationSet, MeshRecordSet& meshRecords, int printLog = SILENT);										//ֻ��ʼ��mesh���������ݼ��У���entrymap�޹ص���Ϣ

	int InitializeMeshEntry(CitationSet& citationSet, MeshRecordSet& meshRecords, EntryMapFeature& entryMap, int printLog = SILENT);		//ֻ��ʼ��mesh entry�����ݼ��е������Ϣ

public:
	MeshInfoSet();
	MeshInfoSet(CitationSet& citationSet, MeshRecordSet& meshRecords, EntryMapFeature& entryMap);
	~MeshInfoSet();

	int Initialize(const std::string& citationFileName, const std::string& meshRecordFileName, const std::string& entryMapFileName, int printLog = SILENT);

	int Initialize(CitationSet& citationSet, MeshRecordSet& meshRecords, EntryMapFeature& entryMap, int printLog = SILENT);

	int QueryTotalCitationNum(int& totalCitationNum);													//��ѯ���ݼ�citation����

	int QueryMeshOcurProb(int keyMesh, double& prob);													//��ѯmeshId�����ݼ��г��ֵĸ���

	int QueryMeshPairOcurProb(int mesh1, int mesh2, double& prob);										//��ѯmesh pair(mesh1,mesh2)ͬʱ���ֵĸ���

	int QueryTitleEntryOcurProb(int keyMesh, double& prob);												//��ѯmeshId��entry��title�г��ֵĸ���

	int QueryAbstractEntryOcurProb(int keyMesh, double& prob);											//��ѯmeshId��entry��abstract�г��ֵĸ���

	int QueryMeshNeiboOcurProb(int keyMesh, int neighborMesh, double& prob);							//��ѯmeshId1����ʱ��meshId2ͬʱ�����������еĸ���

	int QueryTitleEntryNeiboOcurProb(int keyMesh, int neighborMesh, double& prob);						//��ѯmeshId1��entry������title��ʱ��meshId2��entryͬʱ���ֵĸ���

	int QueryAbstractEntryNeiboOcurProb(int keyMesh, int neighborMesh, double& prob);					//��ѯmeshId1��entry������abstract��ʱ��meshId2��entryͬʱ���ֵĸ���

	int QueryMeshTopNeibo(int meshId, int rank, std::vector<int>& neiboIds);							//��ѯ��meshId��ͬ���ִ�������mesh

	int Clear();

	int Save(const std::string fileName, int printLog = SILENT);

	int Save(FILE* outFile, int printLog = SILENT);

	int Load(const std::string fileName, int printLog = SILENT);

	int Load(FILE* inFile, int printLog = SILENT);

	int Load(FileBuffer& buffer, int printLog = SILENT);
};

#endif