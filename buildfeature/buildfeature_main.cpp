#include "medline/basic.h"
#include "medline/citation.h"
#include "medline/mesh.h"
#include "tokenization/tokenization.h"
#include "extractfeature/feature.h"

const string gCitationSetFile = "../medline2014/zip/filterJournal_train.bin";
const string gTokenSetFile = "token_djy_filterJournal_train(4.28).bin";
const string gUnigramDictFile = "djy_unigramfeature(4.28).bin";
const string gBigramDictFile = "djy_bigramfeature(4.28).bin";
const string gTokenDictFile = "dictionary_djytoken(4.28).bin";

int main()
{
	int rtn = 0;

	CitationSet citations;
	clog << "Loading Training Citations" << endl;
	rtn = citations.Load(gCitationSetFile.c_str(), STATUS_ONLY);
	CHECK_RTN(rtn);

	clog << "Loading Tokenization Result" << endl;
	TokenCitationSet tokenCitations;
	rtn = tokenCitations.LoadDictionary(gTokenDictFile.c_str(), STATUS_ONLY);
	CHECK_RTN(rtn);

	rtn = tokenCitations.Load(gTokenSetFile.c_str(), STATUS_ONLY);
	CHECK_RTN(rtn);

	clog << "Build UniGram Features" << endl;
	UniGramFeature uniGrams;
	rtn = uniGrams.Build(tokenCitations);
	CHECK_RTN(rtn);
	rtn = uniGrams.Save(gUnigramDictFile.c_str());
	clog << "Saved " << uniGrams.mDictionary.size() << " unigrams" << endl;

	clog << "Build BiGram Features" << endl;
	BiGramFeature biGrams;
	rtn = biGrams.Build(tokenCitations);
	CHECK_RTN(rtn);
	rtn = biGrams.Save(gBigramDictFile.c_str());
	clog << "Saved " << biGrams.mDictionary.size() << " biGrams" << endl;
	clog << "Complete" << endl;
	return 0;
}