#include "getneighbor/getneighbor.h"
#include "medline/basic.h"
#include "extractfeature/feature.h"
#include "classifier/classifier.h"
#include "evaluation/tools.h"
#include "tokenization/tokenization.h"
#include <iostream>
#include <omp.h>
#include <algorithm>
typedef std::pair<int, double> LabelScore;

int Evaluate(MultiLabelAnswerSet& goldStandard, MultiLabelAnswerSet& predictAnswers, std::string resultFile);

int NumlabelPredict(std::string modelFile, std::string testFile, std::string unigramFile, std::map<int, int>& numlabel);