#ifndef TOKENIZATION_H
#define TOKENIZATION_H
#include "medline/basic.h"
#include "medline/citation.h"
#include "evaluation/tools.h"
typedef	std::vector<std::vector<int> > Tokens;
#define MAXLINE 100000

std::string GetLine(char* buffer, FILE* file);
std::vector<std::string> SplitString(const std::string &string);



struct Word
{
	std::string mStr = "";
	int mId;
	int mAppear; // Count only Once for Each Citation !
};
class Dictionary
{
public:
	std::map<int, Word*> mIdToWords;
	std::map<std::string, Word*> mStringToWords;

public:
	Dictionary();
	~Dictionary();

	//Load means load dictionary bin file
	//Build means Build a Dictionary from citation tokens files(.txt)

	//return NULL if can't find the word
	Word* operator[](int index);

	Word* operator[](std::string str);

	//fill id -1 in vector<int> &tokenVector if can't find the word
	int ParseSentence(std::vector<std::string> &sentence, std::vector<int> &tokensVector);

	int Destroy();

	int Load(FILE* binFile, int printLog = SILENT);

	int Load(const char* const fileName, int printLog = SILENT);

	int Save(FILE* binFile, int printLog = SILENT);

	int Save(const char* const fileName, int printLog = SILENT);

	//write to a text file for reading/debugging
	int PrintText(const char* const fileName);

	int AddNewWord(const std::string &string);

	int AddNewSentence(const std::vector<std::string> &sentence);

	// BuildIn is adding, not reconstruction
	int BuildIn(FILE* txtFile, int printLog = SILENT);

	int BuildIn(const char* const fileName, int printLog = SILENT);

	//parameters in this function designed for the real requirements
	//TokensFile are stored in filename from  "./resultCitationTokens/relsult_Citationtokens_tmp0.txt" to "./resultCitationTokens/relsult_Citationtokens_tmp47.txt"
	//In above condition filePath = ".//resultCitationTokens/relsult_Citationtokens_tmp" filePathTail = ".txt" idStart = 0; idEnd = 47;
	int Build(const char* const filePathHead, const char* filePathTail, const int idStart, const int idEnd, int printLog = SILENT);
};

class TokenCitation
{
public:
	int mPmid;
	Tokens mArticleTitle;
	Tokens mJournalTitle;
	Tokens mAbstract;

	TokenCitation();

	int Save(FILE* binFile, int printLog = SILENT);

	int Save(const char* const fileName, int printLog = SILENT);

	int Load(FileBuffer* buffer, int printLog = SILENT);

	static int CountWords(Tokens &tokens);

	int CountWords();

	//FileBUffer can directly write/load int data. That means IntToString/StringToInt functions are Useless here.
	//include evaluation/tools.h for these functions if still needed
};

class TokenCitationSet
{
public:
	std::map<int, TokenCitation*> mTokenCitations;
	//Dictionary is a inherent attribute for TokenCitationSet
	Dictionary* mDictionary;
public:
	TokenCitationSet();
	~TokenCitationSet();

	int Size();

	int Destroy();

	TokenCitation* operator[](int index);

	//fileName for dictionary;
	TokenCitationSet(const char* const fileName);

	int LoadDictionary(const char* const fileName, int printLog = SILENT);


	//Load means load TokenCitationSet bin file
	//Build means Build a TokenCitaionSet from citation tokens files(.txt) use mDictionary
	//Check mDictionary!=Null first when build a TokenCitationSet

	int Load(FILE* binFile, int printLog = SILENT);

	int Load(const char* const fileName, int printLog = SILENT);

	int Save(FILE* binFile, int printLog = SILENT);

	int Save(const char* const fileName, int printLog = SILENT);

	//write to a text file for reading/debugging
	int PrintTokens(FILE* file, Tokens &tokens);

	int PrintText(const char* const fileName);

	std::vector<int> GetSentence(char* buffer, FILE* txtFile);

	int BuildIn(FILE* txtFile, int printLog = SILENT);

	int BuildIn(const char* const fileName, int printLog = SILENT);


	//parameters in this function designed for the real requirements
	//TokensFile are stored in filename from  "./resultCitationTokens/relsult_Citationtokens_tmp0.txt" to "./resultCitationTokens/relsult_Citationtokens_tmp47.txt"
	//In above condition filePath = ".//resultCitationTokens/relsult_Citationtokens_tmp" filePathTail = ".txt" idStart = 0; idEnd = 47;
	int Build(const char* const filePathHead, const char* filePathTail, const int idStart, const int idEnd, int printLog = SILENT);


};

#endif /*TOKENIZATION_H*/