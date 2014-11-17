#include "medline/basic.h"
#include "medline/citation.h"
#include "medline/mesh.h"
#include "extractfeature/feature.h"
#include "classifier/classifier.h"
#include "evaluation/tools.h"
#include "tokenization/tokenization.h"
#include "metalabelnewtrain_lhtsc.h"
#include <omp.h>
#include <algorithm>
using namespace std;

const string gUnigramFile = "lshtc_unigram_dictionary_loctrain.bin";
const string gLabelFreqFile = "lshtc_label_freqence.bin";//"../medline2014/zip/models/label_freq.bin"
const string gPmidMeshFile = "lshtc_sorted_id_labels_file_loctrain.bin";//_tmp
const string gNewTrainFeatureFile = "lshtc_sorted_loctrain_feature.bin";

int SaveSample(map<int, vector<int> >& samp, const char* fileName)
{
	FILE *outFile = fopen(fileName, "wb");
	Write(outFile, (int)samp.size());
	for (map<int, vector<int> >::iterator it = samp.begin(); it != samp.end(); ++it)
	{
		Write(outFile, it->first);
		Write(outFile, (int)it->second.size());
		for (unsigned i = 0; i < it->second.size(); ++i)
			Write(outFile, it->second[i]);
	}
	fclose(outFile);
	return 0;
}

int LoadSample(map<int, vector<int> >& samp, const char* fileName)
{
	samp.clear();
	FileBuffer buffer(fileName);
	int len;
	buffer.GetNextData(len);
	for (int i = 0; i < len; ++i)
	{
		int meshId;
		buffer.GetNextData(meshId);
		vector<int> &vec = samp[meshId];
		int vecSize;
		buffer.GetNextData(vecSize);
		for (int j = 0; j < vecSize; ++j)
		{
			int tmp;
			buffer.GetNextData(tmp);
			vec.push_back(tmp);
		}
	}
	return 0;
}

int MetalabelTrainModel(const string featureFile, const string pmidMeshFile, const string labelFreqFile, const int trainModelNum, const int posCitationNum, const int posSampleNum, const int negCitationNum, int type, double c, bool bSaveMidData = false, const char* const modelPath = "newmodels")
{
	const int CITATION_STARTPOS = 0;
	int rtn = 0;

	clog << "Load features" << endl;
	size_t featureSize;
	feature_node** featureSpace = NULL;
	feature_node* nodeSpace = NULL;
	rtn = LinearMachine::LoadFeatureNode(featureSpace, nodeSpace, featureSize, featureFile);
	CHECK_RTN(rtn);
	clog << "Load " << featureSize << " train features" << endl;

	clog << "Load validtrain sorted pmid meshs" << endl;
	vector<pair<int, set<int>>> vecMeshs;
	FileBuffer buffer(pmidMeshFile.c_str());
	rtn = buffer.GetNextData(vecMeshs);
	CHECK_RTN(rtn);
	clog << "Total load " << vecMeshs.size() << " pmids" << endl;

	clog << "Load Label Frequence" << endl;
	map<int, double> labelFreq;
	rtn = LoadLabelFreq(labelFreqFile.c_str(), labelFreq);
	CHECK_RTN(rtn);

	vector<pair<int, double> > meshSort;
	for (map<int, double>::iterator it = labelFreq.begin(); it != labelFreq.end(); ++it)
		meshSort.push_back(make_pair(it->first, it->second));
	sort(meshSort.begin(), meshSort.end(), CmpScore);

	vector<int> trainMeshIdVector;
	for (int i = 0; i < trainModelNum && i < (int)meshSort.size(); ++i)
	{
		trainMeshIdVector.push_back(meshSort[i].first);
	}

	/*//test   add test model id
	vector<int> ranks = { 1, 3, 7, 10, 15, 20, 30, 50, 60, 65, 70, 80, 90, 95, 100, 200, 300, 500, 600, 800, 900
	,1000,1500, 2000,2500,3000,3500, 4000,4500,5000,5500,6000,6500,7000,7500, 8000,8500, 9000,9500,10000, 12000 };
	for (size_t i = 0; i < ranks.size(); ++i)
	{
	trainMeshIdVector.push_back(meshSort[ranks[i] - 1].first);
	}
	for (size_t rank = 1000; rank <= 10000; rank += 250)
	trainMeshIdVector.push_back(meshSort[rank - 1].first);*/
	///test end

	map<int, int> pmidIndex;
	for (int i = 0; i < (int)vecMeshs.size(); ++i)
		pmidIndex[vecMeshs[i].first] = i;

	int numThreads = omp_get_num_procs();
	clog << "CPU number: " << numThreads << endl;
	omp_set_num_threads(numThreads/2);

	vector<int> vecPosNum, vecNegNum;
	vecPosNum.resize(trainMeshIdVector.size());
	vecNegNum.resize(trainMeshIdVector.size());
	clog << "Begin train " << trainMeshIdVector.size() << " models" << endl;
	cout << "Begin train " << trainMeshIdVector.size() << " models" << endl;
#pragma omp parallel for schedule(dynamic)
	for (int i = 0; i < (int)trainMeshIdVector.size(); ++i)
	{
		if ((i & 255) == 0)
			clog << "\r" << i << " new model training";
		int meshId = trainMeshIdVector[i];
		string modelFilePath = modelPath + ("/" + intToString(meshId) + ".model");
		if (FileExist(modelFilePath))
			clog << "Warning: model file \"" << modelFilePath << "\" already exists!" << endl;
		else
		{
			vector<double> labels;
			vector<int> posInstance;
			vector<int> negInstance;
			negInstance.reserve(negCitationNum);
			posInstance.reserve(posSampleNum);
			for (unsigned k = CITATION_STARTPOS; k < vecMeshs.size() && (k < size_t(CITATION_STARTPOS + posCitationNum) || posInstance.size() < (size_t)posSampleNum); ++k)
			{
				if (vecMeshs[k].second.count(meshId) > 0)
					posInstance.push_back(vecMeshs[k].first);
			}
			for (unsigned k = CITATION_STARTPOS; k < vecMeshs.size() && k < size_t(CITATION_STARTPOS + negCitationNum); ++k)
			{
				if (vecMeshs[k].second.count(meshId) == 0)
					negInstance.push_back(vecMeshs[k].first);
			}

			vecPosNum[i] = (int)posInstance.size();
			vecNegNum[i] = (int)negInstance.size();

			int len = (int)negInstance.size() + (int)posInstance.size();
			feature_node** x = Malloc(feature_node*, len);
			for (size_t k = 0; k < negInstance.size(); ++k)
			{
				if (pmidIndex.count(negInstance[k]) == 0)
					cout << "Warning: Can't find neg instance' pmid " << negInstance[k] << " in pmidIndex" << endl;
				else
				{
					x[labels.size()] = featureSpace[pmidIndex[negInstance[k]]];
					labels.push_back(-1.0);
				}
			}

			for (size_t k = 0; k < posInstance.size(); ++k)
			{
				if (pmidIndex.count(posInstance[k]) == 0)
					cout << "Warning: Can't find pos instance' pmid " << posInstance[k] << " in pmidIndex" << endl;
				else
				{
					x[labels.size()] = featureSpace[pmidIndex[posInstance[k]]];
					labels.push_back(1.0);
				}
			}

			LinearMachine machine;
			machine.Train(labels, x, type, c);
			machine.Save(modelFilePath);
			machine.Destroy();
			SmartFree(x);
		}
	}
	clog << endl;
	/*
	//map < int, double> aupr;
	//rtn = Evaluator::LoadAupr(aupr, "newmodelaupr.bin");
	//CHECK_RTN(rtn);

	clog << "Load Mesh" << endl;
	MeshRecordSet meshRecords;
	rtn = meshRecords.Load(gMeshRecordFile.c_str());
	CHECK_RTN(rtn);

	cout << "Saving trainmodelsamplenum.csv" << endl;
	FILE *outFile = fopen("trainmodelsamplenum.csv", "w");
	fprintf(outFile, "meshId,meshName,pos,neg\n");//,aupr
	for (int i = 0; i < (int)trainMeshIdVector.size(); ++i)
	{
	int meshId = trainMeshIdVector[i];
	fprintf(outFile, "%d,\"%s\",%d,%d\n", meshId, meshRecords[meshId]->mName, vecPosNum[i], vecNegNum[i]);
	}
	fclose(outFile);*/

	clog << "Train Complete" << endl;
	cout << "Train Complete" << endl;
	rtn = SmartFree(featureSpace);
	CHECK_RTN(rtn);
	rtn = SmartFree(nodeSpace);
	CHECK_RTN(rtn);
	return 0;
}

/////shecme 1, now use
int MetaLabelNewTrain1(const string featureFile, const string pmidMeshFile, const string labelFreqFile, const int trainModelNum, const int posCitationNum, const int posSampleNum, const int negCitationNum, bool bSaveMidData = false, const char* const modelPath = "newmodels")
{
	int rtn = 0;
	rtn = MetalabelTrainModel(featureFile, pmidMeshFile, labelFreqFile, trainModelNum, posCitationNum, posSampleNum, negCitationNum, L2R_LR, 1, bSaveMidData, modelPath);//L2R_LR L2R_L1LOSS_SVC_DUAL L2R_L2LOSS_SVC_DUAL
	CHECK_RTN(rtn);
	return 0;
}

int main()
{
	const int MODLE_NUM = 30000;
	const int POS_CITATION_NUM = 1000000;
	const int NEG_CITATION_NUM = 1000000;
	const int POS_SAMPLE_NUM = 50000;
	const int NEG_SAMPLE_NUM = 500000;
	int rtn = 0;

	LhtcDocumentSet lshtcSet;
	//rtn = lshtcSet.LoadBin("../data/loc_train.bin", FULL_LOG);//load lhtcdocumentset
	CHECK_RTN(rtn);

	//rtn = SaveLabelFreq(lshtcSet, "lshtc_label_freqence.bin");
	CHECK_RTN(rtn);
	//clog << "Save label freq completed" << endl;

	//rtn = SavePmidMesh(lshtcSet, "lshtc_sorted_id_labels_file_loctrain.bin");
	CHECK_RTN(rtn);
	//clog << "Save pmid mesh completed" << endl;

	//rtn = SaveLhtcFeature(lshtcSet, "lshtc_sorted_loctrain_feature.bin", "lshtc_sorted_id_labels_file_loctrain.bin");
	CHECK_RTN(rtn);
	//clog << "Save features completed" << endl;

	clog << "Metalabel models train" << endl;
	rtn = MetaLabelNewTrain1(gNewTrainFeatureFile, gPmidMeshFile, gLabelFreqFile, MODLE_NUM, POS_CITATION_NUM, POS_SAMPLE_NUM, NEG_CITATION_NUM, false, "../models_1109");//testmodels/train_parameters/L2R_L2LOSS_SVC_DUAL
	CHECK_RTN(rtn);

	//clog << "Metalabel number model train" << endl;
	//rtn = MetaLabelNumModelTrain("../data/loc_train.bin", gUnigramFile, "../models_1109/numlabel_1109.model");
	clog << "Complete" << endl;
	return 0;
}
