#include "getneighbor/getneighbor.h"
#include "medline/basic.h"
#include "extractfeature/feature.h"
#include "classifier/classifier.h"
#include "evaluation/tools.h"
#include "tokenization/tokenization.h"
#include "predict_basic.h"
#include <iostream>
#include <omp.h>
#include <algorithm>

int CosineKnnPredict(std::string trainFile, std::string testFile, std::string neighborFile,  MultiLabelAnswerSet& goldStandard, std::vector<std::vector<LabelScore>>& predctLabelScore);

int CosineKnnEvaluate(std::string trainFile, std::string testFile, std::string neighborFile, std::string numlabelFile, std::string resultFile);
