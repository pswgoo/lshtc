#ifndef STRING_UTILITY_H
#define STRING_UTILITY_H
#include "common_basic.h"
#include <sstream>
#include <string>
#include <vector>

int ReadUntil(FILE* inFile, char* &retString, char separator);

int GetElement(FILE* inFile, char* &element);

int GetContent(FILE* inFile, char* &content);

int GetContent(FILE* inFile, int *num);

int GetFirstElement(FILE* inFile, char* &element);

bool CmpHead(char *element, const char* head);

int Pending(FILE* inFile, const char* keyString);

char ToLow(char tempC);

int ToLow(std::string& origText);

bool IsLetter(char chr);

bool IsPunctuation(char chr);

bool IsNumber(char chr);

template<typename tData>
int DataToString(tData oriData, std::string& retStr)
{
	retStr.clear();
	std::stringstream ss;
	ss << oriData;
	ss >> retStr;
	return 0;
}

template<typename tData>
inline int StringToData(const std::string& oriStr, tData& retData)
{
	std::stringstream ss;
	ss << oriStr;
	ss >> retData;
	return 0;
}

// Specialize for StringToData<int>
template<>
inline int StringToData(const std::string &oriStr, int &retData) {
	int r = 0;
	bool neg = false;
	const char *p = oriStr.c_str();

	while (*p == ' ') ++p;

	if (*p == '-') {
		neg = true;
		++p;
	}
	while (*p >= '0' && *p <= '9') {
		r = r * 10 + *p - '0';
		++p;
	}

	if (*p != 0 && *p != ' ' && *p != '\r' && *p != '\n' && *p != '\t') {
		std::clog << "Warning: Invalid int " << oriStr << ", using default StringToData instead." << std::endl;
		// using default StringToData
		std::stringstream ss;
		ss << oriStr;
		ss >> retData;
	} else {
		if (neg) {
			r = -r;
		}
		retData = r;
	}

	return 0;
}

// Specialize for StringToData<double>
template<>
inline int StringToData(const std::string &oriStr, double& retData) {
	double r = 0.0;
	bool neg = false;
	const char *p = oriStr.c_str();
	
	while (*p == ' ') ++p;

	if (*p == '-') {
		neg = true;
		++p;
	}
	while (*p >= '0' && *p <= '9') {
		r = r * 10.0 + *p - '0';
		++p;
	}
	if (*p == '.') {
		double f = 0.0, power = 1;
		++p;
		while (*p >= '0' && *p <= '9') {
			f = f * 10.0 + *p - '0';
			++p;
			power *= 10;
		}
		r += f / power;
	}

	if (*p != 0 && *p != ' ' && *p != '\r' && *p != '\n' && *p != '\t') {
		if (*p != 'e' && *p != 'E') { // ignore the case `5.03210000480371e-313`.
			std::clog << "Warning: Invalid double " << oriStr << ", using default StringToData instead." << std::endl;
		}
		// using default StringToData
		std::stringstream ss;
		ss << oriStr;
		ss >> retData;
	} else {
		if (neg) {
			r = -r;
		}
		retData = r;
	}

	return 0;
}


int SplitString(const std::string& oriStr, char separator, std::vector<std::string>& vecStr);

int SplitString(const char* oriStr, char separator, std::vector<std::string>& vecStr);

int ReadLine(FILE* inFile, int maxLen, char* const line);

#endif //STRING_UTILITY_H