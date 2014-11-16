#include "metalabelnewtrain_lhtsc.h"
#include <string>
using namespace std;



int SaveLabelFreq(LhtcDocumentSet& lshtcSet, string freqFileName)
{
	int rtn = 0;
//	LhtcDocumentSet lshtcSet;
//	rtn = lshtcSet.Load(lshtcFile.c_str(), FULL_LOG);//load lhtcdocumentset
//	CHECK_RTN(rtn);

	map<int, int> labelCnt;
	for (std::map<int, LhtcDocument>::iterator iter = lshtcSet.mLhtcDocuments.begin(); iter != lshtcSet.mLhtcDocuments.end(); ++iter)
	{
		int labelSize = (int)iter->second.mLabels.size();
		for (int i = 0; i < labelSize; i++)
		{
			int tempLabel = iter->second.mLabels[i];
			if (labelCnt.count(tempLabel) == 0) labelCnt[tempLabel] = 0;
			labelCnt[tempLabel] += 1;//freq = appear / tot;
		}
	}
	
	map<int, double> labelFreq;
	double totDocument = lshtcSet.mLhtcDocuments.size();
	for (map<int, int>::iterator it = labelCnt.begin(); it != labelCnt.end(); ++it)
	{
		labelFreq[it->first] = it->second / totDocument;
	}
	rtn = SaveLabelFreq(freqFileName, labelFreq);
	CHECK_RTN(rtn);

	return 0;
}

int SavePmidMesh(LhtcDocumentSet& lshtcSet, string instanceIdLabelIdFile)
{
	int rtn = 0;
	//LhtcDocumentSet lshtcSet;
	//rtn = lshtcSet.Load(lshtcFile.c_str(), FULL_LOG);
	CHECK_RTN(rtn);

	std::vector<std::pair<int, std::set<int> > >  pmidMeshs;
	pmidMeshs.clear();
	std::vector<int> toShuffle;
	toShuffle.clear();
	int totDocument = lshtcSet.Size();
	for (int i = 0; i < totDocument; i++) toShuffle.push_back(i);
	std::random_shuffle(toShuffle.begin(), toShuffle.end());//get shuffle sequence

	vector<LhtcDocument*> vecDocument;
	for (map<int, LhtcDocument>::iterator it = lshtcSet.mLhtcDocuments.begin(); it != lshtcSet.mLhtcDocuments.end(); ++it)
		vecDocument.push_back(&(it->second));
	for (size_t i = 0; i < toShuffle.size(); i++)
	{
		std::set<int> tempSet;
		int labelSize = vecDocument[toShuffle[i]]->mLabels.size();
		for (int j = 0; j < labelSize; j++)
			tempSet.insert(vecDocument[toShuffle[i]]->mLabels[j]);
		pmidMeshs.push_back(std::make_pair(vecDocument[toShuffle[i]]->mId, tempSet));
	}

	FILE* outFile = fopen(instanceIdLabelIdFile.c_str(), "wb");
	rtn = Write(outFile, pmidMeshs);
	CHECK_RTN(rtn);

	return 0;
}

int ExtractFeature(const vector<LhtcDocument*>& tokenVector, UniGramFeature& uniGrams, feature_node** &featureSpace, int printLog)
{
	int numThreads = omp_get_num_procs();
	if (printLog != SILENT)
		clog << "CPU number: " << numThreads << endl;
	omp_set_num_threads(numThreads);
	if (printLog != SILENT)
		clog << "Extract unigram" << endl;

	if (printLog != SILENT)
		clog << "Make Feature table" << endl;
	int featureNum = (int)tokenVector.size();
	featureSpace = NULL;
	featureSpace = Malloc(feature_node*, featureNum);
	memset(featureSpace, 0, sizeof(feature_node*)* featureNum);

	//int uniMaxIndex = uniGrams.mDictionary.rbegin()->first + 1;

	if (printLog != SILENT)
		clog << "Extract features parallel" << endl;

#pragma omp parallel for schedule(dynamic)
	for (int i = 0; i < (int)tokenVector.size(); i++)
	{
		FeatureSet tabAllFeatures;
		tabAllFeatures.mFeatures.resize(1);

		uniGrams.ExtractLhtc(*tokenVector[i], tabAllFeatures.mFeatures[0]);
		tabAllFeatures.Normalize();

		featureSpace[i] = NULL;
		LinearMachine::TransFeatures(featureSpace[i], tabAllFeatures.mFeatures[0]);
	}
	return 0;
}

int SaveLhtcFeature(LhtcDocumentSet& lshtcSet, string outFeatureFile, std::string instanceIdLabelIdFile, string unigramFile)
{
	int rtn = 0;
	clog << "Extract feature" << endl;
	feature_node** featureSpace = NULL;

	clog << "Load validtrain sorted pmid meshs" << endl;
	vector<pair<int, set<int>>> vecMeshs;
	FileBuffer buffer(instanceIdLabelIdFile.c_str());
	rtn = buffer.GetNextData(vecMeshs);
	CHECK_RTN(rtn);
	clog << "Total load " << vecMeshs.size() << " pmids" << endl;

	vector<LhtcDocument*> vecDocument;
	for (size_t i = 0; i < vecMeshs.size(); ++i)
		vecDocument.push_back(&(lshtcSet.mLhtcDocuments[vecMeshs[i].first]));

	UniGramFeature unigram;
	//rtn = unigram.BuildLhtc(lshtcSet);
	//CHECK_RTN(rtn);

	rtn = unigram.Load(unigramFile.c_str());
	CHECK_RTN(rtn);
	clog << "Load " << unigram.mDictionary.size() << " unigram words" << endl;

	rtn = ExtractFeature(vecDocument, unigram, featureSpace, FULL_LOG);
	CHECK_RTN(rtn);

	clog << "Save metalabel train feature" << endl;
	rtn = LinearMachine::SaveFeatureNode(featureSpace, vecDocument.size(), outFeatureFile);
	CHECK_RTN(rtn);
	clog << "MetalabelNewTrainFeature Save completed" << endl;
	clog << "Free memory" << endl;
	for (size_t i = 0; i < vecDocument.size(); ++i)
		SmartFree(featureSpace[i]);
	SmartFree(featureSpace);

	return 0;
}

int BuildUnigramDictionary(LhtcDocumentSet& lshtcSet, std::string unigramFile)
{
	int rtn = 0;
	UniGramFeature unigram;
	rtn = unigram.BuildLhtc(lshtcSet);
	CHECK_RTN(rtn);

	rtn = unigram.Save(unigramFile.c_str());
	CHECK_RTN(rtn);
	clog << "Save " << unigram.mDictionary.size() << " unigram words" << endl;

	return 0;
}

int MetaLabelNumModelTrain(const string tokenFile, const string unigramFile, const string outModelPath)
{
	int rtn = 0;
	clog << "Loading Tokenization Result" << endl;
	LhtcDocumentSet tokenDocuments;
	rtn = tokenDocuments.LoadBin(tokenFile.c_str(), STATUS_ONLY);
	CHECK_RTN(rtn);

	clog << "Load Unigram Dictionary" << endl;
	UniGramFeature uniGrams;
	rtn = uniGrams.Load(unigramFile.c_str());
	CHECK_RTN(rtn);
	clog << "Total " << uniGrams.mDictionary.size() << " unigrams" << endl;

	vector<int> meshNum;
	vector<LhtcDocument*> tokenDocVector;
	tokenDocVector.reserve(tokenDocuments.Size());
	for (map<int, LhtcDocument>::iterator it = tokenDocuments.mLhtcDocuments.begin(); it != tokenDocuments.mLhtcDocuments.end(); it++)
	{
		meshNum.push_back((int)it->second.mLabels.size());
		tokenDocVector.push_back(&(it->second));
	}

	clog << "Prepare for Extract Features" << endl;
	FeatureSet allFeatures;
	allFeatures.mFeatures.resize(tokenDocVector.size());
#pragma omp parallel for schedule(dynamic)
	for (int i = 0; i < (int)tokenDocVector.size(); i++)
	{
		uniGrams.ExtractLhtc(*tokenDocVector[i], allFeatures.mFeatures[i]);
	}
	allFeatures.Normalize();

	vector<double> labels;
	labels.reserve(meshNum.size());
	for (size_t i = 0; i < meshNum.size(); i++)
		labels.push_back((double)meshNum[i]);

	LinearMachine machine;
	rtn = machine.Train(labels, allFeatures.mFeatures, L2R_L2LOSS_SVR);
	CHECK_RTN(rtn);
	clog << "Save Num Label Model to " + outModelPath << endl;
	rtn = machine.Save(outModelPath);
	CHECK_RTN(rtn);
	rtn = machine.Destroy();
	CHECK_RTN(rtn);

	clog << "Numlabel model train completed" << endl;
	return 0;
}

