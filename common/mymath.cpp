#include "mymath.h"
#include <algorithm>
using namespace std;

int StatisticalDistribution(std::vector<std::vector<double>>& matrix, std::vector<double>& criticalValue, std::vector<int>& distribution)
{
	distribution.clear();
	if (criticalValue.empty())
		return 0;

	sort(criticalValue.begin(), criticalValue.end());
	distribution.resize(criticalValue.size());
	for (int i = 0; i < (int)criticalValue.size(); ++i)
		distribution[i] = 0;

	for (int i = 0; i < (int)matrix.size(); ++i)
	{
		for (int j = 0; j < (int)matrix[i].size(); ++j)
		{
			double value = matrix[i][j];
			int left = 0, right = (int)criticalValue.size() - 1;
			while (left <= right)
			{
				int mid = (left + right) >> 1;
				if (criticalValue[mid] <= value)
					left = mid + 1;
				else
					right = mid - 1;
			}
			if (right >= 0)
				distribution[right]++;
		}
	}
	return 0;
}

int GetPearsonCorrelation(const vector<double>& xx, const vector<double>& yy, double& pearson)
{
	if (xx.size() != yy.size())
		return -1;

	double sumX = 0.0, sumY = 0.0;
	for (size_t i = 0; i < xx.size(); ++i)
	{
		sumX += xx[i];
		sumY += yy[i];
	}

	double averX = sumX / (double)xx.size();
	double averY = sumY / (double)yy.size();

	double up = 0.0;
	double xSquare = 0.0, ySquare = 0.0;
	for (size_t i = 0; i < xx.size(); ++i)
	{
		up += (xx[i] - averX) * (yy[i] - averY);
		xSquare += (xx[i] - averX) * (xx[i] - averX);
		ySquare += (yy[i] - averY) * (yy[i] - averY);
	}

	double down = 0.0;
	down = sqrt(xSquare) * sqrt(ySquare);
	if (down < EPS)
		pearson = -1.0e10;
	else
		pearson = up / down;
	return 0;
}

int GetSpearmanCorrelation(const vector<double>& xx, const vector<double>& yy, double& spearman)
{
	if (xx.size() != yy.size())
		return 0;

	int rtn = 0;
	vector<pair<double, int>> sortX;
	vector<pair<double, int>> sortY;
	for (size_t i = 0; i < xx.size(); ++i)
	{
		sortX.push_back(make_pair(xx[i], (int)i));
		sortY.push_back(make_pair(yy[i], (int)i));
	}
	sort(sortX.begin(), sortX.end());
	sort(sortY.begin(), sortY.end());
	
	vector<double> rankX;
	rankX.resize(xx.size());
	int rank = (int)xx.size();
	for (size_t i = 0; i < sortX.size();--rank,++i)
	{
		double sum = (double)rank;
		int cnt = 1;
		int start = (int)i;

		while (i < sortX.size() - 1 && sortX[i].first == sortX[i + 1].first)
		{
			--rank, ++i;
			sum += rank;
			++cnt;
		}
		
		for (int j = start; j <= i; ++j)
			rankX[sortX[j].second] = sum / (double)cnt;
	}

	vector<double> rankY;
	rankY.resize(yy.size());
	rank = (int)yy.size();
	for (size_t i = 0; i < sortY.size(); --rank, ++i)
	{
		double sum = (double)rank;
		int cnt = 1;
		int start = (int)i;

		while (i < sortY.size() - 1 && sortY[i].first == sortY[i + 1].first)
		{
			--rank, ++i;
			sum += rank;
			++cnt;
		}

		for (int j = start; j <= i; ++j)
			rankY[sortY[j].second] = sum / (double)cnt;
	}

	rtn = GetPearsonCorrelation(rankX, rankY, spearman);
	return 0;
}

int GetKaSquare(const std::vector<std::vector<double>>& matrix, double& kaValue)
{
	kaValue = 0.0;
	if (matrix.empty())
		return 0;
	vector<double> rowSum;
	vector<double> colSum;
	rowSum.resize(matrix.size());
	colSum.resize(matrix[0].size());
	for (size_t i = 0; i < rowSum.size(); ++i)
		rowSum[i] = 0.0;
	for (size_t i = 0; i < colSum.size(); ++i)
		colSum[i] = 0.0;

	double sum = 0.0;
	for (size_t i = 0; i < matrix.size(); i++)
	for (size_t j = 0; j < matrix[i].size(); ++j)
	{
		rowSum[i] += matrix[i][j];
		colSum[j] += matrix[i][j];
		sum += matrix[i][j];
	}

	kaValue = 0.0;
	for (size_t i = 0; i < matrix.size(); i++)
	for (size_t j = 0; j < matrix[i].size(); ++j)
	{
		double eij = rowSum[i] * colSum[j] / sum;
		if (eij > EPS)
			kaValue += (matrix[i][j] - eij) * (matrix[i][j] - eij) / eij;
	}
	return 0;
}

int GetRocCurve(std::vector<std::pair<int, bool>>& instances, std::vector<std::pair<double, double>>& rocPoint)
{
	sort(instances.begin(), instances.end());
	int pos = 0, neg = 0;
	for (size_t i = 0; i < instances.size(); ++i)
	{
		if (instances[i].second)
			++pos;
		else
			++neg;
	}
	
	rocPoint.clear();
	int tp = 0, fp = 0;
	rocPoint.push_back(make_pair(0.0, 0.0));
	for (size_t i = 0; i < instances.size(); ++i)
	{
		if (instances[i].second)
			++tp;
		else
			++fp;

		rocPoint.push_back(make_pair(double(fp) / double(neg), double(tp) / double(pos)));
	}
	return 0;
}

int GetAuc(const std::vector<std::pair<double, double>>& xy, double& auc)
{
	vector<pair<double, double>> locXy = xy;
	sort(locXy.begin(), locXy.end(), CmpPairBySmallerFirst<double, double>);

	auc = 0.0;

	for (size_t i = 1; i < locXy.size(); ++i)
	{
		auc += (locXy[i - 1].second + locXy[i].second) * (locXy[i].first - locXy[i - 1].first) * 0.5;
	}
	return 0;
}
