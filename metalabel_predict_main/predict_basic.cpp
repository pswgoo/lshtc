#include "predict_basic.h"
using namespace std;

int Evaluate(MultiLabelAnswerSet& goldStandard, MultiLabelAnswerSet& predictAnswers, std::string resultFile)
{
	clog << "Begin Evaluation" << endl;
	double precision = 0.0;
	double recall = 0.0;
	double f1 = 0.0;

	FILE *outEvaluate = fopen(resultFile.c_str(), "w");
	Evaluator evaluator;
	evaluator.LabelBasedMicroEvaluate(predictAnswers, goldStandard, precision, recall, f1);
	printf("micro: MIP=%lf,MIR=%lf,MIF=%lf\n", precision, recall, f1);
	fprintf(outEvaluate, "micro: MIP=%lf,MIR=%lf,MIF=%lf\n", precision, recall, f1);

	evaluator.ExampleBasedEvaluate(predictAnswers, goldStandard, precision, recall, f1, "./lshtc_example_analyse.csv");
	printf("examp: EBP=%lf,EBR=%lf,EBF=%lf\n", precision, recall, f1);
	fprintf(outEvaluate, "examp: EBP=%lf,EBR=%lf,EBF=%lf\n", precision, recall, f1);

	evaluator.LabelBasedMacroEvaluate(predictAnswers, goldStandard, precision, recall, f1, "./lshtc_model_analyse.csv");//, "./ltr_model_analyse.csv"
	printf("MAP=%lf,MAR=%lf,MAF=%lf\n", precision, recall, f1);
	fprintf(outEvaluate, "MAP=%lf,MAR=%lf,MAF=%lf\n", precision, recall, f1);
	fclose(outEvaluate);

	clog << "Evaluation complete" << endl;
	return 0;
}

int NumlabelPredict(std::string modelFile, std::string testFile, std::string unigramFile, map<int, int>& numlabel)
{
	int rtn = 0;
	clog << "Loading Tokenization Result" << endl;
	LhtcDocumentSet tokenDocuments;
	rtn = tokenDocuments.LoadBin(testFile.c_str(), STATUS_ONLY);
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
	labels.resize(tokenDocuments.Size());
	LinearMachine machine;
	clog << "Save Num Label Model to " + modelFile << endl;
	rtn = machine.Load(modelFile);
	CHECK_RTN(rtn);
	for (int i = 0; i < tokenDocuments.Size(); ++i)
		machine.Predict(allFeatures.mFeatures[i], labels[i]);
	
	for (size_t i = 0; i < labels.size(); ++i)
	{
		if (labels[i] < 1.0)
			labels[i] = 1.0;
		numlabel[tokenDocVector[i]->mId] = (int)round(labels[i]);
	}
	clog << "Precit num label completed" << endl;
	return 0;
}
