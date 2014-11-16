#include"basic.h"
using namespace std;

bool CmpStringList(const std::string &str, const std::vector<std::string> &subStringList, const int mod)
{
	if (mod == MATCH)
	{
		for (unsigned int i = 0; i < subStringList.size(); i++)
		if (str == subStringList[i])
			return true;
		return false;
	}
	else if (mod == INCLUDE_ANY)
	{
		for (unsigned int i = 0; i < subStringList.size(); i++)
		if (mod && str.find(subStringList[i]) != std::string::npos)
			return true;
		return false;
	}
	else if (mod == INCLUDE_ALL)
	{
		for (unsigned int i = 0; i < subStringList.size(); i++)
		if (mod && str.find(subStringList[i]) == std::string::npos)
			return false;
		return true;
	}
	return false;
}

char Escaping(char* seq)
{
	if (0 == strcmp(seq, "&lt;"))
		return '<';
	else if (0 == strcmp(seq, "&gt;"))
		return '>';
	else if (0 == strcmp(seq, "&amp;"))
		return '&';
	else if (0 == strcmp(seq, "&apos;"))
		return '\'';
	else if (0 == strcmp(seq, "&quot;"))
		return '\"';
	else
		return 0;
}

int EscapingStrCpy(char* dest, char* sour)
{
	if (dest == NULL)
		return 0;
	if (sour == NULL)
		sour = dest;
	int sLen = 0, dLen = 0;
	while (sour[sLen] != '\0')
	{
		if (sour[sLen] == '&')
		{
			char tStr[10];
			int tCur = 0;
			while (sour[sLen] != ';' && tCur<6 && sour[sLen] != '\0')
				tStr[tCur++] = sour[sLen++];
			tStr[tCur++] = sour[sLen];
			tStr[tCur] = '\0';
			if (sour[sLen] != '\0')
				++sLen;
			char ch = Escaping(tStr);
			if (ch != 0)
			{
				dest[dLen++] = ch;
			}
			else
			{
				for (int t = 0; tStr[t] != '\0'; ++t)
					dest[dLen++] = tStr[t];
			}
		}
		else
			dest[dLen++] = sour[sLen++];
	}
	dest[dLen] = '\0';
	return dLen;
}

int CpyString(char* &dest, const std::string& sour)
{
	int len = (int)sour.size();
	dest = Malloc(char, len + 1);
	strcpy(dest, sour.c_str());
	return 0;
}

int NextBracket(FileBuffer &buffer, const char left, const char right, std::string& seq)
{
	seq.clear();
	char ch;
	while (!buffer.Eof())
	{
		buffer.GetNextData(ch);
		if (ch == left) break;
	}
	if (buffer.Eof())
		return -1;

	seq += ch;
	int cnt = 1;
	while (cnt != 0)
	{
		if (buffer.Eof())
		{
			seq.clear();
			return -1;
		}
		buffer.GetNextData(ch);
		seq += ch;
		if (ch == left)
			++cnt;
		if (ch == right)
			--cnt;
	}
	return 0;
}

int PutJsonEscapes(FILE* outFile, const char* str)
{
	int pos = 0;
	while (str[pos] != '\0')
	{
		char ch = str[pos++];
		if (ch == '"' || ch == '\\' || ch == '/')
		{
			fputc('\\', outFile);
		}
		fputc(ch, outFile);
	}
	return 0;
}

int NextJsonObject(FileBuffer &buffer, std::string& seq)
{
	seq.clear();
	char left = '{';
	char right = '}';
	char innerChar = '"';
	char excape = '\\';
	bool bEnterInner = false;

	char ch;
	while (!buffer.Eof())
	{
		buffer.GetNextData(ch);
		if (ch == left) break;
	}
	if (buffer.Eof())
		return -1;

	seq += ch;
	int cnt = 1;
	while (cnt != 0)
	{
		if (buffer.Eof())
		{
			seq.clear();
			return -1;
		}
		
		buffer.GetNextData(ch);
		seq += ch;

		if (ch == innerChar)
			bEnterInner = !bEnterInner;
		if (ch == excape)
		{
			if (buffer.Eof())
			{
				seq.clear();
				return -1;
			}
			buffer.GetNextData(ch);
			seq += ch;
		}

		if (!bEnterInner && ch == left)
			++cnt;
		if (!bEnterInner && ch == right)
			--cnt;
	}
	return 0;
}

bool CmpScore(pair<int, double>& x, pair<int, double>& y)
{
	return x.second > y.second;
}
