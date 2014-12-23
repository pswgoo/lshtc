#ifndef _LHTC_LIB_H
#define _LHTC_LIB_H
#include "common/common_basic.h"

class LhtcDocument
{
public:
	int mId;
	std::vector<int> mLabels;
	std::map<int, double> mTf;

public:
	LhtcDocument();
	~LhtcDocument();

	int Clear();

	int Load(FILE* inFile);

	int LoadBin(FILE* inFile);

	int Save(FILE* outFile);

	int SaveBin(FILE* outFile);
};

class LhtcDocumentSet
{
public:
	std::map<int, LhtcDocument> mLhtcDocuments;
	LhtcDocumentSet();
	~LhtcDocumentSet();

	LhtcDocument& operator[](int index);

	int Size();

	int Save(const char* fileName, int printLog = SILENT);

	int Save(FILE* outFile, int printLog = SILENT);

	int Load(const char* fileName, int printLog = SILENT);

	int Load(FILE* inFile, int printLog = SILENT);

	int SaveBin(std::string fileName, int printLog = SILENT);

	int LoadBin(std::string fileName, int printLog = SILENT);
};

#endif