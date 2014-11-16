#include "feature.h"
#include "omp.h"
using namespace std;

UniGramFeature::UniGramFeature()
{
	mDictionary.clear();
	mIndex.clear();
	mTotCitations = 0;
}

UniGramFeature::~UniGramFeature()
{

}

UniGramFeature::UniGramFeature(const char* const fileName)
{
	Load(fileName);
}

UniGram UniGramFeature::operator[](int idx)
{
	if (mDictionary.count(idx) > 0)
		return mDictionary[idx];
	else
		return UniGram(-1, 0, 0);
}

UniGram UniGramFeature::find(int uniGram)
{
	if (mIndex.count(uniGram) > 0)
		return *mIndex[uniGram];
	else
		return UniGram(-1, 0, 0);
}

int UniGramFeature::Load(FILE* binFile)
{
	int rtn = 0;
	FileBuffer buffer(binFile);
	rtn = buffer.GetNextData(mTotCitations);
	CHECK_RTN(rtn);
	while (!buffer.Eof())
	{
		int id = -1;
		rtn = buffer.GetNextData(id);
		CHECK_RTN(rtn);
		int first = 0;
		rtn = buffer.GetNextData(first);
		CHECK_RTN(rtn);
		CHECK_RTN(rtn);
		int appear;
		rtn = buffer.GetNextData(appear);
		CHECK_RTN(rtn);
		mDictionary[id] = UniGram(id, first, appear);
		mIndex[first] = &mDictionary[id];
	}
	return 0;
}

int UniGramFeature::Load(const char* const fileName)
{
	int rtn;
	FILE* binFile = fopen(fileName, "rb");
	rtn = Load(binFile);
	CHECK_RTN(rtn);
	fclose(binFile);
	return 0;
}

int UniGramFeature::Save(FILE* binFile)
{
	int rtn = 0;
	rtn = Write(binFile, mTotCitations);
	CHECK_RTN(rtn);
	for (map<int, UniGram>::iterator it = mDictionary.begin(); it != mDictionary.end(); it++)
	{
		rtn = Write(binFile, it->second.mId);
		CHECK_RTN(rtn);
		rtn = Write(binFile, it->second.mFirst);
		CHECK_RTN(rtn);
		rtn = Write(binFile, it->second.mAppear);
		CHECK_RTN(rtn);
	}
	return 0;
}

int UniGramFeature::Save(const char* const fileName)
{
	int rtn = 0;
	FILE* binFile = fopen(fileName, "wb");
	rtn = Save(binFile);
	CHECK_RTN(rtn);
	fclose(binFile);
	return 0;
}

int UniGramFeature::Build(TokenCitationSet& tokenCitationSet, int minAppear)
{
	int rtn = 0;
	mTotCitations = (int)tokenCitationSet.Size();
	map<int, int> status;
	status.clear();
	for (map<int, TokenCitation*>::iterator tokenCitaion = tokenCitationSet.mTokenCitations.begin(); tokenCitaion != tokenCitationSet.mTokenCitations.end(); tokenCitaion++)
	{
		set<int> appear;
		appear.clear();
		rtn = AddUniGram(appear, tokenCitaion->second->mArticleTitle);
		CHECK_RTN(rtn);
		rtn = AddUniGram(appear, tokenCitaion->second->mJournalTitle);
		CHECK_RTN(rtn);
		rtn = AddUniGram(appear, tokenCitaion->second->mAbstract);
		CHECK_RTN(rtn);
		for (set<int>::iterator it = appear.begin(); it != appear.end(); it++)
		{
			if (status.count(*it) == 0)
			{
				status[*it] = 0;
			}
			status[*it]++;
		}
	}

	int maxIndex = 0;
	for (map<int, int>::iterator it = status.begin(); it != status.end(); it++)
	if (it->second >= minAppear)
	{
		maxIndex++;
		mDictionary[maxIndex] = UniGram(maxIndex, it->first, it->second);
		mIndex[it->first] = &mDictionary[maxIndex];
	}
	return 0;
}

int UniGramFeature::Build(const char* const tokenBinFile, int minAppear)
{
	int rtn = 0;
	TokenCitationSet tokenCitationSet(tokenBinFile);
	rtn = Build(tokenCitationSet, minAppear);
	CHECK_RTN(rtn);
	return 0;
}

int UniGramFeature::BuildLhtc(LhtcDocumentSet& lhtcDocumentSet, int minAppear)
{
	int rtn = 0;
	mTotCitations = (int)lhtcDocumentSet.Size();
	map<int, int> appear;
	appear.clear();
	for (map<int, LhtcDocument>::iterator iter = lhtcDocumentSet.mLhtcDocuments.begin(); iter != lhtcDocumentSet.mLhtcDocuments.end(); ++iter)
	for (map<int, double>::iterator it = iter->second.mTf.begin(); it != iter->second.mTf.end(); ++it)
	{
		if (it->second > 0)
		{
			if (appear.count(it->first) == 0) appear[it->first] = 0;
			appear[it->first]++;
		}
	}

	int maxIndex = 0;
	for (map<int, int>::iterator it = appear.begin(); it != appear.end(); it++)
	if (it->second >= minAppear)
	{
		maxIndex++;
		mDictionary[maxIndex] = UniGram(maxIndex, it->first, it->second);
		mIndex[it->first] = &mDictionary[maxIndex];
	}
	clog << "Build " << maxIndex << " lshtc dictionary word" << endl;
	return 0;
}

int UniGramFeature::ExtractLhtc(const LhtcDocument& lhtcDocument, Feature& feature, int minAppear) const
{
	int rtn = 0;
	feature.clear();

	for (map<int, double>::const_iterator it = lhtcDocument.mTf.cbegin(); it != lhtcDocument.mTf.cend(); it++)
	{
		if (mIndex.count(it->first) == 0)
			continue;
		int index = mIndex.at(it->first)->mId;
		if (mDictionary.count(index) > 0)
		{
			if (mDictionary.at(index).mAppear >= minAppear)
			{
				double tf = (double)it->second;
				double idf = log(mTotCitations / (mDictionary.at(index).mAppear + 0.001));
				double value = tf * idf;
				feature[index] = value;
			}
		}
	}
	return 0;
}

int UniGramFeature::Extract(TokenCitation& tokenCitation, Feature& feature, int minAppear)
{
	int rtn = 0;
	feature.clear();
	map<int, int> countAppear;
	countAppear.clear();
	rtn = AddUniGram(countAppear, tokenCitation.mArticleTitle);
	CHECK_RTN(rtn);
	rtn = AddUniGram(countAppear, tokenCitation.mJournalTitle);
	CHECK_RTN(rtn);
	rtn = AddUniGram(countAppear, tokenCitation.mAbstract);
	CHECK_RTN(rtn);

	for (map<int, int>::iterator it = countAppear.begin(); it != countAppear.end(); it++)
	{
		if (mDictionary.count(it->first))
		{
			if (mDictionary[it->first].mAppear >= minAppear)
			{
				int index = it->first;
				double tf = (double)it->second;// / (CountUniGram(tokenCitation) + 0.001);
				double idf = log(mTotCitations / (mDictionary[it->first].mAppear + 0.001));// / LG;
				double value = tf * idf;
				feature[index] = value;
			}
		}
	}
	return 0;
}

int UniGramFeature::CountUniGram(Tokens &tokens)
{
	int count = 0;
	for (unsigned int i = 0; i < tokens.size(); i++)
		count += (int)tokens[i].size();
	return count;
}

int UniGramFeature::CountUniGram(TokenCitation &tokenCitation)
{
	return CountUniGram(tokenCitation.mArticleTitle) + CountUniGram(tokenCitation.mJournalTitle) + CountUniGram(tokenCitation.mAbstract);
}

int UniGramFeature::AddUniGram(set<int> &grams, Tokens &tokens)
{
	for (unsigned int i = 0; i < tokens.size(); i++)
	for (unsigned int j = 0; j < tokens[i].size(); j++)
		grams.insert(tokens[i][j]);
	return 0;
}

int UniGramFeature::AddUniGram(map<int, int> &grams, Tokens &tokens)
{
	for (unsigned int i = 0; i < tokens.size(); i++)
	for (unsigned int j = 0; j < tokens[i].size(); j++)
	{
		int gram = tokens[i][j];
		if (mIndex.count(gram) > 0)
		{
			int index = mIndex[gram]->mId;
			if (grams.count(index) == 0)
				grams[index] = 0;
			grams[index]++;
		}
	}
	return 0;
}

BiGramFeature::BiGramFeature()
{
	mDictionary.clear();
	mIndex.clear();
	mTotCitations = 0;
}

BiGramFeature::~BiGramFeature()
{

}

BiGramFeature::BiGramFeature(const char* const fileName)
{
	Load(fileName);
}

BiGram BiGramFeature::operator[](int idx)
{
	if (mDictionary.count(idx) > 0)
		return mDictionary[idx];
	else
		return BiGram(-1, 0, 0);
}

BiGram BiGramFeature::operator[](pair<int, int> biGram)
{
	if (mIndex.count(biGram) > 0)
		return *mIndex[biGram];
	else
		return BiGram(-1, 0, 0);
}

int BiGramFeature::Load(FILE* binFile)
{
	int rtn = 0;
	FileBuffer buffer(binFile);
	rtn = buffer.GetNextData(mTotCitations);
	CHECK_RTN(rtn);
	while (!buffer.Eof())
	{
		int id = -1;
		rtn = buffer.GetNextData(id);
		CHECK_RTN(rtn);
		int first = 0;
		rtn = buffer.GetNextData(first);
		CHECK_RTN(rtn);
		int second = 0;
		rtn = buffer.GetNextData(second);
		CHECK_RTN(rtn);
		int appear;
		rtn = buffer.GetNextData(appear);
		CHECK_RTN(rtn);
		mDictionary[id] = BiGram(id, first, second, appear);
		mIndex[pair<int, int>(first, second)] = &mDictionary[id];
	}
	return 0;
}

int BiGramFeature::Load(const char* const fileName)
{
	int rtn;
	FILE* binFile = fopen(fileName, "rb");
	rtn = Load(binFile);
	CHECK_RTN(rtn);
	fclose(binFile);
	return 0;
}

int BiGramFeature::Save(FILE* binFile)
{
	int rtn = 0;
	rtn = Write(binFile, mTotCitations);
	CHECK_RTN(rtn);
	for (map<int, BiGram>::iterator it = mDictionary.begin(); it != mDictionary.end(); it++)
	{
		rtn = Write(binFile, it->second.mId);
		CHECK_RTN(rtn);
		rtn = Write(binFile, it->second.mFirst);
		CHECK_RTN(rtn);
		rtn = Write(binFile, it->second.mSecond);
		CHECK_RTN(rtn);
		rtn = Write(binFile, it->second.mAppear);
		CHECK_RTN(rtn);
	}
	return 0;
}

int BiGramFeature::Save(const char* const fileName)
{
	int rtn = 0;
	FILE* binFile = fopen(fileName, "wb");
	rtn = Save(binFile);
	CHECK_RTN(rtn);
	fclose(binFile);
	return 0;
}

int BiGramFeature::Build(TokenCitationSet& tokenCitationSet, int minAppear)
{
	int rtn = 0;
	mTotCitations = (int)tokenCitationSet.Size();
	map<pair<int, int>, int> status;
	status.clear();
	for (map<int, TokenCitation*>::iterator tokenCitaion = tokenCitationSet.mTokenCitations.begin(); tokenCitaion != tokenCitationSet.mTokenCitations.end(); tokenCitaion++)
	{
		set<pair<int, int> > appear;
		appear.clear();
		rtn = AddBiGram(appear, tokenCitaion->second->mArticleTitle);
		CHECK_RTN(rtn);
		rtn = AddBiGram(appear, tokenCitaion->second->mJournalTitle);
		CHECK_RTN(rtn);
		rtn = AddBiGram(appear, tokenCitaion->second->mAbstract);
		CHECK_RTN(rtn);
		for (set<pair<int, int> >::iterator it = appear.begin(); it != appear.end(); it++)
		{
			if (status.count(*it) == 0)
			{
				status[*it] = 0;
			}
			status[*it]++;
		}
	}
	int maxIndex = 0;
	for (map<pair<int, int>, int>::iterator it = status.begin(); it != status.end(); it++)
	if (it->second >= minAppear)
	{
		maxIndex++;
		mDictionary[maxIndex] = BiGram(maxIndex, it->first.first, it->first.second, it->second);
		mIndex[it->first] = &mDictionary[maxIndex];
	}
	return 0;
}

int BiGramFeature::Build(const char* const tokenBinFile, int minAppear)
{
	int rtn = 0;
	TokenCitationSet tokenCitationSet(tokenBinFile);
	rtn = Build(tokenCitationSet, minAppear);
	CHECK_RTN(rtn);
	return 0;
}

int BiGramFeature::Extract(TokenCitation& tokenCitation, Feature& feature, int minAppear)
{
	int rtn = 0;
	feature.clear();
	map<int, int> countAppear;
	countAppear.clear();
	rtn = AddBiGram(countAppear, tokenCitation.mArticleTitle);
	CHECK_RTN(rtn);
	rtn = AddBiGram(countAppear, tokenCitation.mJournalTitle);
	CHECK_RTN(rtn);
	rtn = AddBiGram(countAppear, tokenCitation.mAbstract);
	CHECK_RTN(rtn);

	for (map<int, int>::iterator it = countAppear.begin(); it != countAppear.end(); it++)
	{
		if (mDictionary.count(it->first))
		{
			if (mDictionary[it->first].mAppear >= minAppear)
			{
				int index = it->first;
				double tf = (double)it->second;// / double(CountBiGram(tokenCitation));
				double idf = log(mTotCitations / (mDictionary[it->first].mAppear + 0.001));// / LG;
				double value = tf * idf;
				feature[index] = value;
			}
		}
	}
	return 0;
}

int BiGramFeature::CountBiGram(Tokens &tokens)
{
	int count = 0;
	for (unsigned int i = 0; i <tokens.size(); i++)
	if (tokens[i].size() > 1)
		count += (int)tokens[i].size() - 1;
	return count;
}

int BiGramFeature::CountBiGram(TokenCitation &tokenCitation)
{
	return CountBiGram(tokenCitation.mArticleTitle) + CountBiGram(tokenCitation.mJournalTitle) + CountBiGram(tokenCitation.mAbstract);
}

int BiGramFeature::AddBiGram(set<pair<int, int> > &grams, Tokens &tokens)
{
	for (unsigned int i = 0; i < tokens.size(); i++)
	for (unsigned int j = 0; j + 1 < tokens[i].size(); j++)
		grams.insert(pair<int, int>(tokens[i][j], tokens[i][j + 1]));
	return 0;
}

int BiGramFeature::AddBiGram(map<int, int> &grams, Tokens &tokens)
{
	for (unsigned int i = 0; i < tokens.size(); i++)
	for (unsigned int j = 0; j + 1 < tokens[i].size(); j++)
	{
		pair<int, int> gram(tokens[i][j], tokens[i][j + 1]);
		if (mIndex.count(gram) > 0)
		{
			int index = mIndex[gram]->mId;
			if (grams.count(index) == 0)
				grams[index] = 0;
			grams[index]++;
		}
	}
	return 0;
}

EntryMapFeature::EntryMapFeature()
{
	mMeshMap.Clear();
	mFeatureDim = 0;
}

EntryMapFeature::EntryMapFeature(const char* const fileName)
{
	Load(fileName);
	Init();
}

int EntryMapFeature::Init()
{
	mFeatureDim = 0;
	for (map<string, int>::iterator it = mMeshMap.mStringIndex.begin(); it != mMeshMap.mStringIndex.end(); it++)
	{
		mFeatureDim = max(mFeatureDim, it->second);
	}
	return 0;
}

int EntryMapFeature::Load(FILE* binFile)
{
	int rtn = 0;
	rtn = mMeshMap.Load(binFile);
	CHECK_RTN(rtn);
	rtn = Init();
	CHECK_RTN(rtn);
	return 0;
}

int EntryMapFeature::Load(const char* const fileName)
{
	int rtn = 0;
	rtn = mMeshMap.Load(fileName);
	CHECK_RTN(rtn);
	rtn = Init();
	CHECK_RTN(rtn);
	return 0;
}

int EntryMapFeature::Save(FILE* binFile)
{
	int rtn = mMeshMap.Save(binFile);
	CHECK_RTN(rtn);
	return 0;
}

int EntryMapFeature::Save(const char* const fileName)
{
	int rtn = mMeshMap.Save(fileName);
	CHECK_RTN(rtn);
	return 0;
}

int EntryMapFeature::ExtractTitle(Citation* citation, Feature& feature)
{
	int rtn = 0;
	feature.clear();
	rtn = mMeshMap.PhaseText(string(citation->mArticleTitle), feature);
	CHECK_RTN(rtn);
	return 0;
}

int EntryMapFeature::ExtractAbstract(Citation* citation, Feature& feature)
{
	int rtn = 0;
	feature.clear();
	string abstract;
	rtn = citation->GetAllAbstract(abstract);
	CHECK_RTN(rtn);
	rtn = mMeshMap.PhaseText(abstract, feature);
	CHECK_RTN(rtn);
	return 0;
}

int EntryMapFeature::ExtractTitle(std::vector<Citation*> &citationVector, FeatureSet& featureSet)
{
	int rtn = 0;
	featureSet.mMaxIndex = mFeatureDim;
	featureSet.mFeatures.clear();
	featureSet.mFeatures.resize(citationVector.size());

	int numThreads = omp_get_num_procs();
	clog << "CPU number: " << numThreads << endl;
	omp_set_num_threads(numThreads);
	clog << "Start Parallel Extract Title Entry Map Features" << endl;
#pragma omp parallel for schedule(dynamic)
	for (int i = 0; i < citationVector.size(); i++)
	{
		Feature feature;
		ExtractTitle(citationVector[i], featureSet.mFeatures[i]);
		//rtn = ExtractTitle(citationVector[i], featureSet.mFeatures[i]);
		//CHECK_RTN(rtn);
	}
	return 0;
}

int EntryMapFeature::ExtractAbstract(std::vector<Citation*> &citationVector, FeatureSet& featureSet)
{
	int rtn = 0;
	featureSet.mMaxIndex = mFeatureDim;
	featureSet.mFeatures.clear();
	featureSet.mFeatures.resize(citationVector.size());
	int numThreads = omp_get_num_procs();
	omp_set_num_threads(numThreads);
	clog << "Start Parallel Extract Abstract Entry Map Features" << endl;
#pragma omp parallel for schedule(dynamic)
	for (int i = 0; i < citationVector.size(); i++)
	{
		Feature feature;
		ExtractAbstract(citationVector[i], featureSet.mFeatures[i]);
		//rtn = ExtractTitle(citationVector[i], featureSet.mFeatures[i]);
		//CHECK_RTN(rtn);
	}
	return 0;
}

int EntryMapFeature::ExtractAll(std::vector<Citation*> &citationVector, FeatureSet& featureSet)
{
	int rtn = 0;
	rtn = ExtractTitle(citationVector, featureSet);
	CHECK_RTN(rtn);

	FeatureSet abstractFeature;
	rtn = ExtractAbstract(citationVector, abstractFeature);
	CHECK_RTN(rtn);

	rtn = featureSet.Merge(abstractFeature);
	CHECK_RTN(rtn);

	return 0;
}