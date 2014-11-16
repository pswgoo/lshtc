#define _CRT_SECURE_NO_WARNINGS
#include<iostream>
#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<cmath>
#include<string>
#include<algorithm>
#include<map>
#include<vector>
#include<set>
#include"libsvm/svm.h"
#include"mysvm.h"
using namespace std;

SVM_Machine::SVM_Machine()
{
	mState = 0;
	mModel = NULL;
	mParam = NULL;
	mProb.l = 0;
	mProb.x = NULL;
	mProb.y = NULL;
	mX_space = NULL;
}

SVM_Machine::SVM_Machine(string modelFilePath)
{
	mState = 0;
	mModel = NULL;
	mParam = NULL;
	Load(modelFilePath);
	mProb.l = 0;
	mProb.x = NULL;
	mProb.y = NULL;
	mX_space = NULL;
}

SVM_Machine::~SVM_Machine()
{
	if (mModel)
		svm_free_and_destroy_model(&mModel);
	if (mParam)
		svm_destroy_param(mParam);
	mProb.l = 0;
	if (mProb.y)
		free(mProb.y);
	if (mProb.x)
		free(mProb.x);
	if (mX_space)
		free(mX_space);
}

int SVM_Machine::Load(string modelFilePath)
{
	mState = 1;
	mModel = svm_load_model(modelFilePath.c_str());
	return 0;
}

int SVM_Machine::Save(string modelFilePath)
{
	if (mState == 0)
	{
		cerr << "Error! In SVM_Machine :: No valid model for save " << endl;
		return 1;
	}
	svm_save_model(modelFilePath.c_str(), mModel);
	return 0;
}

int SVM_Machine::Train(vector<double>& labels, vector<map<int, double> >& features, int type, int kernel, double c, double g)
{
	if (mState)
	{
		cerr << "Warning! Already has model, will cover !" << endl;
		if (mModel)
			svm_free_and_destroy_model(&mModel);
		if (mParam)
			svm_destroy_param(mParam);
	}

	mParam = new svm_parameter;
	mParam->svm_type = type;
	mParam->kernel_type = kernel;
	mParam->degree = 3;
	mParam->gamma = g; // 1/features
	if (g < 1e-9)
	{
		int maxIndex = 0;
		for (unsigned int i = 0; i < features.size(); i++)
			maxIndex = max(maxIndex, features[i].rbegin()->first);
		mParam->gamma = 1.0 / maxIndex;
	}
	mParam->coef0 = 0;
	mParam->nu = 0.5;
	mParam->cache_size = 1000;
	mParam->C = c;
	mParam->eps = 1e-3;
	mParam->p = 0.1;
	mParam->shrinking = 1;
	if (mParam->svm_type == C_SVC)
		mParam->probability = 1;
	else
		mParam->probability = 0;
	mParam->nr_weight = 0;
	mParam->weight_label = NULL;
	mParam->weight = NULL;

	if (TransFeatures(labels, features))
		return 1;

	mModel = svm_train(&mProb, mParam);

	mState = 1;
	return 0;
}

int SVM_Machine::Predict(map<int, double>& feature, double& score)
{
	if (mState == 0)
	{
		cerr << "Error ! Model not exist" << endl;
		return 1;
	}
	svm_node* x_space = Malloc(svm_node, feature.size() + 1);

	int i = 0;
	for (map<int, double>::iterator it = feature.begin(); it != feature.end(); it++)
	{
		x_space[i].index = it->first;
		x_space[i].value = it->second;
		i++;
	}

	x_space[i].index = -1;
	int labels[2];
	double tmp[2];
	if (mModel->param.svm_type == C_SVC)
	{
		svm_get_labels(mModel, labels);
		svm_predict_probability(mModel, x_space, tmp);
		if (labels[0] == 1)
			score = tmp[0];
		else
			score = tmp[1];
	}
	else if (mModel->param.svm_type == EPSILON_SVR)
		score = svm_predict(mModel, x_space);

	free(x_space);
	return 0;
}

int SVM_Machine::Predict(vector<map<int, double> >& features, vector<double>& scores)
{
	scores.clear();
	for (unsigned int i = 0; i < features.size(); i++)
	{
		double score;
		if (Predict(features[i], score))
			return 1;
		scores.push_back(score);
	}
	return 0;
}

int SVM_Machine::OutputFeature(vector<double>& labels, vector<map<int, double>>& features, string filePath)
{
	if (labels.size() != features.size())
	{
		cerr << "ERROR! Features size & labels size not match" << endl;
		return 1;
	}
	FILE* pOutFile = fopen(filePath.c_str(), "w");
	for (unsigned int i = 0; i < features.size(); i++)
	{
		fprintf(pOutFile, "%.2f", labels[i]);
		for (map<int, double>::iterator it = features[i].begin(); it != features[i].end(); it++)
			fprintf(pOutFile, " %4d:%.6f", it->first, it->second);
		fprintf(pOutFile, "\n");
	}
	fclose(pOutFile);
	return 0;
}

int SVM_Machine::TransFeatures(vector<double>& labels, vector<map<int, double> >& features)
{
	if (mProb.l)
	{
		cerr << "ERROR! mProb is not empty" << endl;
		return 1;
	}
	mProb.l = (int)features.size();
	mProb.y = Malloc(double, mProb.l);
	mProb.x = Malloc(svm_node*, mProb.l);
	int totNode = 0;
	for (unsigned int i = 0; i < features.size(); i++)
		totNode += (int)features[i].size() + 1;
	mX_space = Malloc(svm_node, totNode);

	//msg
	clog << "Instance Count : " << mProb.l << endl;

	unsigned int j = 0;
	for (unsigned int i = 0; i < (unsigned int)mProb.l; i++)
	{
		mProb.x[i] = &mX_space[j];
		mProb.y[i] = labels[i];
		unsigned int k = 0;
		for (map<int, double>::iterator it = features[i].begin(); it != features[i].end(); it++)
		{
			mX_space[j].index = it->first;
			mX_space[j].value = it->second;
			j++;
			k++;
		}
		mX_space[j++].index = -1;
	}

	return 0;
}