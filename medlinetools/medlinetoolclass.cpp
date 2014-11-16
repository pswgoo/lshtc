#include "medlinetoolclass.h"
#include "medlinetoolfunction.h"
#include "common/file_utility.h"
using namespace std;

int MetalabelFeature::ExtractFeature(const vector<TokenCitation*>& tokenVector, CitationSet& citationSet, UniGramFeature& uniGrams, BiGramFeature& biGrams, JournalSet& journalSet, feature_node** &featureSpace, int printLog)
{
	int numThreads = omp_get_num_procs();
	if (printLog != SILENT)
		clog << "CPU number: " << numThreads << endl;
	omp_set_num_threads(numThreads);
	if (printLog != SILENT)
		clog << "Extract unigram & bigram" << endl;

	if (printLog != SILENT)
		clog << "Make Feature table" << endl;
	int featureNum = (int)tokenVector.size();
	featureSpace = NULL;
	featureSpace = Malloc(feature_node*, featureNum);
	memset(featureSpace, 0, sizeof(feature_node*)* featureNum);

	int uniMaxIndex = uniGrams.mDictionary.rbegin()->first + 1;
	int biMaxIndex = biGrams.mDictionary.rbegin()->first + 1;
	int jourMaxIndex = journalSet.mJournals.rbegin()->first + 1;

	if (printLog != SILENT)
		clog << "Extract features parallel" << endl;

#pragma omp parallel for schedule(dynamic)
	for (int i = 0; i < (int)tokenVector.size(); i++)
	{
		FeatureSet tabAllFeatures;
		tabAllFeatures.mMaxIndex = uniMaxIndex;
		tabAllFeatures.mFeatures.resize(1);

		FeatureSet tabBiFeatures;
		tabBiFeatures.mMaxIndex = biMaxIndex;
		tabBiFeatures.mFeatures.resize(1);

		uniGrams.Extract(*tokenVector[i], tabAllFeatures.mFeatures[0]);
		biGrams.Extract(*tokenVector[i], tabBiFeatures.mFeatures[0]);

		FeatureSet tabJourFeatures;
		tabJourFeatures.mMaxIndex = jourMaxIndex;
		tabJourFeatures.mFeatures.resize(1);
		if (citationSet[tokenVector[i]->mPmid]->mJournalTitle != NULL)
		{
			Journal* ptrJournal = journalSet.SearchJournalTitle(citationSet[tokenVector[i]->mPmid]->mJournalTitle);
			if (ptrJournal != NULL)
			{
				tabJourFeatures.mFeatures[0][ptrJournal->mJournalId] = 1.0;
			}
			else
				cerr << "Error: \"" << citationSet[tokenVector[i]->mPmid]->mJournalTitle << "\" can't find journal in journal set in pmid " << tokenVector[i]->mPmid << endl;
		}
		else
			cerr << "Error: " << tokenVector[i]->mPmid << " can't find journal title in citation" << endl;

		tabAllFeatures.Merge(tabBiFeatures);
		tabAllFeatures.Merge(tabJourFeatures);
		tabAllFeatures.Normalize();

		featureSpace[i] = NULL;
		LinearMachine::TransFeatures(featureSpace[i], tabAllFeatures.mFeatures[0]);
	}
	return 0;
}

int MetalabelFeature::ExtractFeature(const vector<TokenCitation*>& tokenVector, CitationSet& citationSet, UniGramFeature& uniGrams, BiGramFeature& biGrams, JournalSet& journalSet, FeatureSet& allFeatures, int printLog)
{
	int rtn = 0;
	allFeatures.mFeatures.clear();
	allFeatures.mMaxIndex = uniGrams.mDictionary.rbegin()->first + 1;
	allFeatures.mFeatures.resize(tokenVector.size());

	FeatureSet biFeatures;
	biFeatures.mMaxIndex = biGrams.mDictionary.rbegin()->first + 1;
	biFeatures.mFeatures.resize(tokenVector.size());

	FeatureSet jourFeatures;
	jourFeatures.mMaxIndex = journalSet.mJournals.rbegin()->first + 1;
	jourFeatures.mFeatures.resize(tokenVector.size());

	int numThreads = omp_get_num_procs();
	if (printLog != SILENT)
		clog << "CPU number: " << numThreads << endl;

	omp_set_num_threads(numThreads);
	if (printLog != SILENT)
		clog << "Start Parallel Extract Features" << endl;
#pragma omp parallel for schedule(dynamic) 
	for (int i = 0; i < tokenVector.size(); i++)
	{
		uniGrams.Extract(*tokenVector[i], allFeatures.mFeatures[i]);
	}

#pragma omp parallel for schedule(dynamic) 
	for (int i = 0; i < tokenVector.size(); i++)
	{
		biGrams.Extract(*tokenVector[i], biFeatures.mFeatures[i]);
	}

#pragma omp parallel for schedule(dynamic)
	for (int i = 0; i < tokenVector.size(); ++i)
	{
		Journal* ptrJournal = NULL;
		ptrJournal = journalSet.SearchJournalTitle(citationSet[tokenVector[i]->mPmid]->mJournalTitle);
		if (ptrJournal != NULL)
		{
			jourFeatures.mFeatures[i][ptrJournal->mJournalId] = 1.0;
		}
		else
		{
			cerr << "Error: can't find \"" << citationSet[tokenVector[i]->mPmid]->mJournalTitle << " in pmid " << tokenVector[i]->mPmid << endl;
		}
	}

	rtn = allFeatures.Merge(biFeatures);
	CHECK_RTN(rtn);
	rtn = allFeatures.Merge(jourFeatures);
	CHECK_RTN(rtn);
	rtn = allFeatures.Normalize();
	CHECK_RTN(rtn);
	return 0;
}

int MetalabelFeature::ExtractFeature(const vector<TokenCitation*>& tokenVector, UniGramFeature& uniGrams, BiGramFeature& biGrams, feature_node** &featureSpace, int printLog)
{
	int numThreads = omp_get_num_procs();
	if (printLog != SILENT)
		clog << "CPU number: " << numThreads << endl;
	omp_set_num_threads(numThreads);
	if (printLog != SILENT)
		clog << "Extract unigram & bigram" << endl;

	if (printLog != SILENT)
		clog << "Make Feature table" << endl;
	int featureNum = (int)tokenVector.size();
	featureSpace = NULL;
	featureSpace = Malloc(feature_node*, featureNum);
	memset(featureSpace, 0, sizeof(feature_node*)* featureNum);

	int uniMaxIndex = uniGrams.mDictionary.rbegin()->first + 1;
	int biMaxIndex = biGrams.mDictionary.rbegin()->first + 1;

	if (printLog != SILENT)
		clog << "Extract features parallel" << endl;

#pragma omp parallel for schedule(dynamic)
	for (int i = 0; i < (int)tokenVector.size(); i++)
	{
		FeatureSet tabAllFeatures;
		tabAllFeatures.mMaxIndex = uniMaxIndex;
		tabAllFeatures.mFeatures.resize(1);

		FeatureSet tabBiFeatures;
		tabBiFeatures.mMaxIndex = biMaxIndex;
		tabBiFeatures.mFeatures.resize(1);

		uniGrams.Extract(*tokenVector[i], tabAllFeatures.mFeatures[0]);
		biGrams.Extract(*tokenVector[i], tabBiFeatures.mFeatures[0]);

		tabAllFeatures.Merge(tabBiFeatures);
		tabAllFeatures.Normalize();

		featureSpace[i] = NULL;
		LinearMachine::TransFeatures(featureSpace[i], tabAllFeatures.mFeatures[0]);
	}
	return 0;
}

int MetalabelFeature::ExtractFeature(const vector<TokenCitation*>& tokenVector, UniGramFeature& uniGrams, BiGramFeature& biGrams, FeatureSet& allFeatures, int printLog)
{
	int rtn = 0;
	allFeatures.mFeatures.clear();
	allFeatures.mMaxIndex = uniGrams.mDictionary.rbegin()->first + 1;
	allFeatures.mFeatures.resize(tokenVector.size());

	FeatureSet biFeatures;
	biFeatures.mMaxIndex = biGrams.mDictionary.rbegin()->first + 1;
	biFeatures.mFeatures.resize(tokenVector.size());

	int numThreads = omp_get_num_procs();
	if (printLog != SILENT)
		clog << "CPU number: " << numThreads << endl;

	omp_set_num_threads(numThreads);
	if (printLog != SILENT)
		clog << "Start Parallel Extract Features" << endl;
#pragma omp parallel for schedule(dynamic) 
	for (int i = 0; i < tokenVector.size(); i++)
	{
		uniGrams.Extract(*tokenVector[i], allFeatures.mFeatures[i]);
	}

#pragma omp parallel for schedule(dynamic) 
	for (int i = 0; i < tokenVector.size(); i++)
	{
		biGrams.Extract(*tokenVector[i], biFeatures.mFeatures[i]);
	}

	allFeatures.Merge(biFeatures);
	rtn = allFeatures.Normalize();
	CHECK_RTN(rtn);

	return 0;
}

int MetalabelFeature::ExtractFeature(const std::vector<TokenCitation*>& tokenVector, UniGramFeature& uniGrams, feature_node** &featureSpace, int printLog)
{
	int numThreads = omp_get_num_procs();
	if (printLog != SILENT)
		clog << "CPU number: " << numThreads << endl;
	omp_set_num_threads(numThreads);
	if (printLog != SILENT)
		clog << "Extract unigram" << endl;

	if (printLog != SILENT)
		clog << "Make Feature table" << endl;
	int featureNum = (int)tokenVector.size();
	featureSpace = NULL;
	featureSpace = Malloc(feature_node*, featureNum);
	memset(featureSpace, 0, sizeof(feature_node*)* featureNum);

	int uniMaxIndex = uniGrams.mDictionary.rbegin()->first + 1;

	if (printLog != SILENT)
		clog << "Extract features parallel" << endl;

#pragma omp parallel for schedule(dynamic)
	for (int i = 0; i < (int)tokenVector.size(); i++)
	{
		FeatureSet tabAllFeatures;
		tabAllFeatures.mMaxIndex = uniMaxIndex;
		tabAllFeatures.mFeatures.resize(1);

		uniGrams.Extract(*tokenVector[i], tabAllFeatures.mFeatures[0]);
		tabAllFeatures.Normalize();

		featureSpace[i] = NULL;
		LinearMachine::TransFeatures(featureSpace[i], tabAllFeatures.mFeatures[0]);
	}
	return 0;
}

int MetalabelFeature::ExtractFeature(const std::vector<TokenCitation*>& tokenVector, UniGramFeature& uniGrams, FeatureSet& allFeatures, int printLog)
{
	int rtn = 0;
	allFeatures.mFeatures.clear();
	allFeatures.mMaxIndex = uniGrams.mDictionary.rbegin()->first + 1;
	allFeatures.mFeatures.resize(tokenVector.size());

	int numThreads = omp_get_num_procs();
	if (printLog != SILENT)
		clog << "CPU number: " << numThreads << endl;

	omp_set_num_threads(numThreads);
	if (printLog != SILENT)
		clog << "Start Parallel Extract Features" << endl;
#pragma omp parallel for schedule(dynamic) 
	for (int i = 0; i < tokenVector.size(); i++)
	{
		uniGrams.Extract(*tokenVector[i], allFeatures.mFeatures[i]);
	}
	rtn = allFeatures.Normalize();
	CHECK_RTN(rtn);

	return 0;
}

MeshInfoSet::MeshInfoSet()
{
	Clear();
}

MeshInfoSet::MeshInfoSet(CitationSet& citationSet, MeshRecordSet& meshRecords, EntryMapFeature& entryMap)
{
	Clear();
	Initialize(citationSet, meshRecords, entryMap);
}

MeshInfoSet::~MeshInfoSet()
{
	Clear();
}

int MeshInfoSet::AddMeshNum(int meshId, std::map<int, int>& meshCnt)
{
	if (meshCnt.count(meshId) == 0)
		meshCnt[meshId] = 0;
	++meshCnt[meshId];
	return 0;
}

int MeshInfoSet::AddMeshPairNum(int meshId1, int meshId2, std::map<MeshPair, int>& meshPairCnt)
{
	if (meshId1 == meshId2)
		return -1;
	if (meshId1 > meshId2)
	{
		int tmp = meshId1;
		meshId1 = meshId2;
		meshId2 = tmp;
	}
	MeshPair meshPair = make_pair(meshId1, meshId2);
	if (meshPairCnt.count(meshPair) == 0)
		meshPairCnt[meshPair] = 0;
	++meshPairCnt[meshPair];
	return 0;
}

int MeshInfoSet::AddMeshNeiboNum(int meshId, int meshNeiboId, int occurNum, map<int, vector<pair<int, int>>>& meshNeiboNum)
{
	if (meshId == meshNeiboId)
		return 0;
	if (meshNeiboNum.count(meshId) == 0)
		meshNeiboNum[meshId].clear();
	meshNeiboNum[meshId].push_back(make_pair(meshNeiboId, occurNum));
	return 0;
}

int MeshInfoSet::GetPairNum(int meshId1, int meshId2, std::map<MeshPair, int>& meshPairCnt, int& num)
{
	num = 0;
	if (meshId1 > meshId2)
	{
		int tmp = meshId1;
		meshId1 = meshId2;
		meshId2 = tmp;
	}
	MeshPair meshPair = make_pair(meshId1, meshId2);
	if (meshPairCnt.count(meshPair) > 0)
		num = meshPairCnt[meshPair];

	return 0;
}

int MeshInfoSet::InitializeMeshNeiboNum(const std::map<MeshPair, int>& meshPair, std::map<int, std::vector<std::pair<int, int>>>& meshNeiboNum)
{
	int rtn = 0;
	meshNeiboNum.clear();
	for (map<MeshPair, int>::const_iterator it = meshPair.cbegin(); it != meshPair.cend(); ++it)
	{
		rtn = AddMeshNeiboNum(it->first.first, it->first.second, it->second, meshNeiboNum);
		CHECK_RTN(rtn);
		rtn = AddMeshNeiboNum(it->first.second, it->first.first, it->second, meshNeiboNum);
		CHECK_RTN(rtn);
	}
	for (map<int, vector<pair<int, int>>>::iterator it = meshNeiboNum.begin(); it != meshNeiboNum.end(); ++it)
		sort(it->second.begin(), it->second.end(), CmpPairByLagerSecond<int, int>);
	return 0;
}

int MeshInfoSet::InitializeMesh(CitationSet& citationSet, MeshRecordSet& meshRecords, int printLog)
{
	int rtn = 0;
	mSumMeshOcurNum = 0;
	mUniqueMeshNum = 0;
	mUniqueMeshPairNum = 0;
	mTotalCitationNum = citationSet.Size();
	mMeshOcurNum.clear();
	mMeshPairOcurNum.clear();

	if (printLog != SILENT)
		clog << "InitializeMesh: Counting unique mesh and unique mesh pair in citationset" << endl;
	int cnt = 0;
	for (map<int, Citation*>::iterator it = citationSet.mCitations.begin(); it != citationSet.mCitations.end(); ++it)
	{
		++cnt;
		if (printLog != SILENT && (cnt & ((1 << 18) - 1)) == 0)
			clog << "\r" << cnt << " citation counting";
		mSumMeshOcurNum += it->second->mNumberMesh;
		vector<int> meshIds;
		meshIds.reserve(it->second->mNumberMesh);
		for (int i = 0; i < it->second->mNumberMesh; ++i)
			meshIds.push_back(meshRecords[it->second->mMeshHeadingList[i].mDescriptorName.mText]->mUid);
		for (size_t i = 0; i < meshIds.size(); ++i)
		{
			rtn = AddMeshNum(meshIds[i], mMeshOcurNum);
			CHECK_RTN(rtn);
			for (size_t j = i + 1; j < meshIds.size(); ++j)
			{
				rtn = AddMeshPairNum(meshIds[i], meshIds[j], mMeshPairOcurNum);
				CHECK_RTN(rtn);
			}
		}
	}
	if (printLog != SILENT)
		clog << endl;

	mUniqueMeshNum = (int)mMeshOcurNum.size();
	mUniqueMeshPairNum = (int)mMeshPairOcurNum.size();

	if (printLog != SILENT)
	{
		clog << "Total get " << mUniqueMeshNum << " unique meshs in citationset" << endl;
		clog << "Total get " << mUniqueMeshPairNum << " unique meshs pair in citationset" << endl;
	}

	rtn = InitializeMeshNeiboNum(mMeshPairOcurNum, mMeshNeiboNum);
	CHECK_RTN(rtn);
	return 0;
}

int MeshInfoSet::InitializeMeshEntry(CitationSet& citationSet, MeshRecordSet& meshRecords, EntryMapFeature& entryMap, int printLog)
{
	int rtn = 0;

	mTitleMeshEntryOcurNum.clear();
	mTitleMeshEntryPairOcurNum.clear();
	mAbstractMeshEntryOcurNum.clear();
	mAbstractMeshEntryPairOcurNum.clear();

	vector<Citation*> citationVector;
	citationVector.reserve(citationSet.Size());
	for (map<int, Citation*>::iterator it = citationSet.mCitations.begin(); it != citationSet.mCitations.end(); it++)
		citationVector.push_back(it->second);

	if (printLog != SILENT)
		clog << "Extract title entry feature" << endl;
	FeatureSet titleFeature;
	rtn = entryMap.ExtractTitle(citationVector, titleFeature);
	CHECK_RTN(rtn);

	if (printLog != SILENT)
		clog << "Extract abstract entry feature" << endl;
	FeatureSet abstractFeature;
	rtn = entryMap.ExtractAbstract(citationVector, abstractFeature);
	CHECK_RTN(rtn);

	for (int i = 0; i < titleFeature.Size(); ++i)
	{
		if (printLog != SILENT && (i & ((1 << 18) - 1)) == 0)
			clog << "\r" << i << " citation title counting";
		for (Feature::iterator it = titleFeature[i].begin(); it != titleFeature[i].end(); ++it)
		{
			if (it->second > 0.0)
			{
				rtn = AddMeshNum(it->first, mTitleMeshEntryOcurNum);
				CHECK_RTN(rtn);

				Feature::iterator it2 = it;
				while ((++it2) != titleFeature[i].end())
				{
					if (it2->second > 0)
					{
						rtn = AddMeshPairNum(it->first, it2->first, mTitleMeshEntryPairOcurNum);
						CHECK_RTN(rtn);
					}
				}
			}
		}
	}

	if (printLog != SILENT)
	{
		clog << "\nTotal " << mTitleMeshEntryOcurNum.size() << " meshs occur entry in title" << endl;
		clog << "Total " << mTitleMeshEntryPairOcurNum.size() << " meshs pair occur entry in title" << endl;
	}

	for (int i = 0; i < abstractFeature.Size(); ++i)
	{
		if (printLog != SILENT && (i & ((1 << 18) - 1)) == 0)
			clog << "\r" << i << " citation abstract counting";
		for (Feature::iterator it = abstractFeature[i].begin(); it != abstractFeature[i].end(); ++it)
		{
			if (it->second > 0.0)
			{
				rtn = AddMeshNum(it->first, mAbstractMeshEntryOcurNum);
				CHECK_RTN(rtn);

				Feature::iterator it2 = it;
				while ((++it2) != abstractFeature[i].end())
				{
					if (it2->second > 0)
					{
						rtn = AddMeshPairNum(it->first, it2->first, mAbstractMeshEntryPairOcurNum);
						CHECK_RTN(rtn);
					}
				}
			}
		}
	}
	if (printLog != SILENT)
	{
		clog << "\nTotal " << mAbstractMeshEntryOcurNum.size() << " meshs occur entry in abstract" << endl;
		clog << "Total " << mAbstractMeshEntryPairOcurNum.size() << " meshs pair occur entry in abstract" << endl;
	}
	return 0;
}

int MeshInfoSet::Initialize(const string& citationFileName, const string& meshRecordFileName, const string& entryMapFileName, int printLog)
{
	int rtn = 0;
	CitationSet citationSet;
	if (printLog != SILENT)
		clog << "Load citations" << endl;
	rtn = citationSet.Load(citationFileName.c_str(), printLog);
	CHECK_RTN(rtn);
	if (printLog != SILENT)
		clog << "Load meshrecordset" << endl;
	MeshRecordSet meshRecords;
	rtn = meshRecords.Load(meshRecordFileName.c_str());
	CHECK_RTN(rtn);
	if (printLog != SILENT)
		clog << "Load entrymap" << endl;
	EntryMapFeature mapFeature;
	rtn = mapFeature.Load(entryMapFileName.c_str());
	CHECK_RTN(rtn);
	if (printLog != SILENT)
		clog << "build trie" << endl;
	rtn = mapFeature.mMeshMap.BuildTrie();
	CHECK_RTN(rtn);

	rtn = Initialize(citationSet, meshRecords, mapFeature, printLog);
	CHECK_RTN(rtn);
	return 0;
}

int MeshInfoSet::Initialize(CitationSet& citationSet, MeshRecordSet& meshRecords, EntryMapFeature& entryMap, int printLog)
{
	int rtn = 0;
	rtn = Clear();
	CHECK_RTN(rtn);
	rtn = InitializeMesh(citationSet, meshRecords, printLog);
	CHECK_RTN(rtn);

	rtn = InitializeMeshEntry(citationSet, meshRecords, entryMap, printLog);
	CHECK_RTN(rtn);
	return 0;
}

int MeshInfoSet::QueryTotalCitationNum(int& totalCitationNum)
{
	totalCitationNum = mTotalCitationNum;
	return 0;
}

int MeshInfoSet::QueryMeshOcurProb(int keyMesh, double& prob)
{
	prob = 0.0;
	if (mMeshOcurNum.count(keyMesh) == 0)
		return 0;
	prob = double(mMeshOcurNum[keyMesh]) / double(mTotalCitationNum);
	return 0;
}

int MeshInfoSet::QueryMeshPairOcurProb(int mesh1, int mesh2, double& prob)
{
	int rtn = 0;
	prob = 0.0;
	if (mesh1 == mesh2)
	{
		prob = 0.0;
		return 0;
	}
	if (mMeshOcurNum.count(mesh1) > 0 && mMeshOcurNum[mesh1] > 0
		&& mMeshOcurNum.count(mesh2) > 0 && mMeshOcurNum[mesh2] > 0)
	{
		int pairCnt = 0;
		rtn = GetPairNum(mesh1, mesh2, mMeshPairOcurNum, pairCnt);
		CHECK_RTN(rtn);
		prob = double(pairCnt) / double(mTotalCitationNum);
	}
	return 0;
}

int MeshInfoSet::QueryTitleEntryOcurProb(int keyMesh, double& prob)
{
	prob = 0.0;
	if (mTitleMeshEntryOcurNum.count(keyMesh) == 0)
		return 0;
	prob = double(mTitleMeshEntryOcurNum[keyMesh]) / double(mTotalCitationNum);
	return 0;
}

int MeshInfoSet::QueryAbstractEntryOcurProb(int keyMesh, double& prob)
{
	prob = 0.0;
	if (mAbstractMeshEntryOcurNum.count(keyMesh) == 0)
		return 0;
	prob = double(mAbstractMeshEntryOcurNum[keyMesh]) / double(mTotalCitationNum);
	return 0;
}

int MeshInfoSet::QueryMeshNeiboOcurProb(int keyMesh, int neighborMesh, double& prob)
{
	int rtn = 0;
	prob = 0.0;
	if (keyMesh == neighborMesh)
	{
		prob = 1.0;
		return 0;
	}
	if (mMeshOcurNum.count(keyMesh) > 0 && mMeshOcurNum[keyMesh] > 0)
	{
		int pairCnt = 0;
		rtn = GetPairNum(keyMesh, neighborMesh, mMeshPairOcurNum, pairCnt);
		CHECK_RTN(rtn);
		prob = double(pairCnt) / double(mMeshOcurNum[keyMesh]);
	}
	return 0;
}

int MeshInfoSet::QueryTitleEntryNeiboOcurProb(int keyMesh, int neighborMesh, double& prob)
{
	int rtn = 0;
	prob = 0.0;
	if (keyMesh == neighborMesh)
	{
		prob = 1.0;
		return 0;
	}
	if (mTitleMeshEntryOcurNum.count(keyMesh) > 0 && mTitleMeshEntryOcurNum[keyMesh] > 0)
	{
		int pairCnt = 0;
		rtn = GetPairNum(keyMesh, neighborMesh, mTitleMeshEntryPairOcurNum, pairCnt);
		CHECK_RTN(rtn);
		prob = double(pairCnt) / double(mTitleMeshEntryOcurNum[keyMesh]);
	}
	return 0;
}

int MeshInfoSet::QueryAbstractEntryNeiboOcurProb(int keyMesh, int neighborMesh, double& prob)
{
	int rtn = 0;
	prob = 0.0;
	if (keyMesh == neighborMesh)
	{
		prob = 1.0;
		return 0;
	}
	if (mAbstractMeshEntryOcurNum.count(keyMesh) > 0 && mAbstractMeshEntryOcurNum[keyMesh] > 0)
	{
		int pairCnt = 0;
		rtn = GetPairNum(keyMesh, neighborMesh, mAbstractMeshEntryPairOcurNum, pairCnt);
		CHECK_RTN(rtn);
		prob = double(pairCnt) / double(mAbstractMeshEntryOcurNum[keyMesh]);
	}
	return 0;
}

int MeshInfoSet::QueryMeshTopNeibo(int meshId, int rank, std::vector<int>& neiboIds)
{
	neiboIds.clear();
	if (mMeshNeiboNum.count(meshId) == 0)
		return 0;
	vector<pair<int, int>> neighbors = mMeshNeiboNum[meshId];
	for (size_t i = 0; i < (size_t)rank && neighbors.size(); ++i)
		neiboIds.push_back(neighbors[i].first);
	return 0;
}

int MeshInfoSet::Clear()
{
	mSumMeshOcurNum = 0;
	mUniqueMeshNum = 0;
	mUniqueMeshPairNum = 0;
	mTotalCitationNum = 0;
	mMeshOcurNum.clear();
	mMeshPairOcurNum.clear();

	mTitleMeshEntryOcurNum.clear();
	mTitleMeshEntryPairOcurNum.clear();
	mAbstractMeshEntryOcurNum.clear();
	mAbstractMeshEntryPairOcurNum.clear();

	//mMeshPmids.clear();
	return 0;
}

int MeshInfoSet::Save(const string fileName, int printLog)
{
	int rtn = 0;
	FILE *outFile = fopen(fileName.c_str(), "wb");
	if (outFile == NULL)
		return -1;
	rtn = Save(outFile, printLog);
	CHECK_RTN(rtn);
	fclose(outFile);
	return 0;
}

int MeshInfoSet::Save(FILE* outFile, int printLog)
{
	if (outFile == NULL)
		return -1;
	int rtn = 0;
	rtn = Write(outFile, mTotalCitationNum);
	CHECK_RTN(rtn);
	rtn = Write(outFile, mUniqueMeshNum);
	CHECK_RTN(rtn);
	rtn = Write(outFile, mUniqueMeshPairNum);
	CHECK_RTN(rtn);
	rtn = Write(outFile, mSumMeshOcurNum);
	CHECK_RTN(rtn);

	rtn = Write(outFile, mMeshOcurNum);
	CHECK_RTN(rtn);
	rtn = Write(outFile, mMeshPairOcurNum);
	CHECK_RTN(rtn);

	rtn = Write(outFile, mTitleMeshEntryOcurNum);
	CHECK_RTN(rtn);
	rtn = Write(outFile, mTitleMeshEntryPairOcurNum);
	CHECK_RTN(rtn);

	rtn = Write(outFile, mAbstractMeshEntryOcurNum);
	CHECK_RTN(rtn);
	rtn = Write(outFile, mAbstractMeshEntryPairOcurNum);
	CHECK_RTN(rtn);

	if (printLog != SILENT)
		clog << "MeshInfoSet saved completed, saved " << mTotalCitationNum << " citations" << endl;
	return 0;
}

int MeshInfoSet::Load(const string fileName, int printLog)
{
	int rtn = 0;
	if (!FileExist(fileName))
		return -1;
	FileBuffer buffer(fileName.c_str());
	rtn = Load(buffer, printLog);
	CHECK_RTN(rtn);
	return 0;
}

int MeshInfoSet::Load(FILE* inFile, int printLog)
{
	if (inFile == NULL)
		return -1;
	int rtn = 0;
	rtn = Read(inFile, mTotalCitationNum);
	CHECK_RTN(rtn);
	rtn = Read(inFile, mUniqueMeshNum);
	CHECK_RTN(rtn);
	rtn = Read(inFile, mUniqueMeshPairNum);
	CHECK_RTN(rtn);
	rtn = Read(inFile, mSumMeshOcurNum);
	CHECK_RTN(rtn);

	rtn = Read(inFile, mMeshOcurNum);
	CHECK_RTN(rtn);
	rtn = Read(inFile, mMeshPairOcurNum);
	CHECK_RTN(rtn);

	rtn = Read(inFile, mTitleMeshEntryOcurNum);
	CHECK_RTN(rtn);
	rtn = Read(inFile, mTitleMeshEntryPairOcurNum);
	CHECK_RTN(rtn);

	rtn = Read(inFile, mAbstractMeshEntryOcurNum);
	CHECK_RTN(rtn);
	rtn = Read(inFile, mAbstractMeshEntryPairOcurNum);
	CHECK_RTN(rtn);

	rtn = InitializeMeshNeiboNum(mMeshPairOcurNum, mMeshNeiboNum);
	CHECK_RTN(rtn);

	if (printLog != SILENT)
		clog << "MeshInfoSet load completed, loaded " << mTotalCitationNum << " citations" << endl;
	return 0;
}

int MeshInfoSet::Load(FileBuffer& buffer, int printLog)
{
	if (buffer.Eof())
		return -1;
	int rtn = 0;
	rtn = buffer.GetNextData(mTotalCitationNum);
	CHECK_RTN(rtn);
	rtn = buffer.GetNextData(mUniqueMeshNum);
	CHECK_RTN(rtn);
	rtn = buffer.GetNextData(mUniqueMeshPairNum);
	CHECK_RTN(rtn);
	rtn = buffer.GetNextData(mSumMeshOcurNum);
	CHECK_RTN(rtn);

	rtn = buffer.GetNextData(mMeshOcurNum);
	CHECK_RTN(rtn);
	rtn = buffer.GetNextData(mMeshPairOcurNum);
	CHECK_RTN(rtn);

	rtn = buffer.GetNextData(mTitleMeshEntryOcurNum);
	CHECK_RTN(rtn);
	rtn = buffer.GetNextData(mTitleMeshEntryPairOcurNum);
	CHECK_RTN(rtn);

	rtn = buffer.GetNextData(mAbstractMeshEntryOcurNum);
	CHECK_RTN(rtn);
	rtn = buffer.GetNextData(mAbstractMeshEntryPairOcurNum);
	CHECK_RTN(rtn);

	rtn = InitializeMeshNeiboNum(mMeshPairOcurNum, mMeshNeiboNum);
	CHECK_RTN(rtn);

	if (printLog != SILENT)
		clog << "MeshInfoSet load completed, loaded " << mTotalCitationNum << " citations" << endl;
	return 0;
}

int LtrInstanceInfoSet::Initialize(string pmidMeshFile, string ltrScoreFile, string featureFile, string goldStandardFile, string meshRecordFile, int printLog)
{
	int rtn = 0;
	rtn = Clear();
	CHECK_RTN(rtn);
	MeshRecordSet meshRecords;
	rtn = meshRecords.Load(meshRecordFile.c_str());
	CHECK_RTN(rtn);
	rtn = Initialize(pmidMeshFile, ltrScoreFile, featureFile, printLog);
	CHECK_RTN(rtn);

	if (printLog != SILENT)
		clog << "Load goldstandard" << endl;
	MultiLabelAnswerSet goldstandard;
	rtn = goldstandard.LoadJsonSet(goldStandardFile.c_str());
	CHECK_RTN(rtn);

	if (goldstandard.Size() != mPmidList.size())
		cerr << "Error: GoldStandard.size not equal to mPmidList.size" << endl;
	for (int i = 0; i < goldstandard.Size(); ++i)
	{
		if (goldstandard[i].mPmid != mPmidList[i])
			cerr << "Error: GoldStandard' pmid not match to mPmidList" << endl;
		set<int> meshIds;
		for (size_t j = 0; j < goldstandard[i].mLabels.size(); ++j)
			meshIds.insert(meshRecords[goldstandard[i].mLabels[j]]->mUid);
		mGoldStandard.push_back(meshIds);
	}

	return 0;
}

int LtrInstanceInfoSet::Initialize(std::string pmidMeshFile, std::string ltrScoreFile, std::string featureFile, int printLog)
{
	int rtn = 0;
	rtn = Clear();
	CHECK_RTN(rtn);
	vector<int> qids, meshIds;
	map<int, int> numLabels;
	if (printLog != SILENT)
		clog << "Load pmid mesh numlabel file" << endl;
	rtn = LoadPmidMeshNumlabel(pmidMeshFile.c_str(), qids, meshIds, numLabels);
	CHECK_RTN(rtn);

	if (printLog != SILENT)
		clog << "Load ltr score file" << endl;
	vector<double> scores;
	rtn = LoadLtrToolScore(ltrScoreFile.c_str(), scores);
	CHECK_RTN(rtn);

	if (printLog != SILENT)
		clog << "Load ltr feature file" << endl;
	vector<double> tmpLabels;
	vector<int> tmpQids;
	vector<Feature> features;
	rtn = LoadLtrFeature(featureFile.c_str(), tmpLabels, tmpQids, features);
	CHECK_RTN(rtn);

	if (qids != tmpQids)
	{
		cerr << "Error: pmidMeshFile not match to featureFile" << endl;
		return -1;
	}
	if (scores.size() != qids.size())
	{
		cerr << "Error: ltrScore.size not equal to qids.size" << endl;
		return -1;
	}

	clog << "Extract ltr data" << endl;
	for (size_t i = 0; i < qids.size();)
	{
		int pmid = qids[i];
		vector<pair<int, double>> ltrScore;
		vector<pair<int, Feature>> ltrFeature;
		while (i < qids.size() && pmid == qids[i])
		{
			ltrScore.push_back(make_pair(meshIds[i], scores[i]));
			ltrFeature.push_back(make_pair(meshIds[i], features[i]));
			++i;
		}
		mPmidList.push_back(pmid);
		mPredictNumlabel.push_back(numLabels[pmid]);
		mLtrFeature.push_back(ltrFeature);
		mLtrScore.push_back(ltrScore);
	}
	return 0;
}

int LtrInstanceInfoSet::Initialize(std::string pmidMeshFile, std::string ltrScoreFile, int printLog)
{
	int rtn = 0;
	rtn = Clear();
	CHECK_RTN(rtn);
	vector<int> qids, meshIds;
	map<int, int> numLabels;
	if (printLog != SILENT)
		clog << "Load pmid mesh numlabel file" << endl;
	rtn = LoadPmidMeshNumlabel(pmidMeshFile.c_str(), qids, meshIds, numLabels);
	CHECK_RTN(rtn);

	if (printLog != SILENT)
		clog << "Load ltr score file" << endl;
	vector<double> scores;
	rtn = LoadLtrToolScore(ltrScoreFile.c_str(), scores);
	CHECK_RTN(rtn);

	if (scores.size() != qids.size())
	{
		cerr << "Error: ltrScore.size not equal to qids.size" << endl;
		return -1;
	}

	clog << "Extract ltr data" << endl;
	for (size_t i = 0; i < qids.size();)
	{
		int pmid = qids[i];
		vector<pair<int, double>> ltrScore;
		vector<pair<int, Feature>> ltrFeature;
		while (i < qids.size() && pmid == qids[i])
		{
			ltrScore.push_back(make_pair(meshIds[i], scores[i]));
			++i;
		}
		mPmidList.push_back(pmid);
		mPredictNumlabel.push_back(numLabels[pmid]);
		mLtrFeature.push_back(ltrFeature);
		mLtrScore.push_back(ltrScore);
	}
	return 0;
}

int LtrInstanceInfoSet::Clear()
{
	mPmidList.clear();
	mLtrFeature.clear();
	mLtrScore.clear();
	mGoldStandard.clear();
	mPredictNumlabel.clear();
	return 0;
}

CitationPredictInfoSet::CitationPredictInfoSet()
{
	Clear();
}

CitationPredictInfoSet::CitationPredictInfoSet(CitationSet& citationSet)
{
	Initialize(citationSet);
}

CitationPredictInfoSet::~CitationPredictInfoSet()
{
	Clear();
}

int CitationPredictInfoSet::Initialize(CitationSet& citationSet, int printLog)
{
	int rtn = 0;
	rtn = Clear();
	CHECK_RTN(rtn);

	for (map<int, Citation*>::iterator it = citationSet.mCitations.begin(); it != citationSet.mCitations.end(); ++it)
	{
		mPmidList.push_back(it->first);
		rtn = mGoldStandard.AddAnswer(MultiLabelAnswer(it->second));
		CHECK_RTN(rtn);
	}
	if (printLog != SILENT)
		clog << "PmidList.size = " << mPmidList.size() << endl;
	return 0;
}

int CitationPredictInfoSet::Initialize(CitationSet& citationSet, std::string metaScoreFile, std::string entryTitleScoreFile, std::string entryAbstractScoreFile, std::string ltrScoreFile, std::string ltrPmidMeshFile, std::string mtidefFile, std::string mtiflFile, std::string meshRecordFile, std::string* numlabelFile, int printLog)
{
	int rtn = 0;
	rtn = Initialize(citationSet,printLog);

	if (printLog != SILENT)
		clog << "Load metalabel score" << endl;
	rtn = LoadPredictScore(metaScoreFile, mMetalabelScore);
	CHECK_RTN(rtn);

	if (printLog != SILENT)
		clog << "Load entrytitle score" << endl;
	rtn = LoadPredictScore(entryTitleScoreFile, mEntrymapTitleScore);
	CHECK_RTN(rtn);

	if (printLog != SILENT)
		clog << "Load entryabstract score" << endl;
	rtn = LoadPredictScore(entryAbstractScoreFile, mEntrymapAbstractScore);
	CHECK_RTN(rtn);

	if (printLog != SILENT)
		clog << "Load ltr score" << endl;
	if (numlabelFile==NULL)
	{
		rtn = LoadLtrScore(ltrScoreFile, ltrPmidMeshFile, true);
		CHECK_RTN(rtn);
	}
	else
	{
		rtn = LoadLtrScore(ltrScoreFile, ltrPmidMeshFile);
		CHECK_RTN(rtn);
		rtn = LoadPredictNumLabel(*numlabelFile);
		CHECK_RTN(rtn);
	}

	if (printLog != SILENT)
		clog << "Load mtidef prediction" << endl;
	rtn = LoadMtiPrediction(mtidefFile, meshRecordFile, mMtidefPrediction);
	CHECK_RTN(rtn);

	if (printLog != SILENT)
		clog << "Load mtifl prediction" << endl;
	rtn = LoadMtiPrediction(mtiflFile, meshRecordFile, mMtiflPrediction);
	CHECK_RTN(rtn);

	return 0;
}

int CitationPredictInfoSet::LoadPredictScore(const std::string& fileName, std::vector<std::vector<std::pair<int, double>>>& predictScore)
{
	int rtn = 0;
	rtn = CheckInit(fileName);
	CHECK_RTN(rtn);

	predictScore.clear();
	rtn = MultiLabelAnswerSet::LoadPredictScores(predictScore, fileName.c_str());
	CHECK_RTN(rtn);

	if (predictScore.size() != mPmidList.size())
	{
		cerr << "Error: predictScore.size not equal to mPmidList.size" << endl;
		return -1;
	}
	return 0;
}

int CitationPredictInfoSet::LoadLtrScore(const std::string& ltrScoreFile, const std::string& pmidMeshFile, const bool initializeNumlabel)
{
	int rtn = 0;
	rtn = CheckInit(ltrScoreFile);
	CHECK_RTN(rtn);
	rtn = CheckInit(pmidMeshFile);
	CHECK_RTN(rtn);
	mLtrScore.clear();
	if (initializeNumlabel)
		mPredictNumLabel.clear();

	vector<int> qids, meshIds;
	map<int, int> numLabels;
	rtn = LoadPmidMeshNumlabel(pmidMeshFile.c_str(), qids, meshIds, numLabels);
	CHECK_RTN(rtn);

	vector<double> scores;
	rtn = LoadLtrToolScore(ltrScoreFile.c_str(), scores);
	CHECK_RTN(rtn);

	for (size_t i = 0, cur = 0; i < qids.size(); ++cur)
	{
		int pmid = qids[i];
		if (mPmidList[cur] != pmid || cur >= mPmidList.size())
		{
			cerr << "Error: ltr qid not match mPmidList" << endl;
			return -1;
		}
		vector<pair<int, double>> ltrScore;
		while (i < qids.size() && pmid == qids[i])
		{
			ltrScore.push_back(make_pair(meshIds[i], scores[i]));
			++i;
		}
		if (initializeNumlabel)
		{
			mPredictNumLabel[pmid] = numLabels[pmid];
		}
		mLtrScore.push_back(ltrScore);
	}

	return 0;
}

int CitationPredictInfoSet::LoadMtiPrediction(const std::string& fileName,const std::string& meshRecordFile, MultiLabelAnswerSet& mtiPrediction)
{
	int rtn = 0;
	rtn = CheckInit(fileName);
	CHECK_RTN(rtn);

	MultiLabelAnswerSet answers;
	rtn = answers.LoadUidJsonSet(fileName.c_str(), meshRecordFile.c_str());
	CHECK_RTN(rtn);

	map<int, MultiLabelAnswer> mapAnswers;
	for (int i = 0; i < answers.Size(); ++i)
		mapAnswers[answers[i].mPmid] = answers[i];

	mtiPrediction.mLabelSet.clear();
	for (size_t i = 0; i < mPmidList.size(); ++i)
	{
		if (mapAnswers.count(mPmidList[i]) == 0)
		{
			cerr << "Error: Can't find pmid:" << mPmidList[i] << " in mti prediction set" << endl;
			return -1;
		}
		rtn = mtiPrediction.AddAnswer(mapAnswers[mPmidList[i]]);
		CHECK_RTN(rtn);
	}
	return 0;
}

int CitationPredictInfoSet::LoadGoldStandard(const std::string& fileName)
{
	int rtn = 0;
	rtn = CheckInit(fileName);
	rtn = mGoldStandard.LoadJsonSet(fileName.c_str());
	CHECK_RTN(rtn);

	if ((size_t)mGoldStandard.Size() != mPmidList.size())
	{
		cerr << "Error: mGoldStandard not equal to mPmidList.size" << endl;
		return -1;
	}
	return 0;
}

int CitationPredictInfoSet::LoadPredictNumLabel(const std::string& fileName)
{
	int rtn = 0;
	rtn = CheckInit(fileName);
	CHECK_RTN(rtn);
	FileBuffer buffer(fileName.c_str());
	rtn = buffer.GetNextData(mPredictNumLabel);
	CHECK_RTN(rtn);
	if (mPredictNumLabel.size() != mPmidList.size())
	{
		cerr << "Error: mPredictSize not equal to mPmidList.size" << endl;
		return -1;
	}
	return 0;
}

int CitationPredictInfoSet::Clear()
{
	mPmidList.clear();
	mMetalabelScore.clear();
	mEntrymapTitleScore.clear();
	mEntrymapAbstractScore.clear();
	mLtrScore.clear();
	mMtidefPrediction.mLabelSet.clear();
	mMtiflPrediction.mLabelSet.clear();
	mGoldStandard.mLabelSet.clear();
	mPredictNumLabel.clear();
	return 0;
}

int CitationPredictInfoSet::CheckInit(const std::string& fileName)
{
	if (mPmidList.empty())
	{
		cerr << "Error: mPmidList is empty, please initialize pmidList first!" << endl;
		return -1;
	}

	if (!FileExist(fileName))
	{
		cerr << "Error: " << fileName << " not exist, please confirm filename" << endl;
		return -1;
	}
	return 0;
}
