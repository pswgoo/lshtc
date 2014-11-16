#ifndef MEDLINE_TOOL_FUNCTION_H
#define MEDLINE_TOOL_FUNCTION_H

#include "medline/citation.h"
#include "classifier/classifier.h"
#include "neighbor/citationneighbor.h"
#include "tokenization/tokenization.h"

//modified 2014/04/28 by Peng Shengwen
bool CmpDate(const Citation* p1, const Citation* p2);

int GetThreshold(const std::string modelPath, std::vector<int>& queryModels, CitationSet& ciatationSet, TokenCitationSet& tokenCitations, UniGramFeature& uniGrams, BiGramFeature& biGrams, MeshRecordSet& meshRecords, std::vector<double>& bestThres);

int GetThreshold(std::vector<std::pair<int, LinearMachine> >& queryModels, CitationSet& ciatationSet, TokenCitationSet& tokenCitations, UniGramFeature& uniGrams, BiGramFeature& biGrams, MeshRecordSet& meshRecords, std::vector<double>& bestThres);

int GetThreshold(std::vector<std::pair<int, LinearMachine> >& queryModels, CitationSet& ciatationSet, FeatureSet& featureSet, MeshRecordSet& meshRecords, std::vector<double>& bestThres);

int GetThreshold(const std::vector<int>& queryModels, const std::vector<std::vector<std::pair<int, double> > >& predicScores, CitationSet& ciatationSet, MeshRecordSet& meshRecords, std::vector<double>& bestThres);

int ExtractNumlabelFeature(const std::vector<int>& pmids, CitationSet& citationSet, std::vector<std::vector<std::pair<int, double>>>& predictScore, CitationNeighborSet& neighborSet, JournalLabelNum& journalLabelNum, JournalSet& journalSet, FeatureSet& featureSet, int printLog = SILENT);

int ExtractNumlabelFeature(const std::vector<int>& pmids, CitationSet& citationSet, std::vector<std::vector<std::pair<int, double>>>& predictScore, CitationNeighborSet& neighborSet, JournalLabelNum& journalLabelNum, JournalSet& journalSet, std::vector<std::vector<std::pair<int, double>>>& ltrScore, std::vector<int>& metalabelNum, std::vector<int>& mtidefNum, std::vector<int>& mtiflNum, FeatureSet& featureSet, int printLog = SILENT);

int ExtractNumlabelFeature(const std::vector<int>& pmids, std::vector<std::vector<std::pair<int, double>>>& predictScore, double threshold, FeatureSet& featureSet, int printLog);

int ExtractNumlabelFeature(const std::vector<int>& pmids, CitationSet& citationSet, std::vector<std::vector<std::pair<int, double>>>& predictScore, CitationNeighborSet& neighborSet, JournalLabelNum& journalLabelNum, JournalSet& journalSet/*, FeatureSet& entryTitleFeature, FeatureSet& entryAbstractFeature*/, std::vector<int>& metalabelNum, std::vector<int>& mtidefNum, std::vector<int>& mtiflNum, FeatureSet& featureSet, int printLog = SILENT);

int InitializePrecisionScoreTableSet(std::string sourPredictScore, std::string tokenDocFile, std::string tablePath);

int InitializeRecallScoreTableSet(std::string sourPredictScore, std::string citationFile, std::string meshFile, std::string tablePath);

int InitializeThreshold(std::string sourPredictScore, std::string citationFile, std::string meshFile, std::string tablePath);

int SaveLtrFeature(const char* fileName, std::vector<double>& labels, std::vector<int>& qids, std::vector<Feature>& features, std::vector<std::string>* info = NULL);

int LoadLtrFeature(const char* fileName, std::vector<double>& labels, std::vector<int>& qids, std::vector<Feature>& features, std::vector<std::string>* info = NULL);

int LoadLtrToolScore(const char* fileName, std::vector<double>& scores);

int SaveLtrToolScore(const char* fileName, std::vector<double>& scores);

int SavePmidMeshNumlabel(const char* fileName, std::vector<int>& pmids, std::vector<int>& meshIds, std::map<int, int>& numLabels);

int LoadPmidMeshNumlabel(const char* fileName, std::vector<int>& pmids, std::vector<int>& meshIds, std::map<int, int>& numLabels);

#endif
