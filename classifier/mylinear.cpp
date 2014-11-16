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
#include"medline\basic.h"
#include"mylinear.h"
#include"liblinear\blas\blas.h"
#include"liblinear\blas\blasp.h"
using namespace std;

LinearMachine::LinearMachine()
{
	mState = 0;
	mModel = NULL;
	mParam = NULL;
	mProb.l = 0;
	mProb.n = 0;
	mProb.x = NULL;
	mProb.y = NULL;
	mX_space = NULL;
}

LinearMachine::LinearMachine(string modelFilePath)
{
	mState = 0;
	mModel = NULL;
	mParam = NULL;
	Load(modelFilePath);
	mProb.l = 0;
	mProb.n = 0;
	mProb.x = NULL;
	mProb.y = NULL;
	mX_space = NULL;
}

LinearMachine::~LinearMachine()
{
	Destroy();
}

int LinearMachine::Destroy()
{
	if (mModel)
	{
		free_and_destroy_model(&mModel);
		mModel = NULL;
	}
	if (mParam)
	{
		destroy_param(mParam);
		mParam = NULL;
	}
	mProb.l = 0;
	SmartFree(mProb.y);
	SmartFree(mProb.x);
	SmartFree(mProb.query);
	SmartFree(mX_space);
	return 0;
}

int LinearMachine::Load(string modelFilePath)
{
	mState = 1;
	mModel = LoadModelBinary(modelFilePath.c_str());
	return 0;
}

int LinearMachine::Save(string modelFilePath)
{
	if (mState == 0)
	{
		cerr << "Error! In LinearMachine :: No valid model for save " << endl;
		return 1;
	}
	SaveModelBinary(modelFilePath.c_str(), mModel);
	return 0;
}

int LinearMachine::Train()
{
	if (mParam == NULL)
	{
		cerr << "ERROR : No parameter !" << endl;
		return -1;
	}
	if (mX_space == NULL)
	{
		cerr << "ERROR : No Data Input !" << endl;
		return -2;
	}
	mModel = train(&mProb, mParam);
	return 0;
}

//please complete
int LinearMachine::Train(std::vector<double>&labels, feature_node** &x, int type, double c, double bias)
{
	if (mState)
	{
		cerr << "Warning! Already has model, will cover !" << endl;
		if (mModel)
			free_and_destroy_model(&mModel);
		if (mParam)
			destroy_param(mParam);
	}
	if (mProb.l)
	{
		cerr << "ERROR! mProb is not empty" << endl;
		return -1;
	}
	if (x == NULL)
		return -1;

	TransParam(mParam, type, c, bias);

	problem prob;
	if (TransLabels(prob.y, labels) != 0)
	{
		free(prob.y);
		return 2;
	}

	prob.bias = bias;
	prob.l = (int)labels.size();
	prob.x = x;
	prob.n = 0;
	for (int i = 0; i < prob.l; ++i)
	{
		feature_node* arr = prob.x[i];
		int j = 0;
		while (arr[j].index != -1)
		{
			if (arr[j].index > prob.n)
				prob.n = arr[j].index;
			++j;
		}
	}

	mModel = train(&prob, mParam);
	prob.x = NULL;
	free(prob.y);

	mState = 1;
	return 0;
}

int LinearMachine::Train(vector<double>& labels, vector<Feature>& features, int type, double c, double bias)
{
	if (mState)
	{
		cerr << "Warning! Already has model, will cover !" << endl;
		if (mModel)
			free_and_destroy_model(&mModel);
		if (mParam)
			destroy_param(mParam);
	}

	TransParam(mParam, type, c, bias);
	mProb.bias = bias;
	
	if (TransProblem(labels, features))
		return 1;

	Train();

	mState = 1;
	return 0;
}

int LinearMachine::TrainRank(vector<double>& labels, vector<Feature>& features, vector<int> queries, int type, double c, double bias)
{
	if (mState)
	{
		cerr << "Warning! Already has model, will cover !" << endl;
		if (mModel)
			free_and_destroy_model(&mModel);
		if (mParam)
			destroy_param(mParam);
	}

	TransParam(mParam, type, c, bias);
	mProb.bias = bias;

	if (TransProblem(labels, features))
		return 1;

	int* &query = mProb.query;
	mParam->solver_type = type;
	query = Malloc(int, queries.size());
	for (int i = 0; i < queries.size(); ++i)
		query[i] = queries[i];

	Train();

	mState = 1;
	return 0;
}


int LinearMachine::Predict(Feature& feature, double& score)
{
	if (mState == 0)
	{
		cerr << "Error ! Model not exist" << endl;
		return 1;
	}
	int totNode = (int)feature.size() + 1;
	if (mModel->bias >= 0)
		totNode++;
	feature_node* x_space = Malloc(feature_node, totNode);

	int i = 0;
	for (Feature::iterator it = feature.begin(); it != feature.end(); it++)
	{
		x_space[i].index = it->first;
		x_space[i].value = it->second;
		i++;
	}
	if (mModel->bias >= 0)
	{
		x_space[i].index = mModel->nr_feature + 1;
		x_space[i].value = mModel->bias;
		i++;
	}

	x_space[i].index = -1;
	int labels[2];
	double tmp[2];

	if (check_probability_model(mModel))
	{
		get_labels(mModel, labels);
		predict_probability(mModel, x_space, tmp);
		if (labels[0] > labels[1])
			score = tmp[0];
		else
			score = tmp[1];
	}
	else if (mModel->param.solver_type == L2R_L2LOSS_SVR || mModel->param.solver_type == L2R_L2LOSS_RANKSVM || mModel->param.solver_type == L2R_L2LOSS_SVR_DUAL || mModel->param.solver_type == L2R_L1LOSS_SVR_DUAL)
	{
		score = predict(mModel, x_space);
	}
	else
	{
		get_labels(mModel, labels);
		predict_values(mModel, x_space, tmp);
		if (labels[0] > labels[1])
			score = tmp[0];
		else
			score = tmp[1];
	}
	free(x_space);
	return 0;
}

int LinearMachine::Predict(feature_node* x_space, double& score)
{
	if (mState == 0)
	{
		cerr << "Error ! Model not exist" << endl;
		return 1;
	}
	if (x_space == NULL)
		return -1;
	int labels[2];
	double tmp[2];

	if (check_probability_model(mModel))
	{
		get_labels(mModel, labels);
		predict_probability(mModel, x_space, tmp);
		if (labels[0] > labels[1])
			score = tmp[0];
		else
			score = tmp[1];
	}
	else if (mModel->param.solver_type == L2R_L2LOSS_SVR || mModel->param.solver_type == L2R_L2LOSS_RANKSVM || mModel->param.solver_type == L2R_L2LOSS_SVR_DUAL || mModel->param.solver_type == L2R_L1LOSS_SVR_DUAL)
	{
		score = predict(mModel, x_space);
	}
	else
	{
		get_labels(mModel, labels);
		predict_values(mModel, x_space, tmp);
		if (labels[0] > labels[1])
			score = tmp[0];
		else
			score = tmp[1];
	}
	return 0;
}

int LinearMachine::Predict(vector<Feature >& features, vector<double>& scores)
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

int LinearMachine::TransFeatures(feature_node** &x, feature_node* &x_space, int &l, int &n, double bias, std::vector<Feature >& features)
{
	if (x != NULL || x_space != NULL)
	{
		cerr << "ERROR : feature Space Already has Data!" << endl;
		return -1;
	}

	l = (int)features.size();
	x = Malloc(feature_node*, l);
	n = 0;
	int totNode = 0;
	for (unsigned int i = 0; i < features.size(); i++)
		totNode += (int)features[i].size() + 2;
	x_space = Malloc(feature_node, totNode);

	//msg
	clog << "Instance Count : " << l << endl;

	unsigned int j = 0;
	for (unsigned int i = 0; i < (unsigned int)l; i++)
	{
		x[i] = &x_space[j];
		for (Feature::iterator it = features[i].begin(); it != features[i].end(); it++)
		{
			x_space[j].index = it->first;
			x_space[j].value = it->second;
			j++;
			if (it->first > n)
				n = it->first;
		}
		if (bias >= 0)
			x_space[j++].value = bias;
		x_space[j++].index = -1;
	}

	if (bias >= 0)
	{
		n++;
		for (int i = 1; i< l; i++)
			(x[i] - 2)->index = n;
		x_space[j - 2].index = n;
	}


	return 0;
}

int LinearMachine::TransFeatures(feature_node* &x, Feature& feature, int n, double bias)
{
	if (x != NULL)
	{
		cerr << "ERROR : feature Pointer is not Null!" << endl;
		return -1;
	}

	int nodeNum = (int)feature.size() + 2;
	x = Malloc(feature_node, nodeNum);

	int cur = 0;
	for (Feature::iterator it = feature.begin(); it != feature.end(); it++)
	{
		x[cur].index = it->first;
		x[cur].value = it->second;
		cur++;
	}

	if (bias >= 0.0)
	{
		x[cur].value = bias;
		x[cur].index = n + 1;
		++cur;
	}
	x[cur++].index = -1;
	return 0;
}

int LinearMachine::TransLabels(double* &y, std::vector<double>& labels)
{
	y = Malloc(double, labels.size());
	for (int i = 0; i < labels.size(); i++)
	{
		y[i] = labels[i];
	}
	return 0;
}

int LinearMachine::TransParam(parameter* &param, int type, double c, double bias)
{
	if (param != NULL)
	{
		cerr << "ERROR : Alerady has parameters" << endl;
		return -1;
	}
	param = new parameter;
	param->C = c;
	param->solver_type = type;
	param->C = c;

	switch (param->solver_type)
	{
	case L2R_LR:
	case L2R_L2LOSS_SVC:
		param->eps = 0.01;
		break;
	case L2R_L2LOSS_RANKSVM:
	case L2R_L2LOSS_SVR:
		param->eps = 0.001;
		break;
	case L2R_L2LOSS_SVC_DUAL:
	case L2R_L1LOSS_SVC_DUAL:
	case MCSVM_CS:
	case L2R_LR_DUAL:
		param->eps = 0.1;
		break;
	case L1R_L2LOSS_SVC:
	case L1R_LR:
		param->eps = 0.01;
		break;
	case L2R_L1LOSS_SVR_DUAL:
	case L2R_L2LOSS_SVR_DUAL:
		param->eps = 0.1;
		break;
	}
	param->p = 0.1;
	param->nr_weight = 0;
	param->weight_label = NULL;
	param->weight = NULL;
	param->ovo = 0;
	return 0;
}

int LinearMachine::OutputFeature(vector<double>& labels, vector<Feature>& features, string filePath)
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
		for (Feature::iterator it = features[i].begin(); it != features[i].end(); it++)
			fprintf(pOutFile, " %4d:%.6f", it->first, it->second);
		fprintf(pOutFile, "\n");
	}
	fclose(pOutFile);
	return 0;
}

int LinearMachine::OutputRanksvmFeature(vector<double>& labels, vector<Feature>& features, vector<int>& queries, string filePath)
{
	if (labels.size() != features.size() || labels.size() != queries.size() || features.size() != queries.size() )
	{
		cerr << "ERROR! Features size & labels size & queries size not match" << endl;
		return 1;
	}
	FILE* pOutFile = fopen(filePath.c_str(), "w");
	for (unsigned int i = 0; i < features.size(); i++)
	{
		fprintf(pOutFile, "%.2f", labels[i]);
		fprintf(pOutFile, "\t%d", queries[i]);
		for (Feature::iterator it = features[i].begin(); it != features[i].end(); it++)
			fprintf(pOutFile, " %4d:%.6f", it->first, it->second);
		fprintf(pOutFile, "\n");
	}
	fclose(pOutFile);
	return 0;
}

int LinearMachine::PraseFile(const char* const fileName, vector<double> &labels, vector<Feature >&features)
{
	labels.clear();
	features.clear();
	int readBuffer;
	char tmpchr = 0;
	FILE* fp = fopen(fileName, "r");
	int countInstance = 0;
	while (fscanf(fp, "%d%c", &readBuffer, &tmpchr) != EOF)
	{
		if (tmpchr != ':')
		{
			features.push_back(Feature());
			features.back().clear();
			labels.push_back(readBuffer);
			countInstance++;
		}
		else
		{
			int index = readBuffer;
			double value;
			fscanf(fp, "%lf", &value);
			features.back()[index] = value;
		}
	}
	fclose(fp);
	return 0;
}

int LinearMachine::PraseRanksvmFile(const char* const fileName, vector<double> &labels, vector<int> &queries, vector<Feature>&features)
{
	labels.clear();
	features.clear();
	queries.clear();
	double readBuffer;
	int qid;
	char tmpchr = 0;
	FILE* fp = fopen(fileName, "r");

	if (fp == NULL)
	{
		cerr << "err: could not open ranksvm training file."  << endl;
		return 1;
	}

	int countInstance = 0;
	while (fscanf(fp, "%lf%c", &readBuffer, &tmpchr) != EOF)
	{
		if (tmpchr != ':')
		{
			fscanf(fp, "qid:%d", &qid);
			features.push_back(Feature());
			features.back().clear();
			labels.push_back(readBuffer);
			queries.push_back(qid);
			countInstance++;
		}
		else
		{
			int index = (int)(readBuffer + EPS);
			double value;
			fscanf(fp, "%lf", &value);
			features.back()[index] = value;
		}
	}
	fclose(fp);
	return 0;
}

int LinearMachine::SaveFeatureNode(feature_node** x, size_t featureSize, const std::string& fileName)
{
	int rtn = 0;
	size_t totalNodeNum = 0;
	for (int i = 0; i < featureSize; ++i)
	{
		feature_node* ptr = x[i];
		if (ptr == NULL)
			return -1;
		while (ptr->index != -1)
		{
			++totalNodeNum;
			++ptr;
		}
		++totalNodeNum;
	}
	FILE* outFile = fopen(fileName.c_str(), "wb");
	if (outFile == NULL)
		return -1;
	rtn = Write(outFile, totalNodeNum);
	CHECK_RTN(rtn);
	rtn = Write(outFile, featureSize);
	CHECK_RTN(rtn);
	for (size_t i = 0; i < featureSize; ++i)
	{
		feature_node* ptr = x[i];
		if (ptr == NULL)
			return -1;
		size_t len = 0;
		while (ptr->index != -1)
		{
			++len;
			++ptr;
		}
		rtn = Write(outFile, len);
		CHECK_RTN(rtn);

		ptr = x[i];
		while (ptr->index != -1)
		{
			rtn = Write(outFile, ptr->index);
			CHECK_RTN(rtn);
			rtn = Write(outFile, ptr->value);
			CHECK_RTN(rtn);
			++ptr;
		}
	}
	fclose(outFile);
	return 0;
}

int LinearMachine::LoadFeatureNode(feature_node** &x, feature_node* &x_space, size_t& featureSize, const std::string& fileName)
{
	int rtn = 0;
	if (x != NULL)
		cerr << "Warning: feature_node** &x is not NULL" << endl;
	if (x_space != NULL)
		cerr << "Warning: feature_node* &x_space is not NULL" << endl;

	if (!FileExist(fileName))
		return -1;
	FileBuffer buffer(fileName.c_str());
	size_t totalNodeNum = 0;
	rtn = buffer.GetNextData(totalNodeNum);
	CHECK_RTN(rtn);
	rtn = buffer.GetNextData(featureSize);
	CHECK_RTN(rtn);
	x = Malloc(feature_node*, featureSize);
	x_space = Malloc(feature_node, totalNodeNum);

	size_t cur = 0;
	for (size_t i = 0; i < featureSize; ++i)
	{
		size_t len;
		rtn = buffer.GetNextData(len);
		x[i] = x_space + cur;
		for (size_t j = 0; j < len; ++j)
		{
			rtn = buffer.GetNextData(x_space[cur].index);
			CHECK_RTN(rtn);
			rtn = buffer.GetNextData(x_space[cur++].value);
			CHECK_RTN(rtn);
		}
		x_space[cur++].index = -1;
	}
	return 0;
}

int LinearMachine::TransProblem(vector<double>& labels, vector<Feature >& features)
{
	if (mProb.l)
	{
		cerr << "ERROR! mProb is not empty" << endl;
		return -1;
	}
	if (TransFeatures(mProb.x, mX_space, mProb.l, mProb.n, mProb.bias, features) != 0)
		return 1;
	if (TransLabels(mProb.y, labels) != 0)
		return 2;
	return 0;
}

static const char *solver_type_table[] =
{
	"L2R_LR", "L2R_L2LOSS_SVC_DUAL", "L2R_L2LOSS_SVC", "L2R_L1LOSS_SVC_DUAL", "MCSVM_CS",
	"L1R_L2LOSS_SVC", "L1R_LR", "L2R_LR_DUAL",
	"L2R_L2LOSS_RANKSVM", "", "",
	"L2R_L2LOSS_SVR", "L2R_L2LOSS_SVR_DUAL", "L2R_L1LOSS_SVR_DUAL", NULL
};

int SaveModelBinary(const char *model_file_name, const struct model *model_)
{
	int i;
	int nr_feature = model_->nr_feature;
	int n;
	const parameter& param = model_->param;

	if (model_->bias >= 0)
		n = nr_feature + 1;
	else
		n = nr_feature;
	int w_size = n;
	FILE *fp = fopen(model_file_name, "wb");
	if (fp == NULL) return -1;

#ifdef __GNUC__	
	char *old_locale = strdup(setlocale(LC_ALL, NULL));
#else
	char *old_locale = _strdup(setlocale(LC_ALL, NULL));
#endif
	setlocale(LC_ALL, "C");

	int nr_w;
	if (model_->nr_class == 2 && model_->param.solver_type != MCSVM_CS)
		nr_w = 1;
	else
		nr_w = model_->nr_class;

	Write(fp, "solver_type");
	Write(fp, solver_type_table[param.solver_type]);

	Write(fp, "nr_class");
	Write(fp, model_->nr_class);


	if (model_->label)
	{
		Write(fp, "label");
		for (i = 0; i < model_->nr_class; i++)
			Write(fp, model_->label[i]);
	}

	Write(fp, "nr_feature");
	Write(fp, nr_feature);

	Write(fp, "bias");
	Write(fp, model_->bias);

	Write(fp, "w");
	for (i = 0; i<w_size; i++)
	{
		int j;
		for (j = 0; j < nr_w; j++)
			Write(fp, model_->w[i*nr_w + j]);
	}

	setlocale(LC_ALL, old_locale);
	free(old_locale);

	if (ferror(fp) != 0 || fclose(fp) != 0) return -1;
	else return 0;
}

struct model *LoadModelBinary(const char *model_file_name)
{
	FILE *fp = fopen(model_file_name, "rb");
	if (fp == NULL) return NULL;

	FileBuffer buffer(fp);

	int i;
	int nr_feature;
	int n;
	int nr_class;
	double bias;
	model *model_ = Malloc(model, 1);
	parameter& param = model_->param;

	model_->label = NULL;

#ifdef __GNUC__	
	char *old_locale = strdup(setlocale(LC_ALL, NULL));
#else
	char *old_locale = _strdup(setlocale(LC_ALL, NULL));
#endif
	setlocale(LC_ALL, "C");

	char* cmd = NULL;
	while (1)
	{
		SmartFree(cmd);
		buffer.GetNextData(cmd);
		if (strcmp(cmd, "solver_type") == 0)
		{
			SmartFree(cmd);
			buffer.GetNextData(cmd);
			int i;
			for (i = 0; solver_type_table[i]; i++)
			{
				if (strcmp(solver_type_table[i], cmd) == 0)
				{
					param.solver_type = i;
					break;
				}
			}
			if (solver_type_table[i] == NULL)
			{
				fprintf(stderr, "unknown solver type.\n");

				setlocale(LC_ALL, old_locale);
				free(model_->label);
				free(model_);
				free(old_locale);
				return NULL;
			}
		}
		else if (strcmp(cmd, "nr_class") == 0)
		{
			buffer.GetNextData(nr_class);
			model_->nr_class = nr_class;
		}
		else if (strcmp(cmd, "nr_feature") == 0)
		{
			buffer.GetNextData(nr_feature);
			model_->nr_feature = nr_feature;
		}
		else if (strcmp(cmd, "bias") == 0)
		{
			buffer.GetNextData(bias);
			model_->bias = bias;
		}
		else if (strcmp(cmd, "w") == 0)
		{
			break;
		}
		else if (strcmp(cmd, "label") == 0)
		{
			int nr_class = model_->nr_class;
			model_->label = Malloc(int, nr_class);
			for (int i = 0; i < nr_class; i++)
				buffer.GetNextData(model_->label[i]);
		}
		else
		{
			fprintf(stderr, "unknown text in model file: [%s]\n", cmd);
			setlocale(LC_ALL, old_locale);
			free(model_->label);
			free(model_);
			free(old_locale);
			return NULL;
		}
	}
	SmartFree(cmd);

	nr_feature = model_->nr_feature;
	if (model_->bias >= 0)
		n = nr_feature + 1;
	else
		n = nr_feature;
	int w_size = n;
	int nr_w;
	if (nr_class == 2 && param.solver_type != MCSVM_CS)
		nr_w = 1;
	else
		nr_w = nr_class;

	model_->w = Malloc(double, w_size*nr_w);
	for (i = 0; i<w_size; i++)
	{
		int j;
		for (j = 0; j < nr_w; j++)
			buffer.GetNextData(model_->w[i*nr_w + j]);
	}

	setlocale(LC_ALL, old_locale);
	free(old_locale);

	if (ferror(fp) != 0 || fclose(fp) != 0) return NULL;

	return model_;
}
