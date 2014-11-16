#include "medlinetoolfunction.h"
#include "lshtc_lib/lshtc_data.h"
using namespace std;

bool CmpDate(const Citation* p1, const Citation* p2)
{
	if (p1->mDateCreated != p2->mDateCreated)
		return p1->mDateCreated > p2->mDateCreated;
	else
		return p1->mPmid > p2->mPmid;
}

int GetThreshold(const string modelPath, vector<int>& queryModels, CitationSet& ciatationSet, TokenCitationSet& tokenCitations, UniGramFeature& uniGrams, BiGramFeature& biGrams, MeshRecordSet& meshRecords, vector<double>& bestThres)
{
	int rtn = 0;
	vector<pair<int, LinearMachine> > queryMachines;
	queryMachines.resize(queryModels.size());
	for (size_t i = 0; i < queryModels.size(); ++i)
	{
		string modelFile = modelPath + "/" + intToString(queryModels[i]) + ".model";
		if (!FileExist(modelFile))
			return -1;
		queryMachines[i].first = queryModels[i];
		rtn = queryMachines[i].second.Load(modelFile);
		CHECK_RTN(rtn);
	}
	rtn = GetThreshold(queryMachines, ciatationSet, tokenCitations, uniGrams, biGrams, meshRecords, bestThres);
	CHECK_RTN(rtn);
	return 0;
}

int GetThreshold(vector<pair<int, LinearMachine> >& queryModels, CitationSet& ciatationSet, TokenCitationSet& tokenCitations, UniGramFeature& uniGrams, BiGramFeature& biGrams, MeshRecordSet& meshRecords, vector<double>& bestThres)
{
	int rtn = 0;
	vector<TokenCitation*> tokenCitationVector;
	tokenCitationVector.reserve(tokenCitations.Size());
	for (map<int, TokenCitation*>::iterator it = tokenCitations.mTokenCitations.begin(); it != tokenCitations.mTokenCitations.end(); it++)
	{
		tokenCitationVector.push_back(it->second);
	}

	clog << "Prepare for Extract Features" << endl;
	FeatureSet allFeatures;
	allFeatures.mMaxIndex = uniGrams.mDictionary.rbegin()->first + 1;
	allFeatures.mFeatures.resize(tokenCitations.Size());

	FeatureSet biFeatures;
	biFeatures.mMaxIndex = biGrams.mDictionary.rbegin()->first + 1;
	biFeatures.mFeatures.resize(tokenCitations.Size());

	int numThreads = omp_get_num_procs();
	clog << "CPU number: " << numThreads << endl;

	omp_set_num_threads(numThreads);
	clog << "Start Parallel Extract Features" << endl;
#pragma omp parallel for schedule(dynamic, 10) 
	for (int i = 0; i < tokenCitationVector.size(); i++)
	{
		uniGrams.Extract(*tokenCitationVector[i], allFeatures.mFeatures[i]);
	}

#pragma omp parallel for schedule(dynamic, 10) 
	for (int i = 0; i < tokenCitationVector.size(); i++)
	{
		biGrams.Extract(*tokenCitationVector[i], biFeatures.mFeatures[i]);
	}

	allFeatures.Merge(biFeatures);
	rtn = allFeatures.Normalize();
	CHECK_RTN(rtn);

	rtn = GetThreshold(queryModels, ciatationSet, allFeatures, meshRecords, bestThres);
	CHECK_RTN(rtn);
	return 0;
}

int GetThreshold(vector<pair<int, LinearMachine> >& queryModels, CitationSet& ciatationSet, FeatureSet& featureSet, MeshRecordSet& meshRecords, vector<double>& bestThres)
{
	int rtn = 0;
	clog << "Start Predict" << endl;

	vector<vector<pair<int, double> > > predicScores;
	predicScores.resize(featureSet.Size());
	for (int i = 0; i < featureSet.Size(); i++)
		predicScores[i].resize(queryModels.size());

	for (unsigned int k = 0; k < queryModels.size(); k++)
	{
		if ((k & 255) == 0)
		{
			clog << "LOG : Working for model " << queryModels[k].first << endl;
		}

#pragma omp parallel for schedule(dynamic)
		for (int i = 0; i < featureSet.Size(); i++)
		{
			predicScores[i][k].first = queryModels[k].first;
			queryModels[k].second.Predict(featureSet[i], predicScores[i][k].second);
		}
	}

	vector<int> meshIds;
	for (size_t i = 0; i < queryModels.size(); ++i)
		meshIds.push_back(queryModels[i].first);
	rtn = GetThreshold(meshIds, predicScores, ciatationSet, meshRecords, bestThres);
	CHECK_RTN(rtn);
	return 0;
}

int GetThreshold(const vector<int>& queryModels, const std::vector<std::vector<pair<int, double> > >& predicScores, CitationSet& ciatationSet, MeshRecordSet& meshRecords, vector<double>& bestThres)
{
	set<int> labelIds;
	for (int i = 0; i < (int)queryModels.size(); ++i)
		labelIds.insert(queryModels[i]);
	map<int, vector<pair<double, bool> > > modelScores;

	size_t i = 0;
	for (map<int, Citation*>::iterator it = ciatationSet.mCitations.begin(); it != ciatationSet.mCitations.end() && i < predicScores.size(); ++it, ++i)
	{
		set<int> gs;
		for (int j = 0; j < it->second->mNumberMesh; ++j)
		{
			int meshId = meshRecords[it->second->mMeshHeadingList[j].mDescriptorName.mText]->mUid;

			if (labelIds.count(meshId) > 0)
			{
				gs.insert(meshId);
			}
		}

		for (unsigned j = 0; j < predicScores[i].size(); ++j)
		{
			if (gs.count(predicScores[i][j].first) > 0)
				modelScores[predicScores[i][j].first].push_back(make_pair(predicScores[i][j].second, true));
			else
				modelScores[predicScores[i][j].first].push_back(make_pair(predicScores[i][j].second, false));
		}
	}

	bestThres.resize(queryModels.size());
	int rtn = 0;
	for (int k = 0; k < (int)queryModels.size(); ++k)
	{
		bestThres[k] = 1.0;
		rtn = Threshold::GetThreshold(modelScores[queryModels[k]], bestThres[k]);
		CHECK_RTN(rtn);
	}
	return 0;
}

int ExtractNumlabelFeature(const vector<int>& pmids, CitationSet& citationSet, vector<vector<pair<int, double>>>& predictScore, CitationNeighborSet& neighborSet, JournalLabelNum& journalLabelNum, JournalSet& journalSet, FeatureSet& featureSet, int printLog)
{
	const int LATEST_CITATION_NUM = 5;
	const int NEIGHBOR_NUM = 10;
	const int METALABEL_SCORE_NUM = 200;
	const int AVERAGE_LABEL_NUM = 13;

	int numThreads = omp_get_num_procs();
	if (printLog != SILENT)
		clog << "CPU number: " << numThreads << endl;
	omp_set_num_threads(numThreads);

	featureSet.mFeatures.clear();
	if (pmids.size() != predictScore.size())
		return -1;

	int rtn = 0;
	featureSet.mFeatures.resize(pmids.size());
#pragma omp parallel for schedule(dynamic) 
	for (int i = 0; i < (int)pmids.size(); ++i)
	{
		sort(predictScore[i].begin(), predictScore[i].end(), CmpScore);
		Feature &feature = featureSet.mFeatures[i];
		int featureIdx = 1;


		int journalId = 0;
		Journal* ptrJournal = journalSet.SearchJournalTitle(citationSet[pmids[i]]->mJournalTitle);
		if (ptrJournal != NULL)
			journalId = ptrJournal->mJournalId;
		else
			clog << "Error: Can't find journalTitle:" << citationSet[pmids[i]]->mJournalTitle << endl;
		int dateCreated = citationSet[pmids[i]]->mDateCreated;
		CitationNeighbor* neighbor = neighborSet[pmids[i]];
		if (neighbor == NULL)
			cerr << "Error: pointer neighbor is empty" << endl;

		double value = 0.0;
		//feature 1.a average label num total Journal
		journalLabelNum.GetAverageLabelNum(journalId, value);
		if (value == 0)
			value = AVERAGE_LABEL_NUM;
		feature[featureIdx++] = value;

		//feature 1.b stdDev of total Journal's label num
		journalLabelNum.GetStandardDeviation(journalId, value);
		feature[featureIdx++] = value;

		//feature 2.a average label num in the journal by year
		journalLabelNum.GetAverageLabelNum(journalId, dateCreated, value);
		if (value == 0)
			value = AVERAGE_LABEL_NUM;
		feature[featureIdx++] = value;

		//feature 2.b stdDev of feature 2.a
		journalLabelNum.GetStandardDeviation(journalId, dateCreated, value);
		feature[featureIdx++] = value;

		//feature 3.a average label num in latest 5 citation
		journalLabelNum.GetAverageLabelNum(journalId, dateCreated, LATEST_CITATION_NUM, value);
		if (value == 0)
			value = AVERAGE_LABEL_NUM;
		feature[featureIdx++] = value;

		//feature 3.b stdDev of feature 3.a
		journalLabelNum.GetStandardDeviation(journalId, dateCreated, LATEST_CITATION_NUM, value);
		feature[featureIdx++] = value;

		//feature 4.a average of top 10 neighbor's label nums
		map<int, Citation*> neighCitations;
		neighbor->GetNeighborCitation(neighCitations, NEIGHBOR_NUM);
		double neiAverage = 0.0;
		double sum = 0.0;
		for (map<int, Citation*>::iterator it = neighCitations.begin(); it != neighCitations.end(); ++it)
			sum += it->second->mNumberMesh;
		if (!neighCitations.empty())
			neiAverage = sum / (double)neighCitations.size();
		else
			neiAverage = AVERAGE_LABEL_NUM;
		feature[featureIdx++] = neiAverage;

		//feature 4.b standard deviation of top 10 neighbor's label nums
		double sum2 = 0.0;
		for (map<int, Citation*>::iterator it = neighCitations.begin(); it != neighCitations.end(); ++it)
			sum2 += ((double)it->second->mNumberMesh - neiAverage) * ((double)it->second->mNumberMesh - neiAverage);
		if (!neighCitations.empty())
			feature[featureIdx++] = sqrt(sum2 / (double)neighCitations.size());
		else
			feature[featureIdx++] = 0.0;
		/**/
		//feature 5.a top K metalabel score
		for (int j = 0; j < METALABEL_SCORE_NUM && j<predictScore[i].size(); ++j)
			feature[featureIdx++] = predictScore[i][j].second;
		
	}

	return 0;
}

int ExtractNumlabelFeature(const std::vector<int>& pmids, CitationSet& citationSet, std::vector<std::vector<std::pair<int, double>>>& predictScore, CitationNeighborSet& neighborSet, JournalLabelNum& journalLabelNum, JournalSet& journalSet, std::vector<std::vector<std::pair<int, double>>>& ltrScore, std::vector<int>& metalabelNum, std::vector<int>& mtidefNum, std::vector<int>& mtiflNum, FeatureSet& featureSet, int printLog)
{
	const int LATEST_CITATION_NUM = 5;
	const int NEIGHBOR_NUM = 10;
	const int METALABEL_SCORE_NUM = 200;
	const int AVERAGE_LABEL_NUM = 13;
	const int TOP_NEIGHBOR_SCORE_NUM = 20;
	const int TOP_LTR_SCORE_NUM = 20;
	const double LTR_COUNT_NORMALIZE_NUM = 70.0;
	const double LTR_SCORE_NORMALIZE_NUM = 10.0;

	featureSet.mFeatures.clear();
	if (pmids.size() != predictScore.size()
		|| pmids.size() != metalabelNum.size()
		|| pmids.size() != mtidefNum.size()
		|| pmids.size() != mtiflNum.size()
		|| pmids.size() != ltrScore.size())
		return -1;

	vector<double> ltrThres = { 1.0, 0.0, -1.0, -2.0 };

	vector<vector<pair<int, double>>> ltrSortScore = ltrScore;
	for (int i = 0; i < ltrSortScore.size(); ++i)
		sort(ltrSortScore[i].begin(), ltrSortScore[i].end(), CmpPairByLagerSecond<int, double>);

	int numThreads = omp_get_num_procs();
	if (printLog != SILENT)
		clog << "CPU number: " << numThreads << endl;
	omp_set_num_threads(numThreads);

	int rtn = 0;
	featureSet.mFeatures.resize(pmids.size());
#pragma omp parallel for schedule(dynamic) 
	for (int i = 0; i < (int)pmids.size(); ++i)
	{
		sort(predictScore[i].begin(), predictScore[i].end(), CmpScore);
		Feature &feature = featureSet.mFeatures[i];
		int featureIdx = 1;


		int journalId = 0;
		Journal* ptrJournal = journalSet.SearchJournalTitle(citationSet[pmids[i]]->mJournalTitle);
		if (ptrJournal != NULL)
			journalId = ptrJournal->mJournalId;
		else
			clog << "Error: Can't find journalTitle:" << citationSet[pmids[i]]->mJournalTitle << endl;
		int dateCreated = citationSet[pmids[i]]->mDateCreated;
		CitationNeighbor* neighbor = neighborSet[pmids[i]];
		if (neighbor == NULL)
			cerr << "Error: pointer neighbor is empty" << endl;

		double value = 0.0;
		//feature 1.a average label num total Journal
		journalLabelNum.GetAverageLabelNum(journalId, value);
		if (value == 0)
			value = AVERAGE_LABEL_NUM;
		feature[featureIdx++] = value;

		//feature 1.b stdDev of total Journal's label num
		journalLabelNum.GetStandardDeviation(journalId, value);
		feature[featureIdx++] = value;

		//feature 2.a average label num in the journal by year
		journalLabelNum.GetAverageLabelNum(journalId, dateCreated, value);
		if (value == 0)
			value = AVERAGE_LABEL_NUM;
		feature[featureIdx++] = value;

		//feature 2.b stdDev of feature 2.a
		journalLabelNum.GetStandardDeviation(journalId, dateCreated, value);
		feature[featureIdx++] = value;

		//feature 3.a average label num in latest 5 citation
		journalLabelNum.GetAverageLabelNum(journalId, dateCreated, LATEST_CITATION_NUM, value);
		if (value == 0)
			value = AVERAGE_LABEL_NUM;
		feature[featureIdx++] = value;

		//feature 3.b stdDev of feature 3.a
		journalLabelNum.GetStandardDeviation(journalId, dateCreated, LATEST_CITATION_NUM, value);
		feature[featureIdx++] = value;

		//feature 4.a average of top 10 neighbor's label nums
		map<int, Citation*> neighCitations;
		neighbor->GetNeighborCitation(neighCitations, NEIGHBOR_NUM);
		double neiAverage = 0.0;
		double sum = 0.0;
		for (map<int, Citation*>::iterator it = neighCitations.begin(); it != neighCitations.end(); ++it)
			sum += it->second->mNumberMesh;
		if (!neighCitations.empty())
			neiAverage = sum / (double)neighCitations.size();
		else
			neiAverage = AVERAGE_LABEL_NUM;
		feature[featureIdx++] = neiAverage;

		//feature 4.b standard deviation of top 10 neighbor's label nums
		double sum2 = 0.0;
		for (map<int, Citation*>::iterator it = neighCitations.begin(); it != neighCitations.end(); ++it)
			sum2 += ((double)it->second->mNumberMesh - neiAverage) * ((double)it->second->mNumberMesh - neiAverage);
		if (!neighCitations.empty())
			feature[featureIdx++] = sqrt(sum2 / (double)neighCitations.size());
		else
			feature[featureIdx++] = 0.0;
		/**/
		//feature 5.a top K metalabel score
		for (int j = 0; j < METALABEL_SCORE_NUM && j<predictScore[i].size(); ++j)
			feature[featureIdx++] = predictScore[i][j].second;

		//feature 9.a metalabel numlabel
		feature[featureIdx++] = metalabelNum[i];

		//feature 10.a mtidef num
		feature[featureIdx++] = mtidefNum[i];

		//feature 11.a mtidef num
		feature[featureIdx++] = mtiflNum[i];

		//LTR top k score
		for (int j = 0; j < TOP_LTR_SCORE_NUM && j < ltrSortScore[i].size(); ++j)
			feature[featureIdx++] = ltrSortScore[i][j].second / LTR_SCORE_NORMALIZE_NUM;

		//ltr score up threshold num
		for (size_t j = 0; j < ltrThres.size(); ++j)
		{
			int cnt = 0;
			for (size_t k = 0; k < ltrSortScore.size(); ++k)
			{
				if (ltrSortScore[i][k].second >= ltrThres[j])
					++cnt;
				else
					break;
			}
			feature[featureIdx++] = double(cnt) / LTR_COUNT_NORMALIZE_NUM;
		}
	}

	return 0;
}

int ExtractNumlabelFeature(const vector<int>& pmids, vector<vector<pair<int, double>>>& predictScore, double threshold, FeatureSet& featureSet, int printLog)
{
	const int METALABEL_SCORE_NUM = 30000;

	int numThreads = omp_get_num_procs();
	if (printLog != SILENT)
		clog << "CPU number: " << numThreads << endl;
	omp_set_num_threads(numThreads);

	featureSet.mFeatures.clear();
	if (pmids.size() != predictScore.size())
		return -1;

	int rtn = 0;
	featureSet.mFeatures.resize(pmids.size());
#pragma omp parallel for schedule(dynamic) 
	for (int i = 0; i < (int)pmids.size(); ++i)
	{
		sort(predictScore[i].begin(), predictScore[i].end(), CmpScore);
		Feature &feature = featureSet.mFeatures[i];
		int featureIdx = 1;

		//feature 5.a top n metalabel score
		for (int j = 0; j < METALABEL_SCORE_NUM && j < predictScore[i].size(); ++j)
		{
			if (predictScore[i][j].second < threshold)
				break;
			feature[featureIdx++] = predictScore[i][j].second;
		}
	}

	return 0;
}

int ExtractNumlabelFeature(const vector<int>& pmids, CitationSet& citationSet, vector<vector<pair<int, double>>>& predictScore, CitationNeighborSet& neighborSet, JournalLabelNum& journalLabelNum, JournalSet& journalSet/*, FeatureSet& entryTitleFeature, FeatureSet& entryAbstractFeature*/, std::vector<int>& metalabelNum, std::vector<int>& mtidefNum, std::vector<int>& mtiflNum, FeatureSet& featureSet, int printLog)
{
	const int LATEST_CITATION_NUM = 5;
	const int NEIGHBOR_NUM = 10;
	const int METALABEL_SCORE_NUM = 200;
	const int AVERAGE_LABEL_NUM = 13;
	const int TOP_NEIGHBOR_SCORE_NUM = 20;
	const double NORMALIZED_NEIGHBOR_SCORE = 30000000.0;
	//const int TOP_ENTRY_TITLE_FEATURE_NUM = 0;
	//const int TOP_ENTRY_ABSTRACT_FEATURE_NUM = 0;

	int numThreads = omp_get_num_procs();
	if (printLog != SILENT)
		clog << "CPU number: " << numThreads << endl;
	omp_set_num_threads(numThreads);

	featureSet.mFeatures.clear();
	if (pmids.size() != predictScore.size()
		|| pmids.size() != metalabelNum.size()
		|| pmids.size() != mtidefNum.size()
		|| pmids.size() != mtiflNum.size())
		return -1;

	int rtn = 0;
	featureSet.mFeatures.resize(pmids.size());
#pragma omp parallel for schedule(dynamic) 
	for (int i = 0; i < (int)pmids.size(); ++i)
	{
		sort(predictScore[i].begin(), predictScore[i].end(), CmpScore);
		Feature &feature = featureSet.mFeatures[i];
		int featureIdx = 1;


		int journalId = 0;
		Journal* ptrJournal = journalSet.SearchJournalTitle(citationSet[pmids[i]]->mJournalTitle);
		if (ptrJournal != NULL)
			journalId = ptrJournal->mJournalId;
		else
			clog << "Error: Can't find journalTitle:" << citationSet[pmids[i]]->mJournalTitle << endl;
		int dateCreated = citationSet[pmids[i]]->mDateCreated;
		CitationNeighbor* neighbor = neighborSet[pmids[i]];
		if (neighbor == NULL)
			cerr << "Error: pointer neighbor is empty" << endl;

		double value = 0.0;
		//feature 1.a average label num total Journal
		journalLabelNum.GetAverageLabelNum(journalId, value);
		if (value == 0)
			value = AVERAGE_LABEL_NUM;
		feature[featureIdx++] = value;

		//feature 1.b stdDev of total Journal's label num
		journalLabelNum.GetStandardDeviation(journalId, value);
		feature[featureIdx++] = value;

		//feature 2.a average label num in the journal by year
		journalLabelNum.GetAverageLabelNum(journalId, dateCreated, value);
		if (value == 0)
			value = AVERAGE_LABEL_NUM;
		feature[featureIdx++] = value;

		//feature 2.b stdDev of feature 2.a
		journalLabelNum.GetStandardDeviation(journalId, dateCreated, value);
		feature[featureIdx++] = value;

		//feature 3.a average label num in latest 5 citation
		journalLabelNum.GetAverageLabelNum(journalId, dateCreated, LATEST_CITATION_NUM, value);
		if (value == 0)
			value = AVERAGE_LABEL_NUM;
		feature[featureIdx++] = value;

		//feature 3.b stdDev of feature 3.a
		journalLabelNum.GetStandardDeviation(journalId, dateCreated, LATEST_CITATION_NUM, value);
		feature[featureIdx++] = value;

		//feature 4.a average of top 10 neighbor's label nums
		map<int, Citation*> neighCitations;
		neighbor->GetNeighborCitation(neighCitations, NEIGHBOR_NUM);
		double neiAverage = 0.0;
		double sum = 0.0;
		for (map<int, Citation*>::iterator it = neighCitations.begin(); it != neighCitations.end(); ++it)
			sum += it->second->mNumberMesh;
		if (!neighCitations.empty())
			neiAverage = sum / (double)neighCitations.size();
		else
			neiAverage = AVERAGE_LABEL_NUM;
		feature[featureIdx++] = neiAverage;

		//feature 4.b standard deviation of top 10 neighbor's label nums
		double sum2 = 0.0;
		for (map<int, Citation*>::iterator it = neighCitations.begin(); it != neighCitations.end(); ++it)
			sum2 += ((double)it->second->mNumberMesh - neiAverage) * ((double)it->second->mNumberMesh - neiAverage);
		if (!neighCitations.empty())
			feature[featureIdx++] = sqrt(sum2 / (double)neighCitations.size());
		else
			feature[featureIdx++] = 0.0;
		/**/
		//feature 5.a top K metalabel score
		for (int j = 0; j < METALABEL_SCORE_NUM && j<predictScore[i].size(); ++j)
			feature[featureIdx++] = predictScore[i][j].second;
		/*
		set<int> entryMeshs;
		//feature 6.a entry title feature num
		int entryTilteFeatureNum = 0;
		for (Feature::iterator it = entryTitleFeature[i].begin(); it != entryTitleFeature[i].end(); ++it)
		if (it->second > 0.0)
		{
		++entryTilteFeatureNum;
		entryMeshs.insert(it->first);
		}
		//feature[featureIdx++] = entryTilteFeatureNum;

		//feature 6.b top K entry title feature score
		vector<double> entryTitleScore;
		for (Feature::iterator it = entryTitleFeature[i].begin(); it != entryTitleFeature[i].end(); ++it)
		entryTitleScore.push_back(it->second);
		sort(entryTitleScore.begin(), entryTitleScore.end(),CmpDataLarger<double>);
		for (int j = 0; j<TOP_ENTRY_TITLE_FEATURE_NUM; ++j)
		{
		if (j < (int)entryTitleScore.size())
		feature[featureIdx++] = entryTitleScore[j];
		else
		feature[featureIdx++] = 0.0;
		}

		//feature 7.a entry title feature num
		int entryAbstractFeatureNum = 0;
		for (Feature::iterator it = entryAbstractFeature[i].begin(); it != entryAbstractFeature[i].end(); ++it)
		if (it->second > 0.0)
		{
		++entryAbstractFeatureNum;
		entryMeshs.insert(it->first);
		}
		//feature[featureIdx++] = entryAbstractFeatureNum;

		//feature 7.b top K entry abstract feature score
		vector<double> entryAbstractScore;
		for (Feature::iterator it = entryAbstractFeature[i].begin(); it != entryAbstractFeature[i].end(); ++it)
		entryAbstractScore.push_back(it->second);
		sort(entryAbstractScore.begin(), entryAbstractScore.end(), CmpDataLarger<double>);
		for (int j = 0; j<TOP_ENTRY_ABSTRACT_FEATURE_NUM; ++j)
		{
		if (j < (int)entryAbstractScore.size())
		feature[featureIdx++] = entryAbstractScore[j];
		else
		feature[featureIdx++] = 0.0;
		}

		//feature 8.a entry map entry num, include title and abstract
		//feature[featureIdx++] = (int)entryMeshs.size();
		*/
		//feature 9.a metalabel numlabel
		feature[featureIdx++] = metalabelNum[i];

		//feature 10.a mtidef num
		feature[featureIdx++] = mtidefNum[i];

		//feature 11.a mtidef num
		feature[featureIdx++] = mtiflNum[i];

		//feature 12.a top K average neighbor score
		/*double sumScore = 0.0;
		neighbor->GetValidScoreCount(TOP_NEIGHBOR_SCORE_NUM, sumScore);
		double averageNeiboScore = sumScore / double(TOP_NEIGHBOR_SCORE_NUM);
		feature[featureIdx++] = averageNeiboScore / NORMALIZED_NEIGHBOR_SCORE;*/
	}

	return 0;
}

int InitializePrecisionScoreTableSet(string sourPredictScore, string tokenDocFile, string tablePath)
{
	int rtn = 0;

	clog << "Load LhtcDocumentSet" << endl;
	LhtcDocumentSet docSet;
	rtn = docSet.LoadBin(tokenDocFile.c_str(), STATUS_ONLY);
	CHECK_RTN(rtn);

	clog << "Get goldstandard" << endl;
	map<int, set<int>> pmidMesh;
	for (map<int, LhtcDocument>::iterator it = docSet.mLhtcDocuments.begin(); it != docSet.mLhtcDocuments.end(); ++it)
	{
		set<int> &st = pmidMesh[it->first];
		for (size_t i = 0; i < it->second.mLabels.size(); ++i)
			st.insert(it->second.mLabels[i]);
	}

	clog << "Begin add table" << endl;
	FILE* inFile = fopen(sourPredictScore.c_str(), "rb");
	if (inFile == NULL)
		return -1;

	size_t len;
	rtn = Read(inFile, len);
	clog << "Total " << len << " lines" << endl;
	ScoreTableSet scoreTable;
	for (size_t i = 0; i < len; ++i)
	{
		if (((int)i & 255) == 0)
			clog << "load " << i << " line" << endl;
		pair<int, vector<pair<int, double>>> modelScore;
		rtn = Read(inFile, modelScore);
		CHECK_RTN(rtn);
		pair<int, vector<pair<double, bool>>> labelScore;
		labelScore.first = modelScore.first;
		labelScore.second.resize(modelScore.second.size());
		for (size_t j = 0; j < modelScore.second.size(); ++j)
		{
			int pmid = modelScore.second[j].first;
			int meshId = modelScore.first;

			if (pmidMesh[pmid].count(meshId) > 0)
				labelScore.second[j] = make_pair(modelScore.second[j].second, true);
			else
				labelScore.second[j] = make_pair(modelScore.second[j].second, false);
		}
		rtn = scoreTable.AddPrecisionTable(labelScore);
		CHECK_RTN(rtn);
	}
	fclose(inFile);
	clog << "Begin Save" << endl;
	rtn = scoreTable.Save(tablePath, FULL_LOG);
	CHECK_RTN(rtn);
	clog << "Total get " << scoreTable.Size() << " tables" << endl;
	clog << "PrecisionScoreTable initialized completed" << endl;
	return 0;
}

int InitializeRecallScoreTableSet(string sourPredictScore, string citationFile, string meshFile, string tablePath)
{
	int rtn = 0;

	clog << "Load meshs" << endl;
	MeshRecordSet meshRecords;
	rtn = meshRecords.Load(meshFile.c_str());
	CHECK_RTN(rtn);

	clog << "Load citations" << endl;
	CitationSet citationSet;
	rtn = citationSet.Load(citationFile.c_str(), STATUS_ONLY);
	CHECK_RTN(rtn);

	clog << "Get goldstandard" << endl;
	map<int, set<int>> pmidMesh;
	for (map<int, Citation*>::iterator it = citationSet.mCitations.begin(); it != citationSet.mCitations.end(); ++it)
	{
		set<int> &st = pmidMesh[it->first];
		for (size_t j = 0; j < it->second->mNumberMesh; ++j)
			st.insert(meshRecords[it->second->mMeshHeadingList[j].mDescriptorName.mText]->mUid);
	}

	clog << "Begin add table" << endl;
	FILE* inFile = fopen(sourPredictScore.c_str(), "rb");
	if (inFile == NULL)
		return -1;

	size_t len;
	rtn = Read(inFile, len);
	clog << "Total " << len << " lines" << endl;
	ScoreTableSet scoreTable;
	for (size_t i = 0; i < len; ++i)
	{
		if (((int)i & 255) == 0)
			clog << "load " << i << " line" << endl;
		pair<int, vector<pair<int, double>>> modelScore;
		rtn = Read(inFile, modelScore);
		CHECK_RTN(rtn);
		pair<int, vector<pair<double, bool>>> labelScore;
		labelScore.first = modelScore.first;
		labelScore.second.resize(modelScore.second.size());
		for (size_t j = 0; j < modelScore.second.size(); ++j)
		{
			int pmid = modelScore.second[j].first;
			int meshId = modelScore.first;

			if (pmidMesh[pmid].count(meshId) > 0)
				labelScore.second[j] = make_pair(modelScore.second[j].second, true);
			else
				labelScore.second[j] = make_pair(modelScore.second[j].second, false);
		}
		rtn = scoreTable.AddRecallTable(labelScore);
		CHECK_RTN(rtn);
	}
	fclose(inFile);
	clog << "Begin Save" << endl;
	rtn = scoreTable.Save(tablePath, FULL_LOG);
	CHECK_RTN(rtn);
	clog << "Total get " << scoreTable.Size() << " tables" << endl;
	clog << "RecallScoreTable initialized completed" << endl;
	return 0;
}

int InitializeThreshold(string sourPredictScore, string citationFile, string meshFile, string tablePath)
{
	int rtn = 0;

	clog << "Load meshs" << endl;
	MeshRecordSet meshRecords;
	rtn = meshRecords.Load(meshFile.c_str());
	CHECK_RTN(rtn);

	clog << "Load citations" << endl;
	CitationSet citationSet;
	rtn = citationSet.Load(citationFile.c_str(), STATUS_ONLY);
	CHECK_RTN(rtn);

	clog << "Get goldstandard" << endl;
	map<int, set<int>> pmidMesh;
	for (map<int, Citation*>::iterator it = citationSet.mCitations.begin(); it != citationSet.mCitations.end(); ++it)
	{
		set<int> &st = pmidMesh[it->first];
		for (size_t j = 0; j < it->second->mNumberMesh; ++j)
			st.insert(meshRecords[it->second->mMeshHeadingList[j].mDescriptorName.mText]->mUid);
	}

	clog << "Begin add table" << endl;
	FILE* inFile = fopen(sourPredictScore.c_str(), "rb");
	if (inFile == NULL)
		return -1;

	size_t len;
	rtn = Read(inFile, len);
	clog << "Total " << len << " lines" << endl;
	Threshold threshold;
	for (size_t i = 0; i < len; ++i)
	{
		if (((int)i & 255) == 0)
			clog << "load " << i << " line" << endl;
		pair<int, vector<pair<int, double>>> modelScore;
		rtn = Read(inFile, modelScore);
		CHECK_RTN(rtn);
		pair<int, vector<pair<double, bool>>> labelScore;
		labelScore.first = modelScore.first;
		labelScore.second.resize(modelScore.second.size());
		for (size_t j = 0; j < modelScore.second.size(); ++j)
		{
			int pmid = modelScore.second[j].first;
			int meshId = modelScore.first;

			if (pmidMesh[pmid].count(meshId) > 0)
				labelScore.second[j] = make_pair(modelScore.second[j].second, true);
			else
				labelScore.second[j] = make_pair(modelScore.second[j].second, false);
		}
		rtn = threshold.AddInstanceByMaxFscore(labelScore);
		CHECK_RTN(rtn);
	}
	fclose(inFile);
	clog << "Begin Save" << endl;
	rtn = threshold.Save(tablePath, FULL_LOG);
	CHECK_RTN(rtn);
	clog << "Total get " << threshold.Size() << " model threshold" << endl;
	clog << "Threshold initialized completed" << endl;
	return 0;
}

int SaveLtrFeature(const char* fileName, vector<double>& labels, vector<int>& qids, vector<Feature>& features, vector<string>* info)
{
	FILE *outFile = fopen(fileName, "w");
	for (size_t i = 0; i < features.size(); ++i)
	{
		fprintf(outFile, "%lf qid:%d", labels[i], qids[i]);
		for (Feature::iterator it = features[i].begin(); it != features[i].end(); ++it)
		{
			//if (it->second > EPS)
			fprintf(outFile, " %d:%lf", it->first, it->second);
			//else
			//	fprintf(outFile, " %d:%lf", it->first, 0.0);
		}

		if (info != NULL)
		{
			fprintf(outFile, " #%s", (*info)[i].c_str());
		}
		fprintf(outFile, "\n");
	}
	fclose(outFile);
	return 0;
}

int LoadLtrFeature(const char* fileName, vector<double>& labels, vector<int>& qids, vector<Feature>& features, vector<string>* info)
{
	ifstream fin(fileName);
	string line;
	while (getline(fin, line))
	{
		string annotation;
		size_t pos = line.find("#");
		if (pos != line.npos)
			annotation = line.substr(pos + 1);
		string featureStr;
		featureStr = line.substr(0, pos);
		istringstream sin(featureStr);
		double label;
		sin >> label;
		char ch;
		while (sin >> ch)
		{
			if (ch == ':')
				break;
		}
		int qid;
		sin >> qid;
		Feature feature;
		int index;
		double value;
		while (sin >> index >> ch >> value)
		{
			feature[index] = value;
		}
		labels.push_back(label);
		qids.push_back(qid);
		features.push_back(feature);
		if (info != NULL)
			info->push_back(annotation);
	}
	fin.close();
	return 0;
}

int LoadLtrToolScore(const char* fileName, vector<double>& scores)
{
	FILE *inFile = fopen(fileName, "r");
	double tmp;
	scores.clear();
	while (fscanf(inFile, "%*d%*d%lf", &tmp) != EOF)
		scores.push_back(tmp);
	fclose(inFile);
	return 0;
}

int SaveLtrToolScore(const char* fileName, std::vector<double>& scores)
{
	FILE *outFile = fopen(fileName, "w");
	for (size_t i = 0; i < scores.size(); ++i)
		fprintf(outFile, "0 0 %lf\n", scores[i]);
	fclose(outFile);
	return 0;
}

int SavePmidMeshNumlabel(const char* fileName, vector<int>& pmids, vector<int>& meshIds, map<int, int>& numLabels)
{
	FILE *outFile = fopen(fileName, "w");
	if (pmids.size() != meshIds.size())
		return -1;
	for (size_t i = 0; i < pmids.size(); ++i)
	{
		fprintf(outFile, "%d %d %d\n", pmids[i], meshIds[i], numLabels[pmids[i]]);
	}
	fclose(outFile);
	return 0;
}

int LoadPmidMeshNumlabel(const char* fileName, vector<int>& pmids, vector<int>& meshIds, map<int, int>& numLabels)
{
	FILE *inFile = fopen(fileName, "r");
	if (inFile == NULL)
		return -1;
	pmids.clear();
	meshIds.clear();
	numLabels.clear();

	int pmid, meshId, num;
	int lastPmid = -1;
	while (fscanf(inFile, "%d%d%d", &pmid, &meshId, &num) != EOF)
	{
		pmids.push_back(pmid);
		meshIds.push_back(meshId);
		if (pmid != lastPmid)
			numLabels[pmid] = num;
		lastPmid = pmid;
	}
	fclose(inFile);
	return 0;
}
