#include "statistics.h"
#include <cstdio>
#include <iostream>
using namespace std;



int main()
{
	int rtn = 0;
	vector<vector<int>> labels;
	vector<map<int, int>> features;
	rtn = LoadTrainSet("train.txt", labels, features);
	rtn = StatisticLabelDistribution("label_freq.csv", labels);
	size_t sum = 0;
	for (size_t i = 0; i < features.size(); ++i)
	{
		sum += features[i].size();
	}
	clog << "Average features is " << double(sum) / double(features.size()) << endl;
	if (rtn != 0)
		clog << "Error" << endl;
	else
		clog << "Complete" << endl;
	return 0;
}