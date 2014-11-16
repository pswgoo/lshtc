#ifndef _MYLINEAR_H
#define _MYLINEAR_H 

#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <string>
#include <algorithm>
#include <map>
#include <vector>
#include <set>
#include "liblinear/linear.h"
#include "liblinear/tron.h"

typedef std::map<int, double> Feature;

class LinearMachine
{
public:
	LinearMachine();
	~LinearMachine();
	LinearMachine(std::string modelFilePath);

	int Destroy();

	int Load(std::string modelFilePath);

	int Save(std::string modelFilePath);

	int Train();
	//please complete
	int Train(std::vector<double>&labels, feature_node** &x, int type = L2R_LR, double c = 1, double bias = -1);

	int Train(std::vector<double>& labels, std::vector<Feature>& features, int type = L2R_LR, double c = 1, double bias = -1);

	int TrainRank(std::vector<double>& labels, std::vector<Feature>& features, std::vector<int> queries, int type = L2R_L2LOSS_RANKSVM, double c = 1, double bias = -1);

	int Predict(Feature& feature, double& score);

	int Predict(feature_node* x_space, double& score);

	int Predict(std::vector<Feature>& features, std::vector<double>& scores);

	static int TransFeatures(feature_node** &x, feature_node* &x_space, int &l, int &n, double bias, std::vector<Feature>& features);

	static int TransFeatures(feature_node* &x, Feature& feature, int n = -1, double bias = -1.0);

	static int TransLabels(double* &y, std::vector<double>& labels);

	static int TransParam(parameter* &param, int type = L2R_LR, double c = 1, double bias = -1);

	static int LinearMachine::PraseFile(const char* const fileName, std::vector<double> &labels, std::vector<Feature>&features);

	static int LinearMachine::PraseRanksvmFile(const char* const fileName, std::vector<double> &labels, std::vector<int> &queries, std::vector<Feature>&features);

	static int OutputFeature(std::vector<double>& labels, std::vector<Feature>& features, std::string filePath);

	static int OutputRanksvmFeature(std::vector<double>& labels, std::vector<Feature>& features, std::vector<int> &queries, std::string filePath);

private:
	/* 0 for empty
	* 1 for stored model in memory
	*/
	int mState;
	model* mModel;
	parameter* mParam;
	feature_node *mX_space;
	problem mProb;

	int TransProblem(std::vector<double>& labels, std::vector<Feature>& features);

};

int SaveModelBinary(const char *model_file_name, const struct model *model_);

struct model *LoadModelBinary(const char *model_file_name);

#endif /* _MYLINEAR_H */