#ifndef STATISTICS_H
#define STATISTICS_H
#include <string>
#include <vector>
#include <map>

int LoadTrainSet(std::string fileName, std::vector<std::vector<int>>& labels, std::vector<std::map<int,int>>& featureSet);

int StatisticLabelDistribution(std::string fileName, std::vector<std::vector<int>>& labels);

#endif