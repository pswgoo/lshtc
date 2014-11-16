#include "string_utility.h"
#include <string>
using namespace std;

int ReadUntil(FILE* inFile, char* &retString, char separator)
{
	char chr;
	string readBuffer;
	while (fread(&chr, 1, 1, inFile) && chr != separator)
		readBuffer += chr;
	if (readBuffer.length() > 0)
	{
		char* ret = Malloc(char, readBuffer.length() + 1);
		strcpy(ret, readBuffer.c_str());
		ret[readBuffer.length()] = '\0';
		retString = ret;
	}
	else retString = NULL;
	return 0;
}

int GetElement(FILE* inFile, char* &element)
{
	return ReadUntil(inFile, element, '>');
}

int GetContent(FILE* inFile, char* &content)
{
	return ReadUntil(inFile, content, '<');
}

int GetContent(FILE* inFile, int *num)
{
	int rtn = 0;
	char* buffer = NULL;
	rtn = GetContent(inFile, buffer);
	CHECK_RTN(rtn);
	sscanf(buffer, "%d", num);
	rtn = SmartFree(buffer);
	CHECK_RTN(rtn);
	return 0;
}

int GetFirstElement(FILE* inFile, char* &element)
{
	int rtn = 0;
	char chr = ' ';
	element = NULL;
	while (element == NULL || element[strlen(element) - 1] == '/')
	{
		while (fread(&chr, 1, 1, inFile))
		if (chr == '<')
			break;
		if (chr != '<')
			return 1;
		rtn = GetElement(inFile, element);
		CHECK_RTN(rtn);
	}
	return 0;
}

bool CmpHead(char *element, const char* head)
{
	size_t headLen = strlen(head);
	for (int i = 0; i < headLen; i++)
	{
		if (!(element[i]))
			return false;
		if (element[i] != head[i])
			return false;
	}
	return true;
}

int Pending(FILE* inFile, const char* keyString)
{
	int rtn = 0;
	size_t length = strlen(keyString);
	char chr;
	char* element = NULL;
	while (fread(&chr, 1, 1, inFile))
	if (chr == '<')
	{
		rtn = GetElement(inFile, element);
		CHECK_RTN(rtn);
		if (CmpHead(element, keyString))
		{
			rtn = SmartFree(element);
			CHECK_RTN(rtn);
			return 0;
		}
		rtn = SmartFree(element);
		CHECK_RTN(rtn);
	}
	return 1;
}

inline char ToLow(char tempC)
{ 
	return (tempC >= 'A' && tempC <= 'Z') ? tempC + 32 : tempC; 
}

int ToLow(std::string& origText)
{
	size_t lenText = origText.size();
	for (size_t i = 0; i < lenText; i++) 
		origText[i] = ToLow(origText[i]);
	return 0;
}

bool IsLetter(char chr)
{
	return (chr >= 'A' && chr <= 'Z') || (chr >= 'a' && chr <= 'z');
}

bool IsPunctuation(char chr)
{
	string punctuationList = "!()=[]{}\\|;:'\"<>,.?/ ";
	for (size_t i = 0; i < punctuationList.length(); i++)
	if (chr == punctuationList[i])
		return true;
	return false;
}

bool IsNumber(char chr)
{
	return (chr >= '0' && chr <= '9');
}

int SplitString(const string& oriStr, char separator, vector<string>& vecStr)
{
	vecStr.clear();
	string segment;
	for (size_t i = 0; i < oriStr.size(); ++i)
	{
		if (separator == oriStr[i])
		{
			vecStr.push_back(segment);
			segment.clear();
		}
		else
			segment += oriStr[i];
	}
	vecStr.push_back(segment);
	return 0;
}

int SplitString(const char* oriStr, char separator, std::vector<std::string>& vecStr)
{
	if (oriStr == NULL)
		return -1;
	vecStr.clear();
	string segment;
	for (size_t i = 0; oriStr[i] != '\0'; ++i)
	{
		if (separator == oriStr[i])
		{
			vecStr.push_back(segment);
			segment.clear();
		}
		else
			segment += oriStr[i];
	}
	vecStr.push_back(segment);
	return 0;
}

int ReadLine(FILE* inFile, int maxLen, char* const line)
{
	if (feof(inFile) != 0 || line == NULL)
		return -1;
	line[0] = '\0';
	int cur = 0;
	for (cur = 0; cur < maxLen - 1; ++cur)
	{
		char ch = (char)fgetc(inFile);
		if (cur == 0 && feof(inFile) != 0)
			return -1;

		if (feof(inFile) != 0)
			break;
		if (ch == '\n')  //for Linux,Unix
			break;
		if (ch == '\r')
		{
			char oriCh = ch;
			ch = (char)fgetc(inFile);
			if (feof(inFile) != 0 && ch == '\n') //for Windows
				break;
#ifdef __GNUC__
			fseek(inFile, -1L, SEEK_CUR);
#else
			_fseeki64(inFile, -1L, SEEK_CUR);    //for Mac
#endif
			break;
		}
		line[cur] = ch;
	}
	line[cur] = '\0';
	return 0;
}
