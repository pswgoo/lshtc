#include "lshtc_data.h"
#include "common/file_utility.h"
#include "common/common_basic.h"
using namespace std;
LhtcDocument::LhtcDocument()
{
	Clear();
}

LhtcDocument::~LhtcDocument()
{
}

int LhtcDocument::Clear()
{
	mId = 0;
	mLabels.clear();
	mTf.clear();
	return 0;
}

int LhtcDocument::Load(FILE* infile)
{
	char tmp = 0;
	bool flag = 1;
	while (flag)
	{
		int tmpX = 0;
		while ((tmp = fgetc(infile)) != EOF)
		{
			if (tmp < '0' || tmp > '9') break;
			tmpX = tmpX * 10 + tmp - '0';
		}
		if (tmp != ',') flag = 0;
		else tmp = fgetc(infile);
		mLabels.push_back(tmpX);
		if (tmp == -1) return -1;
		if (NULL == infile) flag = 0;
	}//get Label

	flag = 1;
	while (flag)
	{
		int tmpX = 0;
		double tmpY = 0.0;
		while ((tmp = fgetc(infile)) != EOF)
		{
			if (tmp < '0' || tmp > '9') break;
			tmpX = tmpX * 10 + tmp - '0';
		}//get feature
		
		while ((tmp = fgetc(infile)) != EOF)
		{
			if (tmp < '0' || tmp > '9') break;
			tmpY = tmpY * 10 + tmp - '0';
		}
		if (tmp == '.')
		{
			double tmpEps = 1.0;
			while ((tmp = fgetc(infile)) != EOF)
			{
				if (tmp < '0' || tmp > '9') break;
				tmpEps *= 0.1;
				tmpY += 1. * tmpEps * (tmp - '0');
			}
		}//get value
		
		if (tmp != ' ') flag = 0;
		if (tmp == -1) return -1;
		mTf.insert(std::pair<int, double>(tmpX, tmpY));
		if (NULL == infile) flag = 0;
	}
	return 0;
}

int LhtcDocument::Save(FILE* outfile)
{
	int labelSize;
	std::map<int, double>::iterator iterTF;
	labelSize = mLabels.size();
	for (int i = 0; i + 1 < labelSize; i++) fprintf(outfile, "%d, ", mLabels[i]);
	fprintf(outfile, "%d", mLabels[labelSize - 1]);

	for (iterTF = mTf.begin(); iterTF != mTf.end(); ++iterTF)
		fprintf(outfile, " %d:%d", iterTF->first, int(iterTF->second));
	fprintf(outfile, "\n");
	return 0;
}

int LhtcDocument::LoadBin(FILE* inFile)
{
	int rtn = 0;
	rtn = Read(inFile, mId);
	CHECK_RTN(rtn);

	rtn = Read(inFile, mLabels);
	CHECK_RTN(rtn);

	rtn = Read(inFile, mTf);
	CHECK_RTN(rtn);

	return 0;
}

int LhtcDocument::SaveBin(FILE* outFile)
{
	int rtn = 0;
	rtn = Write(outFile, mId);
	CHECK_RTN(rtn);

	rtn = Write(outFile, mLabels);
	CHECK_RTN(rtn);

	rtn = Write(outFile, mTf);
	CHECK_RTN(rtn);
	return 0;
}

LhtcDocumentSet::LhtcDocumentSet()
{
	mLhtcDocuments.clear();
}

LhtcDocumentSet::~LhtcDocumentSet()
{

}

LhtcDocument LhtcDocumentSet::operator[](int index)
{
	return mLhtcDocuments[index];
}

int LhtcDocumentSet::Size()
{
	return (int)mLhtcDocuments.size();
}

int LhtcDocumentSet::Load(const char* fileName, int printLog)
{
	int rtn = 0;
	FILE* inFile = fopen(fileName, "r");
	if (NULL == inFile) return 1;
	rtn = Load(inFile);
	CHECK_RTN(rtn);
	fclose(inFile);
	return 0;
}

int LhtcDocumentSet::Save(const char* filename, int pringLog)
{
	int rtn = 0;
	FILE* outfile = fopen(filename, "w");
	if (NULL == outfile) return 1;
	rtn = Save(outfile);
	CHECK_RTN(rtn);
	fclose(outfile);
	return 0;
}

int LhtcDocumentSet::Load(FILE* infile, int printLog)
{
	printLog = FULL_LOG;
	int rtn = 0;
	int loadCnt = 0;
	mLhtcDocuments.clear();
	if (printLog == FULL_LOG)
		printf("Load LhtcDocument");
	while (NULL != infile)
	{
		LhtcDocument tempHtc;
		tempHtc.Clear();
		if (tempHtc.Load(infile) == -1) break;
		tempHtc.mId = ++loadCnt;
		mLhtcDocuments.insert(std::pair<int, LhtcDocument>(tempHtc.mId, tempHtc));
		if (printLog == FULL_LOG)
			if ((loadCnt & 32767) == 0) printf("\n%d LhtcDocuments Load", loadCnt);
	}
	if (printLog == FULL_LOG) printf("\n%d LhtcDocuments Load\n", loadCnt);
	return 0;
}

int LhtcDocumentSet::Save(FILE* outfile, int printLog)
{
	int rtn = 0;
	int saveCnt = 0;
	if (printLog == FULL_LOG)
		printf("Save LhtcDocument");
	std::map<int, LhtcDocument>::iterator iterTF;
	for (iterTF = mLhtcDocuments.begin(); iterTF != mLhtcDocuments.end(); ++iterTF)
	{
		++saveCnt;
		rtn = iterTF->second.Save(outfile);
		CHECK_RTN(rtn);
		if (printLog == FULL_LOG)
		if ((saveCnt & 32767) == 0) printf("\n%d LhtcDocuments Save", saveCnt);
	}
	if (printLog == FULL_LOG) printf("\n%d LhtcDocuments Save\n", saveCnt);
	return 0;
}

int LhtcDocumentSet::SaveBin(std::string fileName, int printLog)
{
	FILE *outFile = fopen(fileName.c_str(), "wb");
	size_t size = mLhtcDocuments.size();
	Write(outFile, size);
	int cnt = 0;
	for (map<int, LhtcDocument>::iterator it = mLhtcDocuments.begin(); it != mLhtcDocuments.end(); ++it)
	{
		Write(outFile, it->first);
		it->second.SaveBin(outFile);
		++cnt;
	}
	fclose(outFile);
	if (printLog != SILENT)
		clog << "Total save " << cnt << " documents" << endl;
	return 0;
}

int LhtcDocumentSet::LoadBin(std::string fileName, int printLog)
{
	mLhtcDocuments.clear();
	FILE *inFile = fopen(fileName.c_str(), "rb");
	size_t size = 0;
	Read(inFile, size);
	for (size_t i = 0; i < size; ++i)
	{
		int id;
		Read(inFile, id);
		LhtcDocument tmpDoc;
		tmpDoc.LoadBin(inFile);
		mLhtcDocuments[id] = tmpDoc;
	}
	fclose(inFile);
	if (printLog != SILENT)
		clog << "Total load " << mLhtcDocuments.size() << " documents" << endl;
	return 0;
}
