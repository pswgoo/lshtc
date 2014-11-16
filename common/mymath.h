#ifndef MYMATH_H
#define MYMATH_H
#include "common_basic.h"
#include <vector>

//统计matrix中值的分布，函数中会将criticalValue从小到大排列，distribution[i]值对应matrix里在区间[criticalValue[i],criticalValue[i+1])的值的个数
int StatisticalDistribution(std::vector<std::vector<double>>& matrix, std::vector<double>& criticalValue, std::vector<int>& distribution);

int GetPearsonCorrelation(const std::vector<double>& xx, const std::vector<double>& yy, double& pearson);

int GetSpearmanCorrelation(const std::vector<double>& xx, const std::vector<double>& yy, double& spearman);

int GetKaSquare(const std::vector<std::vector<double>>& matrix, double& kaValue);

int GetRocCurve(std::vector<std::pair<int, bool>>& instances, std::vector<std::pair<double,double>>& rocPoint);

int GetAuc(const std::vector<std::pair<double, double>>& xy, double& auc);

template <typename tData>
int GetAccuracy(const std::vector<tData>& vec1, const std::vector<tData>& vec2, double& accuracy)
{
	if (vec1.size() != vec2.size())
		return -1;
	
	int trueCnt = 0;
	for (size_t i = 0; i < vec1.size(); ++i)
	{
		if (vec1[i] == vec2[i])
			++trueCnt;
	}

	accuracy = double(trueCnt) / double(vec1.size());
	return 0;
}

#endif
