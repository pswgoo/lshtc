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

int MultilabelKnn(std::string mlknn_model, std::string test_file, std::string neighbor_file, MultiLabelAnswerSet& goldStandard, std::vector<std::vector<LabelScore>>& predctLabelScore);

int MultilabelKnnEvaluate(std::string mlknn_model, std::string test_file, std::string neighbor_file, std::string numlabelFile, std::string resultFile);
