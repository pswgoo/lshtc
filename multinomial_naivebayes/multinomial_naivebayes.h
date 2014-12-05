#include "common/common_basic.h"
#include "lshtc_lib/lshtc_data.h"
#include "extractfeature/feature.h"

class MultinomialNaiveBayes
{

public:
	int mInstanceSize;
	std::vector<int> mAppearLabels;
	std::vector<int> mLabelsWordCnt;//cnt label words
	std::vector<int> mAppearFeatures;
	std::map<int, int> mTransLabels;//trans to 0..M-1
	std::map<int, int> mTransFeatures;//trans to 0..N-1
	//std::vector<std::map<int, int> > mPossiblity;//label<feature, Appear>
	std::vector<std::vector<std::pair<int, int>>> mPossiblity;//label<feature, Appear>
	std::vector<std::set<int> >	mInverseTable;   //token<label1,label2,...> 倒排表，记录每个token在哪些label的样本中出现过

public:

	MultinomialNaiveBayes();

	~MultinomialNaiveBayes();

	int Clear();

	int Save(std::string fileName, int printLog = SILENT);

	int Load(std::string fileName, int printLog = SILENT);

	int Build(const std::vector<std::map<int, double> >& trainSet, const std::vector<std::vector<int> >& trainLabels, int printLog = SILENT);  //

	int Predict(const Feature& testInstance, std::vector<int>& labelID, int topK);

	int Predict(const Feature& testInstance, std::vector<std::pair<int, double> >& labelScore, int topK);

};