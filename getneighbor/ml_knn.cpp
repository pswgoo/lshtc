#include "ml_knn.h"
#include "common/file_utility.h"
using namespace std;

MultiLabelKnn::MultiLabelKnn(int k)
{
	mK = k;
	Clear();
}

MultiLabelKnn::~MultiLabelKnn()
{

}

int MultiLabelKnn::Resize(int size)
{
	mPriorProb.resize(size);
	mPosCnt.resize(size);
	mNegCnt.resize(size);
	mPosSum.resize(size);
	mNegSum.resize(size);
	for (int i = 0; i < size; ++i)
	{
		mPriorProb[i] = 0.0;
		mPosCnt[i].clear();
		mPosSum[i] = 0;
		mNegCnt[i].clear();
		mNegSum[i] = 0;
	}
	return 0;
}

int MultiLabelKnn::Clear()
{
	mTrainSet.clear();
	mIndexToLabel.clear();
	mLabelToIndex.clear();

	mPriorProb.clear();
	mPosCnt.clear();
	mNegCnt.clear();
	mPosSum.clear();
	mNegSum.clear();
	return 0;
}

int MultiLabelKnn::Initialize(const std::map<int, std::vector<int>>& trainSet, const std::map<int, std::vector<int>>& testSet, const FeatureNeighbor& neighbors)
{
	int rtn = 0;
	Clear();
	//Initialize mLabelToIndex and mIndexToLabel
	for (auto it = trainSet.begin(); it != trainSet.end(); ++it)
	{
		for (size_t j = 0; j < it->second.size(); ++j)
		{
			if (mLabelToIndex.count(it->second[j]) == 0)
			{
				int index = (int)mLabelToIndex.size();
				mLabelToIndex[it->second[j]] = index;
				mIndexToLabel[index] = it->second[j];
			}
		}
	}
	for (auto it = testSet.begin(); it != testSet.end(); ++it)
	{
		for (size_t j = 0; j < it->second.size(); ++j)
		{
			if (mLabelToIndex.count(it->second[j]) == 0)
			{
				int index = (int)mLabelToIndex.size();
				mLabelToIndex[it->second[j]] = index;
				mIndexToLabel[index] = it->second[j];
			}
		}
	}
	if (mLabelToIndex.size() != mIndexToLabel.size())
	{
		cerr << "Error: mLabelToIndex.size not match to mIndexToLabel.size" << endl;
		return -1;
	}
	clog << "Total has " << mLabelToIndex.size() << " labels" << endl;

	//Initialize mTrainSet
	mTrainSet = trainSet;
	for (auto it = mTrainSet.begin(); it != mTrainSet.end(); ++it)
	{
		for (size_t j = 0; j < it->second.size(); ++j)
		{
			int index = mLabelToIndex.at(it->second[j]);
			it->second[j] = index;
		}
	}
	clog << "Trainset has " << mTrainSet.size() << " instances" << endl;

	const int label_size = (int)mLabelToIndex.size();
	vector<int> sum_none_zero;
	sum_none_zero.resize(label_size);
	for (int i = 0; i < label_size; ++i)
		sum_none_zero[i] = 0;
	Resize(label_size);

	int cnt_signal = 0;
	//Initialize mPosCnt, mNegCnt
	for (auto it = testSet.begin(); it != testSet.end(); ++it)
	{
		if ((cnt_signal & 4095) == 0)
			clog << "\r" << " count " << cnt_signal << "th instance";
		++cnt_signal;
		set<int> true_labels;
		for (size_t i = 0; i < it->second.size(); ++i)
			true_labels.insert(mLabelToIndex.at(it->second[i]));
		
		vector<int> neighborIds;
		rtn = neighbors.GetNeighbor(it->first, mK, neighborIds);
		CHECK_RTN(rtn);
		
		map<int, int> label_cnt;
		for (size_t i = 0; i < neighborIds.size(); ++i)
		{
			const vector<int>& labels = mTrainSet.at(neighborIds[i]);
			for (size_t j = 0; j < labels.size(); ++j)
			{
				if (label_cnt.count(labels[j]) == 0)
					label_cnt[labels[j]] = 0;
				++label_cnt.at(labels[j]);
			}
		}
		
		for (auto it = label_cnt.begin(); it != label_cnt.end(); ++it)
		{
			++sum_none_zero.at(it->first);
			if (true_labels.count(it->first) > 0)
			{
				if (mPosCnt.at(it->first).count(it->second) == 0)
					mPosCnt.at(it->first)[it->second] = 0;
				++mPosCnt.at(it->first).at(it->second);
			}
			else
			{
				if (mNegCnt.at(it->first).count(it->second) == 0)
					mNegCnt.at(it->first)[it->second] = 0;
				++mNegCnt.at(it->first).at(it->second);
			}
		}
		
		for (auto it = true_labels.begin(); it != true_labels.end(); ++it)
		{
			if (mPosCnt.at(*it).count(0) == 0)
				mPosCnt.at(*it)[0] = 0;
			++mPosCnt.at(*it).at(0);
		}
		
	}
	clog << endl;
	int test_set_size = (int)testSet.size();
	for (size_t i = 0; i < mNegCnt.size(); ++i)
	{
		if (mPosCnt.at(i).count(0) > 0)
			mNegCnt.at(i)[0] = test_set_size - sum_none_zero.at(i) - mPosCnt.at(i).at(0);
		else
			mNegCnt.at(i)[0] = test_set_size - sum_none_zero.at(i);
	}

	//Initialize mPosSum, mNegSum
	for (size_t i = 0; i < mPosCnt.size(); ++i)
	{
		mPosSum.at(i) = 0;
		for (auto it = mPosCnt.at(i).begin(); it != mPosCnt.at(i).end(); ++it)
			mPosSum.at(i) += it->second;

		mNegSum.at(i) = 0;
		for (auto it = mNegCnt.at(i).begin(); it != mNegCnt.at(i).end(); ++it)
			mNegSum.at(i) += it->second;

		if (mNegSum.at(i) + mPosSum.at(i) != test_set_size)
		{
			cerr << "Error: mNegSum + mPosSum != test_set_size" << endl;
		}
	}

	//Initialize mPriorProb with train set
	map<int, int> prior_label_cnt;
	for (auto it = mTrainSet.begin(); it != mTrainSet.end(); ++it)
	{
		for (size_t j = 0; j < it->second.size(); ++j)
		{
			if (prior_label_cnt.count(it->second[j]) == 0)
				prior_label_cnt[it->second[j]] = 0;
			++prior_label_cnt.at(it->second[j]);
		}
	}
	int prior_size = (int)mTrainSet.size();
	for (size_t i = 0; i < mPriorProb.size(); ++i)
	{
		if (prior_label_cnt.count((int)i) > 0)
			mPriorProb[i] = (double(mLaplsSmooth) +  double(prior_label_cnt.at(i))) / (mLaplsSmooth*2.0 + double(prior_size));
		else
			mPriorProb[i] = double(mLaplsSmooth) / (mLaplsSmooth*2.0 + double(prior_size));
	}

	clog << "Initilize completed" << endl;
	return 0;
}

int MultiLabelKnn::Predict(const FeatureNeighbor& neighbors, int testId, std::vector<std::pair<int, double>>& scores) const
{
	int rtn = 0;
	vector<int> neighborIds;
	rtn = neighbors.GetNeighbor(testId, mK, neighborIds);
	CHECK_RTN(rtn);

	map<int, int> labelCnt;
	for (size_t i = 0; i < neighborIds.size(); ++i)
	{
		const vector<int>& labels = mTrainSet.at(neighborIds[i]);
		for (size_t j = 0; j < labels.size(); ++j)
		{
			if (labelCnt.count(labels[j]) == 0)
				labelCnt[labels[j]] = 0;
			++labelCnt[labels[j]];
		}
	}

	scores.clear();
	for (auto it = labelCnt.begin(); it != labelCnt.end(); ++it)
	{
		double posProb = 0.0, negProb = 0.0;
		auto fit = mPosCnt[it->first].find(it->second);
		if (fit != mPosCnt[it->first].end())
			posProb = (double(mLaplsSmooth) + double(fit->second)) / (double(mLaplsSmooth) * double(mK) + double(mPosSum[it->first]));
		else
			posProb = double(mLaplsSmooth) / (double(mLaplsSmooth) * double(mK) + double(mPosSum[it->first]));

		auto neg_fit = mNegCnt[it->first].find(it->second);
		if (neg_fit != mNegCnt[it->first].end())
			negProb = (double(mLaplsSmooth) + double(neg_fit->second)) / (double(mLaplsSmooth) * double(mK) + double(mNegSum[it->first]));
		else
			negProb = double(mLaplsSmooth) / (double(mLaplsSmooth) * double(mK) + double(mNegSum[it->first]));

		double posPrior = mPriorProb[it->first];
		double negPrior = 1.0 - posPrior;
		double score = (posPrior * posProb) / (negPrior * negProb);
		if (mIndexToLabel.count(it->first) == 0)
		{
			cerr << "Error: index to label error" << endl;
			return -1;
		}
		scores.push_back(make_pair(mIndexToLabel.at(it->first), score));
	}

	sort(scores.begin(), scores.end(), CmpPairByLagerSecond<int,double>);

	return 0;
}

int MultiLabelKnn::Predict(const FeatureNeighbor& neighbors, int testId, std::vector<int>& labels, double threshold) const
{
	int rtn = 0;
	vector<pair<int, double> > scores;
	rtn = Predict(neighbors, testId, scores);
	CHECK_RTN(rtn);

	sort(scores.begin(), scores.end(), CmpPairByLagerSecond<int, double>);
	for (size_t i = 0; i < scores.size() && scores[i].second > threshold; ++i)
	{
		labels.push_back(scores[i].first);
	}
	return 0;
}

int MultiLabelKnn::Save(std::string file_name, int print_log)
{
	int rtn = 0;

	FILE* out_file = fopen(file_name.c_str(), "wb");
	rtn = Save(out_file, print_log);
	fclose(out_file);
	CHECK_RTN(rtn);

	return 0;
}

int MultiLabelKnn::Save(FILE* out_file, int print_log)
{
	int rtn = 0;
	rtn = Write(out_file, mK);
	CHECK_RTN(rtn);
	rtn = Write(out_file, mLabelToIndex);
	CHECK_RTN(rtn);
	rtn = Write(out_file, mIndexToLabel);
	CHECK_RTN(rtn);
	rtn = Write(out_file, mTrainSet);
	CHECK_RTN(rtn);
	rtn = Write(out_file, mPriorProb);
	CHECK_RTN(rtn);
	rtn = Write(out_file, mPosCnt);
	CHECK_RTN(rtn);
	rtn = Write(out_file, mPosSum);
	CHECK_RTN(rtn);
	rtn = Write(out_file, mNegCnt);
	CHECK_RTN(rtn);
	rtn = Write(out_file, mNegSum);
	CHECK_RTN(rtn);

	if (print_log != SILENT)
		clog << "Save complete" << endl;
	return 0;
}

int MultiLabelKnn::Load(std::string file_name, int print_log)
{
	int rtn = 0;

	Clear();
	FILE* in_file = fopen(file_name.c_str(), "rb");
	rtn = Load(in_file, print_log);
	fclose(in_file);
	CHECK_RTN(rtn);

	return 0;
}

int MultiLabelKnn::Load(FILE* in_file, int print_log)
{
	int rtn = 0;
	Clear();
	rtn = Read(in_file, mK);
	CHECK_RTN(rtn);
	rtn = Read(in_file, mLabelToIndex);
	CHECK_RTN(rtn);
	rtn = Read(in_file, mIndexToLabel);
	CHECK_RTN(rtn);
	rtn = Read(in_file, mTrainSet);
	CHECK_RTN(rtn);
	rtn = Read(in_file, mPriorProb);
	CHECK_RTN(rtn);
	rtn = Read(in_file, mPosCnt);
	CHECK_RTN(rtn);
	rtn = Read(in_file, mPosSum);
	CHECK_RTN(rtn);
	rtn = Read(in_file, mNegCnt);
	CHECK_RTN(rtn);
	rtn = Read(in_file, mNegSum);
	CHECK_RTN(rtn);

	if (print_log != SILENT)
		clog << "Load complete" << endl;
	return 0;
}
