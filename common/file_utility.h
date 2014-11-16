#ifndef FILE_UTILITY_H
#define FILE_UTILITY_H
#include "common_basic.h"

class FileBuffer
{
private:
	char* mData;
	long long mSize;
	long long mCur;
public:
	FileBuffer();
	
	FileBuffer(long long size);
	
	FileBuffer(FILE* binFile);
	
	FileBuffer(const char* const fileName);
	
	long long GetSize();
	
	long long GetCur();
	
	bool Eof();
	
	int Seek(const long long pos, const int from); //from == 0 Start; from ==1 End; from == 2 mCur;

template<typename tData>
int GetNextData(tData &data)
{
	memcpy(&data, mData + mCur, sizeof(data));
	mCur += sizeof(data);
	return 0;
}

	int GetNextData(char* &str);
	
	int GetNextData(std::string &str);

template<typename tFirst, typename tSecond>
int GetNextData(std::map <tFirst, tSecond> &mapData)
{
	int rtn = 0;
	mapData.clear();
	size_t size;
	rtn = GetNextData(size);
	CHECK_RTN(rtn);
	for (size_t i = 0; i < size; i++)
	{
		tFirst first;
		tSecond second;
		rtn = GetNextData(first);
		CHECK_RTN(rtn);
		rtn = GetNextData(second);
		CHECK_RTN(rtn);
		mapData[first] = second;
	}
	return 0;
}

template<typename tData>
int GetNextData(std::vector<tData> &vectorData)
{
	int rtn = 0;
	size_t size;
	rtn = GetNextData(size);
	CHECK_RTN(rtn);
	vectorData.resize(size);
	for (size_t i = 0; i < size; i++)
	{
		rtn = GetNextData(vectorData[i]);
		CHECK_RTN(rtn);
	}
	return 0;
}

template<typename tFirst, typename tSecond>
int GetNextData(std::pair<tFirst, tSecond>& pairData)
{
	int rtn = 0;
	rtn = GetNextData(pairData.first);
	CHECK_RTN(rtn);
	rtn = GetNextData(pairData.second);
	CHECK_RTN(rtn);
	return 0;
}

template <typename tData>
int GetNextData(std::set<tData>& setData)
{
	int rtn = 0;
	size_t length;
	rtn = GetNextData(length);
	CHECK_RTN(rtn);
	for (size_t i = 0; i < length; ++i)
	{
		tData key;
		rtn = GetNextData(key);
		CHECK_RTN(rtn);
		setData.insert(key);
	}
	return 0;
}

	int SkipNextChar();
	
	int SkipNextBool();
	
	int SkipNextInt();
	
	int SkipNextDouble();
	
	int SkipNextString();
	
	int Destroy();
	
	~FileBuffer();
};

long long GetFileLength(FILE* file);

long long GetFileLength(const char* const fileName);

template <typename tData>
int Write(FILE* outFile, tData data)
{
	fwrite(&data, sizeof(data), 1, outFile);
	return 0;
}

int Write(FILE* outFile, char* str);

int Write(FILE* outFile, const char* str);

int Write(FILE* outFile, std::string str);

template <typename tFirst, typename tSecond>
int Write(FILE* outFile, const std::pair<tFirst, tSecond>& pairData)
{
	int rtn = 0;
	rtn = Write(outFile, pairData.first);
	CHECK_RTN(rtn);
	rtn = Write(outFile, pairData.second);
	CHECK_RTN(rtn);
	return 0;
}

template <typename tFirst, typename tSecond>
int Write(FILE* outFile, const std::map<tFirst, tSecond>& mapData)
{
	int rtn = 0;
	rtn = Write(outFile, mapData.size());
	CHECK_RTN(rtn);
	for (typename std::map<tFirst, tSecond>::const_iterator it = mapData.cbegin(); it != mapData.cend(); it++)
	{
		rtn = Write(outFile, it->first);
		CHECK_RTN(rtn);
		rtn = Write(outFile, it->second);
		CHECK_RTN(rtn);
	}
	return 0;
}

template <typename tData>
int Write(FILE* outFile, const std::vector<tData>& vectorData)
{
	int rtn = 0;
	rtn = Write(outFile, vectorData.size());
	CHECK_RTN(rtn);
	for (size_t i = 0; i < vectorData.size(); i++)
	{
		rtn = Write(outFile, vectorData[i]);
		CHECK_RTN(rtn);
	}
	return 0;
}

template <typename tData>
int Write(FILE* outFile, const std::set<tData>& setData)
{
	int rtn = 0;
	rtn = Write(outFile, setData.size());
	CHECK_RTN(rtn);
	for (typename std::set<tData>::const_iterator it = setData.cbegin(); it != setData.cend(); ++it)
	{
		rtn = Write(outFile, *it);
		CHECK_RTN(rtn);
	}
	return 0;
}

template<typename tData>
int Read(FILE* inFile, tData &data)
{
	if (fread(&data, sizeof(data), 1, inFile) > 0)
		return 0;
	else
		return -1;
}

int Read(FILE* inFile, char* &str);

int Read(FILE* inFile, std::string &str);

template <typename tFirst, typename tSecond>
int Read(FILE* inFile, std::pair<tFirst, tSecond>& pairData)
{
	int rtn = 0;
	rtn = Read(inFile, pairData.first);
	CHECK_RTN(rtn);
	rtn = Read(inFile, pairData.second);
	CHECK_RTN(rtn);
	return 0;
}

template <typename tFirst, typename tSecond>
int Read(FILE* inFile, std::map<tFirst, tSecond>& mapData)
{
	int rtn = 0;
	mapData.clear();
	size_t size = 0;
	Read(inFile, size);
	CHECK_RTN(rtn);
	for (size_t i = 0; i < size; i++)
	{
		tFirst first;
		tSecond second;
		rtn = Read(inFile, first);
		CHECK_RTN(rtn);
		rtn = Read(inFile, second);
		CHECK_RTN(rtn);
		mapData[first] = second;
	}
	return 0;
}

template <typename tData>
int Read(FILE* inFile, std::vector<tData>& vectorData)
{
	int rtn = 0;
	vectorData.clear();
	size_t size;
	rtn = Read(inFile, size);
	CHECK_RTN(rtn);
	vectorData.resize(size);
	for (size_t i = 0; i < size; i++)
	{
		rtn = Read(inFile, vectorData[i]);
		CHECK_RTN(rtn);
	}
	return 0;
}

template <typename tData>
int Read(FILE* inFile, std::set<tData>& setData)
{
	int rtn = 0;
	setData.clear();
	size_t length;
	rtn = Read(inFile, length);
	CHECK_RTN(rtn);
	for (size_t i = 0; i < length; ++i)
	{
		tData tmp;
		rtn = Read(inFile, tmp);
		CHECK_RTN(rtn);
		setData.insert(tmp);
	}
	return 0;
}

template<typename tData>
int WriteFile(std::string fileName, const tData& data)
{
	int rtn = 0;
	FILE *outFile = fopen(fileName.c_str(), "wb");
	rtn = Write(outFile, data);
	CHECK_RTN(rtn);
	fclose(outFile);
	return 0;
}

template<typename tData>
int ReadFile(std::string fileName, tData& data)
{
	int rtn = 0;
	FILE *inFile = fopen(fileName.c_str(), "rb");
	rtn = Read(inFile, data);
	CHECK_RTN(rtn);
	fclose(inFile);
	return 0;
}

bool FileExist(std::string filePath);

#endif