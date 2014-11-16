#ifndef MYSVM_H
#define MYSVM_H
#include "common/common_basic.h"
#include "libsvm/svm.h"



class SVM_Machine
{
public:
	SVM_Machine();
	~SVM_Machine();
	SVM_Machine(std::string modelFilePath);

	int Load(std::string modelFilePath);

	int Save(std::string modelFilePath);

	int Train(std::vector<double>& labels, std::vector<std::map<int, double> >& features, int type = C_SVC, int kernel = RBF, double c = 1, double g = 0);

	int Predict(std::map<int, double>& feature, double& score);

	int Predict(std::vector<std::map<int, double> >& features, std::vector<double>& scores);

	static int OutputFeature(std::vector<double>& labels, std::vector<std::map<int, double> >& features, std::string filePath);

private:
	/* 0 for empty
	* 1 for stored model in memory
	*/
	int mState;
	svm_model* mModel;
	svm_parameter* mParam;
	svm_node *mX_space;
	svm_problem mProb;

	int TransFeatures(std::vector<double>& labels, std::vector<std::map<int, double> >& features);

};

#endif //MYSVM_H