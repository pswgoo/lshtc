#include "statistics.h"
#include <algorithm>
#include <iostream>
using namespace std;

int LoadTrainSet(std::string fileName, std::vector<std::vector<int>>& labels, std::vector<std::map<int, int>>& featureSet)
{
	const int L_MAX_LINE_NUM = 1000000;
	char line[L_MAX_LINE_NUM];

	labels.clear();
	featureSet.clear();
	labels.reserve(5000000);
	featureSet.reserve(5000000);

	FILE* inFile = fopen(fileName.c_str(), "r");
	//fgets(line, L_MAX_LINE_NUM, inFile);
	int label = 0;
	int cnt = 0;
	while (fscanf(inFile, "%d", &label) != EOF)
	{
		if ((cnt & ((1 << 14) - 1)) == 0)
			clog << "\r" << cnt;
		++cnt;
		vector<int> tmpLabels;
		tmpLabels.push_back(label);
		char ch = 0;
		//clog << "load label" << endl;
		while ((ch = (char)fgetc(inFile)) == ',')
		{
			fscanf(inFile, "%d", &label);
			tmpLabels.push_back(label);
		}
		map<int, int> feature;
		if (ch != '\n')
		{
			//clog << "load feature" << endl;
			while (true)
			{
				int index = 0, value = 0;
				fscanf(inFile, "%d:%d%c", &index, &value, &ch);
				feature[index] = value;
				if (ch == '\n')
					break;
			}
		}
		labels.push_back(tmpLabels);
		featureSet.push_back(feature);
	}
	fclose(inFile);
	clog << endl;
	clog << "Total load " << labels.size() << " instances" << endl;
	if (labels.size() != featureSet.size())
		return -1;
	return 0;
}

int Comp(const pair<int, int>& p1, const pair<int, int>& p2)
{
	return p1.second > p2.second;
}

int StatisticLabelDistribution(std::string fileName, std::vector<std::vector<int>>& labels)
{
	map<int, int> labelCnt;
	int sum = 0;
	for (size_t i = 0; i < labels.size(); ++i)
	{
		sum += (int)labels[i].size();
		for (size_t j = 0; j < labels[i].size(); ++j)
		{
			if (labelCnt.count(labels[i][j]) == 0)
				labelCnt[labels[i][j]] = 0;
			++labelCnt[labels[i][j]];
		}
	}

	clog << "Total " << labels.size() << " samples" << endl;
	clog << "Average labels is " << double(sum) / double(labels.size()) << endl;

	vector<pair<int, int>> vecLabel;	
	for (auto it = labelCnt.begin(); it != labelCnt.end(); ++it)
		vecLabel.push_back(*it);
	sort(vecLabel.begin(), vecLabel.end(), Comp);
	FILE *outFile = fopen(fileName.c_str(), "w");
	fprintf(outFile, "id, occur_num, percentage\n");
	for (size_t i = 0; i != vecLabel.size(); ++i)
		fprintf(outFile, "%d,%d,%lf\n", vecLabel[i].first, vecLabel[i].second, double(vecLabel[i].second) / double(labels.size()));
	fclose(outFile);

	outFile = fopen("label_rank_num.csv", "w");
	fprintf(outFile, "id, rank, occur_num, percentage\n");
	for (size_t i = 0; i < vecLabel.size(); i < 5 ? ++i : i += 25000)
		fprintf(outFile, "%d, %d, %d, %lf\n", vecLabel[i].first, (int)i, vecLabel[i].second, double(vecLabel[i].second) / double(labels.size()));
	fclose(outFile);
	return 0;
}
