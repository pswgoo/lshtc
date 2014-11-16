#ifndef _BASIC_H
#define _BASIC_H
#include "common/common_basic.h"
#include "common/file_utility.h"
#include "common/data_struct.h"
#include "common/mymath.h"
#include <cstdlib>
#include <ctime>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <cstring>
#include <algorithm>
#include <vector>
#include <string>
#include <set>
#include <map>

const int MAX_PMID = 30000000;
const int MAX_DESCRIPTOR_UI = 70000;
const int MAX_QUALIFIER_UI = 1000;

bool CmpStringList(const std::string &str, const std::vector<std::string> &subStringList, const int mod);

char Escaping(char* seq);

int EscapingStrCpy(char* dest, char* sour = NULL);

int CpyString(char* &dest, const std::string& sour);

int NextBracket(FileBuffer &buffer, const char left, const char right, std::string& seq);

int PutJsonEscapes(FILE* outFile, const char* str);

int NextJsonObject(FileBuffer& buffer, std::string& seq);

bool CmpScore(std::pair<int, double>& x, std::pair<int, double>& y);


#endif /* _BASIC_H */
