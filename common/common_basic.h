#ifndef COMMON_BASIC
#define COMMON_BASIC
#define _CRT_SECURE_NO_WARNINGS
#define _FILE_OFFSET_BITS  64
#define Malloc(type,n) ((n)==0)?NULL:(type *)malloc((n)*sizeof(type))
#define CHECK_RTN(n) if ((n)!=0) return (n)

#include <cstdlib>
#include <cstdio>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <omp.h>

const int MATCH = 0;
const int INCLUDE_ANY = 1;
const int INCLUDE_ALL = 2;
const int SILENT = 0;
const int STATUS_ONLY = 1;
const int FULL_LOG = 2;
const int PRINTABLE_CHAR = 127 - 31;
const bool SEQUENCE_MODE = false;
const bool WORD_MODE = true;
const double LG = log(10);
const double EPS = 1e-7;

template <typename tPointer>
int SmartFree(tPointer* &pointer)
{
	if (pointer == NULL)
		return 0;
	free(pointer);
	pointer = NULL;
	return 0;
}

template<typename tElement>
int SmartMove(std::vector<tElement> &bufferVector, tElement* &pointer, int &totNum)
{
	totNum = (int)bufferVector.size();
	if (pointer != NULL)
	{
		std::cerr << "ERROR in SmartMove !" << std::endl;
		return -1;
	}
	if (totNum > 0)
	{
		pointer = Malloc(tElement, totNum);
		for (int i = 0; i < totNum; i++)
			memcpy(&pointer[i], &bufferVector[i], sizeof(tElement));
		bufferVector.clear();
	}
	else pointer = NULL;
	return 0;
}

template<typename tFirst, typename tSecond>
bool CmpPairByLagerSecond(std::pair<tFirst, tSecond> pairA, std::pair<tFirst, tSecond> pairB)
{
	return pairA.second > pairB.second;
}

template<typename tFirst, typename tSecond>
bool CmpPairBySmallerSecond(std::pair<tFirst, tSecond> pairA, std::pair<tFirst, tSecond> pairB)
{
	return pairA.second < pairB.second;
}

template<typename tFirst, typename tSecond>
bool CmpPairByLagerFirst(std::pair<tFirst, tSecond> pairA, std::pair<tFirst, tSecond> pairB)
{
	return pairA.first > pairB.first;
}

template<typename tFirst, typename tSecond>
bool CmpPairBySmallerFirst(std::pair<tFirst, tSecond> pairA, std::pair<tFirst, tSecond> pairB)
{
	return pairA.first < pairB.first;
}

template<typename tData>
bool CmpDataLarger(const tData& dataA, const tData& dataB)
{
	return dataA > dataB;
}

template<typename tData>
bool CmpDataSmaller(const tData& dataA, const tData& dataB)
{
	return dataA < dataB;
}

template<typename tFirst, typename tSecond>
int VectorVecPairToVectorMap(std::vector<std::vector<std::pair<tFirst, tSecond>>>& vecVecPair, std::vector<std::map<tFirst, tSecond>>& vecMap)
{
	int rtn = 0;
	vecMap.clear();
	vecMap.resize(vecVecPair.size());
	for (size_t i = 0; i < vecVecPair.size(); ++i)
	{
		for (size_t j = 0; j < vecVecPair[i].size(); ++j)
		{
			vecMap[i][vecVecPair[i][j].first] = vecVecPair[i][j].second;
		}
	}
	return 0;
}

template<typename tFirst, typename tSecond>
int VectorPairToMap(std::vector<std::pair<tFirst, tSecond>>& vecPair, std::map<tFirst, tSecond>& mapData)
{
	int rtn = 0;
	mapData.clear();
	for (size_t i = 0; i < vecPair.size(); ++i)
	{
		mapData[vecPair[i].first] = vecPair[i].second;
	}
	return 0;
}

template<typename tData>
int SetToVector(const std::set<tData>& setData, std::vector<tData>& vecData)
{
	vecData.clear();
	for (typename std::set<tData>::const_iterator it = setData.cbegin(); it != setData.cend(); ++it)
	{
		vecData.push_back(*it);
	}
	return 0;
}


template<typename tData>
int VectorToSet(const std::vector<tData>& vecData, std::set<tData>& setData)
{
	setData.clear();
	for (size_t i = 0; i < vecData.size(); ++i)
	{
		setData.insert(vecData[i]);
	}
	return 0;
}

//binary search the vector<pair<tFirst,tSecond>>, the vector must sort by pair.first ascendingly, if not find the element, the retIndex equals to vecPair.size()
template<typename tFirst, typename tSecond>
int FindAscdVecPair(const std::vector<std::pair<tFirst, tSecond>>& vecPair, const tFirst& value, size_t& retIndex)
{
	retIndex = vecPair.size();
	size_t l = 0;
	size_t r = vecPair.size();
	while (l <= r)
	{
		size_t mid = (l + r) >> 1;
		if (vecPair[mid].first < value)
			l = mid + 1;
		else if (vecPair[mid].first > value)
			r = mid - 1;
		else
		{
			retIndex = mid;
			break;
		}
	}
	return 0;
}

#endif