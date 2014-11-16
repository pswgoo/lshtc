#include "lshtc_lib/lshtc_data.h""
#include "common/common_basic.h"
#include "common/data_struct.h"
#include "classifier/classifier_tools.h"

int SaveLabelFreq(LhtcDocumentSet& lshtcSet, std::string freqFileName);

int SavePmidMesh(LhtcDocumentSet& lshtcSet, std::string instanceIdLabelIdFile);

int SaveLhtcFeature(LhtcDocumentSet& lshtcSet, std::string outFeatureFile, std::string instanceIdLabelIdFile, std::string unigramFile = "lshtc_unigram_dictionary_loctrain.bin");

int ExtractFeature(const std::vector<LhtcDocument*>& tokenVector, UniGramFeature& uniGrams, feature_node** &featureSpace, int printLog);

int BuildUnigramDictionary(LhtcDocumentSet& lshtcSet, std::string unigramFile);

int MetaLabelNumModelTrain(const std::string tokenFile, const std::string unigramFile, const std::string outModelPath);
