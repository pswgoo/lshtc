#ifndef FEATURE_H
#define FEATURE_H
#include "common/data_struct.h"
#include "common/common_basic.h"
#include "medline/citation.h"
#include "lshtc_lib/lshtc_data.h"
#include "tokenization/tokenization.h"
#include <cmath>

class UniGramFeature
{
public:
	std::map<int, UniGram> mDictionary;
	std::map<int, UniGram*> mIndex;
	int mTotCitations;
public:
	UniGramFeature();

	~UniGramFeature();

	UniGramFeature(const char* const fileName);

	//return id -1 for not found
	UniGram operator[](int idx);

	UniGram find(int uniGram);

	int Load(FILE* binFile);

	int Load(const char* const fileName);

	int Save(FILE* binFile);

	int Save(const char* const fileName);

	int Build(TokenCitationSet& tokenCitationSet, int minAppear = 6);

	int Build(const char* const tokenBinFile, int minAppear = 6);

	int BuildLhtc(LhtcDocumentSet& lhtcDocumentSet, int minAppear = 2);

	int ExtractLhtc(const LhtcDocument& lhtcDocument, Feature& feature, int minAppear = 2) const;

	int Extract(TokenCitation& tokenCitation, Feature& feature, int minAppear = 6);

	static int CountUniGram(Tokens &tokens);

	static int CountUniGram(TokenCitation &tokenCitation);

	static int AddUniGram(std::set<int> &grams, Tokens &tokens);

	int AddUniGram(std::map<int, int> &grams, Tokens &tokens);
};

class BiGramFeature
{
public:
	std::map<int, BiGram> mDictionary;
	std::map<std::pair<int, int>, BiGram*> mIndex;
	int mTotCitations;
public:
	BiGramFeature();

	~BiGramFeature();

	BiGramFeature(const char* const fileName);

	//return id -1 for not found
	BiGram operator[](int idx);

	BiGram operator[](std::pair<int, int> biGram);

	int Load(FILE* binFile);

	int Load(const char* const fileName);

	int Save(FILE* binFile);

	int Save(const char* const fileName);

	int Build(TokenCitationSet& tokenCitationSet, int minAppear = 6);

	int Build(const char* const tokenBinFile, int minAppear = 6);

	int Extract(TokenCitation& tokenCitation, Feature& feature, int minAppear = 6);

	static int CountBiGram(Tokens &tokens);

	static int CountBiGram(TokenCitation &tokenCitation);

	static int AddBiGram(std::set<std::pair<int, int> > &grams, Tokens &tokens);

	int AddBiGram(std::map<int, int> &grams, Tokens &tokens);
};

class EntryMapFeature
{
public:
	EntryMap mMeshMap;
	int mFeatureDim;

public:
	EntryMapFeature();

	EntryMapFeature(const char* const fileName);

	int Init();
	
	int Load(FILE* binFile);

	int Load(const char* const fileName);

	int Save(FILE* binFile);

	int Save(const char* const fileName);

	int ExtractTitle(Citation* citation, Feature& feature);

	int ExtractAbstract(Citation* citation, Feature& feature);

	int ExtractTitle(std::vector<Citation*> &citationVector, FeatureSet& featureSet);

	int ExtractAbstract(std::vector<Citation*> &CitationVector, FeatureSet& featureSet);

	int ExtractAll(std::vector<Citation*> &CitationVector, FeatureSet& featureSet);
};

#endif /*FEATURE_H*/