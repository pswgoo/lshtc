#include "file_utility.h"
using namespace std;

FileBuffer::FileBuffer()
{
	mData = NULL;
	mSize = 0;
	mCur = 0;
}

FileBuffer::FileBuffer(long long size)
{
	mData = NULL;
	mSize = 0;
	mCur = 0;
	if (size > 0)
	{
		mData = Malloc(char, (size_t)size);
		mSize = size;
	}
}

FileBuffer::FileBuffer(FILE* binFile)
{
	mCur = 0;
	mSize = GetFileLength(binFile);
	mData = Malloc(char, (size_t)mSize);
	fread(mData, 1, (size_t)mSize, binFile);
}

FileBuffer::FileBuffer(const char* const fileName)
{

	FILE* binFile = fopen(fileName, "rb");
	mCur = 0;
	mSize = GetFileLength(binFile);
	mData = Malloc(char, (size_t)mSize);
	fread(mData, 1, (size_t)mSize, binFile);
	fclose(binFile);
}

long long FileBuffer::GetSize()
{
	return mSize;
}

long long FileBuffer::GetCur()
{
	return mCur;
}

bool FileBuffer::Eof()
{
	return mCur >= mSize;
}

int FileBuffer::Seek(const long long pos, const int from)
{
	if (from > 2 || from < 0)
		return -1;
	long long nowPos = 0;
	if (from == 1)
		nowPos = mSize;
	else if (from == 2)
		nowPos = mCur;
	if (pos + nowPos >= mSize || pos + nowPos < 0)
		return -2;
	mCur = pos + nowPos;
	return 0;
}

int FileBuffer::GetNextData(char* &str)
{
	int length;
	GetNextData(length);
	if (length > 0)
	{
		str = Malloc(char, length + 1);
		memcpy(str, (char*)mData + mCur, (size_t)length);
		str[length] = '\0';
		mCur += length;
	}
	else
		str = NULL;
	return 0;
}

int FileBuffer::GetNextData(string &str)
{
	int rtn = 0;
	int length;
	rtn = GetNextData(length);
	CHECK_RTN(rtn);
	if (length > 0)
	{
		char* tmpString = Malloc(char, length + 1);
		memcpy(tmpString, (char*)mData + mCur, (size_t)length);
		tmpString[length] = '\0';
		str = tmpString;
		rtn = SmartFree(tmpString);
		CHECK_RTN(rtn);
		mCur += length;
	}
	else
		str = "";
	return 0;
}

int FileBuffer::SkipNextChar()
{
	mCur += sizeof(char);
	return 0;
}

int FileBuffer::SkipNextBool()
{
	mCur += sizeof(bool);
	return 0;
}

int FileBuffer::SkipNextInt()
{
	mCur += sizeof(int);
	return 0;
}

int FileBuffer::SkipNextDouble()
{
	mCur += sizeof(double);
	return 0;
}

int FileBuffer::SkipNextString()
{
	int length = 0;
	GetNextData(length);
	mCur += length;
	return 0;
}

int FileBuffer::Destroy()
{
	SmartFree(mData);
	mSize = mCur = 0;
	return 0;
}

FileBuffer::~FileBuffer()
{
	Destroy();
}

long long GetFileLength(FILE* file)
{
	long long bytes;
#ifdef __GNUC__
	fseek(file, 0, SEEK_END);
	bytes = ftell(file);
#else
	_fseeki64(file, 0, SEEK_END);
	bytes = _ftelli64(file);
#endif
	fseek(file, 0, SEEK_SET);
	return bytes;
}

long long GetFileLength(const char* const fileName)
{
	FILE* file = fopen(fileName, "rb");
	long long rtn = GetFileLength(file);
	fclose(file);
	return rtn;
}

int Write(FILE* outFile, char* str)
{
	int length = 0;
	if (str) length = (int)strlen(str);
	fwrite(&length, sizeof(int), 1, outFile);
	if (length > 0)
		fwrite(str, sizeof(char), length, outFile);
	return 0;
}

int Write(FILE* outFile, const char* str)
{
	int length = 0;
	if (str) length = (int)strlen(str);
	fwrite(&length, sizeof(int), 1, outFile);
	if (length > 0)
		fwrite(str, sizeof(char), length, outFile);
	return 0;
}

int Write(FILE* outFile, string str)
{
	int length = (int)str.length();
	fwrite(&length, sizeof(int), 1, outFile);
	if (length > 0)
		fwrite(str.c_str(), sizeof(char), length, outFile);
	return 0;
}

int Read(FILE* inFile, char* &str)
{
	int length = 0;
	if (fread(&length, sizeof(int), 1, inFile) < 1)
		return -1;
	if (length > 0)
	{
		str = Malloc(char, length + 1);
		fread(str, sizeof(char), length, inFile);
		str[length] = '\0';
	}
	else
		str = NULL;
	return 0;
}

int Read(FILE* inFile, string &str)
{
	int length = 0;
	if (fread(&length, sizeof(int), 1, inFile) < 1)
		return -1;
	if (length > 0)
	{
		char *tmpString;
		tmpString = Malloc(char, length + 1);
		fread(tmpString, sizeof(char), length, inFile);
		tmpString[length] = '\0';
		str = tmpString;
		int rtn = SmartFree(tmpString);
		CHECK_RTN(rtn);
	}
	else
		str = "";
	return 0;
}

bool FileExist(string filePath)
{
	FILE* fp = fopen(filePath.c_str(), "rb");
	bool rtn = false;
	if (fp != NULL)
	{
		rtn = true;
		fclose(fp);
	}
	return rtn;
}
