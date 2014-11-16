#include "tokenization.h"
using namespace std;

//getline from file, without the last '\n', eliminate just one blank or one '\n'
std::string GetLine(char* buffer, FILE* file)
{
	fgets(buffer, MAXLINE, file);
	if (strlen(buffer) > 0 && buffer[strlen(buffer) - 1] == '\n')
		buffer[strlen(buffer) - 1] = 0;
	return (std::string)buffer;
}

std::vector<std::string> SplitString(const std::string &string)
{
	std::vector<std::string> vectorString;
	vectorString.clear();
	int n = (int)string.size();
	for (int i = 0; i < n; i++)
	{
		while (i < n && string[i] == ' ') i++;
		std::string word;
		word.clear();
		while (i < n && string[i] != ' ')
		{
			word.push_back(string[i]);
			i++;
		}
		if (word != "") vectorString.push_back(word);
	}
	return vectorString;
}

int Write(FILE* binFile, Tokens &tokens)
{
	int n = (int)tokens.size();
	int rtn = 0;
	rtn = Write(binFile, n);
	CHECK_RTN(rtn);
	for (int i = 0; i < n; i++)
	{
		int m = (int)tokens[i].size();
		rtn = Write(binFile, m);
		CHECK_RTN(rtn);
		for (int j = 0; j < m; j++)
		{
			rtn = Write(binFile, tokens[i][j]);
			CHECK_RTN(rtn);
		}
	}
	return 0;
}

int Read(FileBuffer* buffer, Tokens &tokens)
{
	int n;
	int rtn = 0;
	rtn = buffer->GetNextData(n);
	CHECK_RTN(rtn);
	for (int i = 0; i < n; i++)
	{
		int m;
		int num;
		vector<int> vector;
		vector.clear();
		rtn = buffer->GetNextData(m);
		CHECK_RTN(rtn);
		for (int j = 0; j < m; j++)
		{
			rtn = buffer->GetNextData(num);
			CHECK_RTN(rtn);
			vector.push_back(num);
		}
		tokens.push_back(vector);
	}
	return 0;
}

Dictionary::Dictionary()
{
	mIdToWords.clear();
	mStringToWords.clear();
}

int Dictionary::Destroy()
{
	for (std::map<int, Word*>::iterator ptr = mIdToWords.begin(); ptr != mIdToWords.end(); ptr++)
	{
		delete ptr->second;
	}
	mIdToWords.clear();
	mStringToWords.clear();
	return 0;
}

Dictionary::~Dictionary()
{
	Destroy();
}

Word* Dictionary::operator[](int index)
{
	if (mIdToWords.find(index) == mIdToWords.end())
	{
		return NULL;
	}
	return mIdToWords[index];
}

Word* Dictionary::operator[](std::string str)
{
	if (mStringToWords.find(str) == mStringToWords.end())
	{
		return NULL;
	}
	return mStringToWords[str];
}

int Dictionary::ParseSentence(std::vector<std::string> &sentence, std::vector<int> &tokensVector)
{
	int size = (int)sentence.size();
	tokensVector.clear();
	for (int i = 0; i < size; i++)
	{
		if (mStringToWords.find(sentence[i]) != mStringToWords.end())
		{
			tokensVector.push_back(mStringToWords[sentence[i]]->mId);
		}
		else
			tokensVector.push_back(-1);
	}
	return 0;
}

//格式为：单词，id，出现次数
int Dictionary::Load(FILE* binFile, int printLog)
{
	FileBuffer buffer(binFile);
	char* tWord;
	int id;
	int appear;
	int loadCount = 0;
	Destroy();
	int rtn = 0;
	while (!buffer.Eof())
	{
		Word* ptrWord = new Word;
		rtn = buffer.GetNextData(tWord);

		CHECK_RTN(rtn);
		ptrWord->mStr = (std::string)tWord;

		rtn = buffer.GetNextData(id);
		CHECK_RTN(rtn);
		ptrWord->mId = id;

		rtn = buffer.GetNextData(appear);
		CHECK_RTN(rtn);
		ptrWord->mAppear = appear;

		mIdToWords[ptrWord->mId] = ptrWord;
		mStringToWords[ptrWord->mStr] = ptrWord;

		if (printLog == FULL_LOG)
			std::clog << "To Load Word: " << ptrWord->mStr << " " << ptrWord->mId << std::endl;

		loadCount++;
		if (printLog != SILENT)
		{
			if ((loadCount & 32767) == 0)
				printf("\r %d Dictionary Load", loadCount);
		}
	}
	if (printLog != SILENT)
		printf("\n Total %d Load\n", loadCount);
	return 0;
}

int Dictionary::Load(const char* const fileName, int printLog)
{
	FILE* file = fopen(fileName, "rb");
	int rtn = 0;
	if (printLog != SILENT)
		printf("\nDictionary Load from %s\n", fileName);
	rtn = Load(file, printLog);
	fclose(file);
	CHECK_RTN(rtn);
	return 0;
}

int Dictionary::Save(FILE* binFile, int printLog)
{
	int rtn = 0;
	int storeCount = 0;
	for (std::map<int, Word*>::iterator ptr = mIdToWords.begin(); ptr != mIdToWords.end(); ptr++)
	{
		Word* ptrWord = ptr->second;
		if (printLog == FULL_LOG)
			std::clog << "To Save Word" << ptrWord->mStr << " " << ptrWord->mId << std::endl;
		Write(binFile, (ptrWord->mStr).c_str());
		Write(binFile, ptrWord->mId);
		Write(binFile, ptrWord->mAppear);

		storeCount++;
		if (printLog != SILENT)
		{
			if ((storeCount & 32767) == 0)
				printf("\r %d Dictionary Save", storeCount);
		}
	}
	if (printLog != SILENT)
		printf("\n Total %d Save\n", storeCount);
	return 0;
}

int Dictionary::Save(const char* const fileName, int printLog)
{
	FILE* file = fopen(fileName, "wb");
	int rtn = 0;
	if (printLog != SILENT)
		printf("\nDictionary Save from %s\n", fileName);
	rtn = Save(file, printLog);
	fclose(file);
	CHECK_RTN(rtn);
	return 0;
}

int Dictionary::AddNewWord(const std::string &tString)
{
	if (mStringToWords.find(tString) == mStringToWords.end())
	{
		Word* ptrWord = new Word;
		ptrWord->mStr = tString;
		if ((int)mIdToWords.size() == 0)
			ptrWord->mId = 1;
		else
			ptrWord->mId = ((--mIdToWords.end())->first) + 1;
		ptrWord->mAppear = 1;
		mStringToWords[tString] = ptrWord;
		mIdToWords[ptrWord->mId] = ptrWord;
	}
	else
	{
		mStringToWords[tString]->mAppear++;
	}
	return 0;
}

int Dictionary::AddNewSentence(const std::vector<std::string> &sentence)
{
	int size = (int)sentence.size();
	int rtn = 0;
	for (int i = 0; i < size; i++)
	{
		rtn = AddNewWord(sentence[i]);
		CHECK_RTN(rtn);
	}
	return 0;
}

int Dictionary::BuildIn(FILE* txtFile, int printLog)
{
	std::vector<std::string> sentence;
	std::string s;
	int Pmid;
	int num;
	int rtn = 0;
	char cLine[MAXLINE + 8];
	int buildCount = 0;
	while (fscanf(txtFile, "%d", &Pmid) != EOF)
	{
		if (printLog == FULL_LOG)
			std::clog << "To Build Pmid" << Pmid << std::endl;
		
		fgets(cLine, MAXLINE, txtFile); //move to next line.

		rtn = AddNewSentence(SplitString(GetLine(cLine, txtFile)));
		CHECK_RTN(rtn);
		rtn = AddNewSentence(SplitString(GetLine(cLine, txtFile)));
		CHECK_RTN(rtn);

		fscanf(txtFile, "%d", &num);
		fgets(cLine, MAXLINE, txtFile); //move to next line.
		
		for (int i = 1; i <= num; i++)
		{
			rtn = AddNewSentence(SplitString(GetLine(cLine, txtFile)));
			//debug
			//if (rtn == 1)
			//	printf("%s\n", cLine);
			CHECK_RTN(rtn);
		}
		buildCount++;
		if (printLog != SILENT)
		{
			if ((buildCount & 32767) == 0)
				printf("\r %d Dictionary Build", buildCount);
		}
	}
	if (printLog != SILENT)
		printf("\n Total %d Build\n", buildCount);
	fclose(txtFile);
	return 0;
}

int Dictionary::BuildIn(const char* const fileName, int printLog)
{
	FILE* file = fopen(fileName, "r");
	int rtn = 0;
	if (printLog != SILENT)
		printf("\nDictionary BuildIn from %s\n", fileName);
	rtn = BuildIn(file, printLog);
	fclose(file);
	CHECK_RTN(rtn);
	return 0;
}

int Dictionary::Build(const char* const filePathHead, const char* filePathTail, const int idStart, const int idEnd, int printLog)
{
	Destroy();
	for (int i = idStart; i <= idEnd; i++)
	{
		int rtn = BuildIn(((std::string)filePathHead + intToString(i) + (std::string)filePathTail).c_str(), printLog);
		CHECK_RTN(rtn);
	}
	return 0;
}

int Dictionary::PrintText(const char* const fileName)
{
	FILE* file = fopen(fileName, "w");
	for (map<int, Word*>::iterator ptr = mIdToWords.begin(); ptr != mIdToWords.end(); ptr++)
	{
		fprintf(file, "%d %s %d\n", ptr->second->mId, (ptr->second->mStr).c_str(), ptr->second->mAppear);
	}
	fclose(file);
	return 0;
}

TokenCitation::TokenCitation()
{
	mPmid = 0;
	mArticleTitle.clear();
	mJournalTitle.clear();
	mAbstract.clear();
}

int TokenCitation::Save(FILE* binFile, int printLog)
{
	if (printLog == FULL_LOG)
		std::clog << "To Binary PMID" << mPmid << std::endl;
	int rtn = 0;
	rtn = Write(binFile, mPmid);
	CHECK_RTN(rtn);
	rtn = Write(binFile, mArticleTitle);
	CHECK_RTN(rtn);
	rtn = Write(binFile, mJournalTitle);
	CHECK_RTN(rtn);
	rtn = Write(binFile, mAbstract);
	CHECK_RTN(rtn);
	return 0;
}


int TokenCitation::Save(const char* const fileName, int printLog)
{
	FILE* file = fopen(fileName, "wb");
	int rtn = 0;
	rtn = Save(file, printLog);
	fclose(file);
	CHECK_RTN(rtn);
	return 0;
}

int TokenCitation::Load(FileBuffer* buffer, int printLog)
{
	if (printLog == FULL_LOG)
		std::clog << "To Binary PMID" << mPmid << std::endl;
	int rtn = 0;
	rtn = buffer->GetNextData(mPmid);
	CHECK_RTN(rtn);
	rtn = Read(buffer, mArticleTitle);
	CHECK_RTN(rtn);
	rtn = Read(buffer, mJournalTitle);
	CHECK_RTN(rtn);
	rtn = Read(buffer, mAbstract);
	CHECK_RTN(rtn);
	return 0;
}

int TokenCitation::CountWords(Tokens &tokens)
{
	int n = (int)tokens.size();
	int size = 0;
	for (int i = 0; i < n; i++)
	{
		size += (int)tokens[i].size();
	}
	return size;
}

int TokenCitation::CountWords()
{
	return CountWords(mArticleTitle) + CountWords(mJournalTitle) + CountWords(mAbstract);
}

TokenCitationSet::TokenCitationSet()
{
	mTokenCitations.clear();
	mDictionary = NULL;
}

TokenCitationSet::TokenCitationSet(const char* const fileName)
{
	mTokenCitations.clear();
	mDictionary = new Dictionary;
	(mDictionary->mStringToWords).clear();
	(mDictionary->mIdToWords).clear();
	mDictionary->Load(fileName);
}

int TokenCitationSet::Destroy()
{
	for (std::map<int, TokenCitation*>::iterator ptr = mTokenCitations.begin(); ptr != mTokenCitations.end(); ptr++)
	{
		delete ptr->second;
	}
	mTokenCitations.clear();
	return 0;
}
TokenCitationSet::~TokenCitationSet()
{
	delete mDictionary;
	Destroy();
}

int TokenCitationSet::LoadDictionary(const char* const fileName, int printLog)
{
	int rtn = 0;
	mDictionary = new Dictionary;
	(mDictionary->mStringToWords).clear();
	(mDictionary->mIdToWords).clear();
	rtn = mDictionary->Load(fileName, printLog);
	CHECK_RTN(rtn);
	return 0;
}

int TokenCitationSet::Load(FILE* binFile, int printLog)
{
	if (mDictionary == NULL)
		return 1;
	FileBuffer* buffer = new FileBuffer(binFile);
	int rtn = 0;
	int loadCount = 0;
	Destroy();
	while (!buffer->Eof())
	{
		TokenCitation* ptrTokenCitation = new TokenCitation;
		rtn = ptrTokenCitation->Load(buffer, printLog);
		CHECK_RTN(rtn);
		mTokenCitations[ptrTokenCitation->mPmid] = ptrTokenCitation;
		++loadCount;
		if (printLog != SILENT)
		{
			if ((loadCount & 32767) == 0)
				printf("\r %d TokenCitations Load", loadCount);
		}
	}
	if (printLog != SILENT)
		printf("\n Total %d Load\n", loadCount);
	delete buffer;
	return 0;
}

int TokenCitationSet::Load(const char* const fileName, int printLog)
{
	if (mDictionary == NULL)
		return 1;
	FILE* file = fopen(fileName, "rb");
	int rtn = 0;
	if (printLog != SILENT)
		printf("\nTokenCitationSet Load from %s\n", fileName);
	rtn = Load(file, printLog);
	fclose(file);
	CHECK_RTN(rtn);
	return 0;
}

int TokenCitationSet::Save(FILE* binFile, int printLog)
{
	if (mDictionary == NULL)
		return 1;
	int storeCount = 0;
	for (map<int, TokenCitation*>::iterator ptr = mTokenCitations.begin(); ptr != mTokenCitations.end(); ptr++)
	{
		int rtn = 0;
		rtn = ptr->second->Save(binFile, printLog);
		storeCount++;
		if (printLog != SILENT)
		{
			if ((storeCount & 32767) == 0)
				printf("\r %d TokenCitations Save", storeCount);
		}
		CHECK_RTN(rtn);
	}
	if (printLog != SILENT)
		printf("\n Total %d Save\n", storeCount);
	return 0;
}

int TokenCitationSet::Save(const char* const fileName, int printLog)
{
	if (mDictionary == NULL)
		return 1;
	FILE* file = fopen(fileName, "wb");
	int rtn = 0;
	if (printLog != SILENT)
		printf("\nTokenCitationSet Save from %s\n", fileName);
	rtn = Save(file, printLog);
	fclose(file);
	CHECK_RTN(rtn);
	return 0;
}

std::vector<int> TokenCitationSet::GetSentence(char* cLine, FILE* txtFile)
{
	std::string string = GetLine(cLine, txtFile);
	std::vector<std::string> vectorString = SplitString(string);
	std::vector<int> vectorInt;
	mDictionary->ParseSentence(vectorString, vectorInt);
	return vectorInt;
}

int TokenCitationSet::BuildIn(FILE* txtFile, int printLog)
{
	if (mDictionary == NULL)
		return 1;
	int pmid;
	int buildCount = 0;
	char cLine[MAXLINE + 8];
	while (fscanf(txtFile, "%d", &pmid) != EOF)
	{
		fgets(cLine, MAXLINE, txtFile);//move to next line.
		if (printLog == FULL_LOG)
			std::clog << "To Build Pmid" << pmid << std::endl;
		TokenCitation* ptrTokenCitation = new TokenCitation;
		int num;
		ptrTokenCitation->mPmid = pmid;
		ptrTokenCitation->mArticleTitle.push_back(GetSentence(cLine, txtFile));
		ptrTokenCitation->mJournalTitle.push_back(GetSentence(cLine, txtFile));
		
		fscanf(txtFile, "%d", &num);
		fgets(cLine, MAXLINE, txtFile); //move to next line.

		for (int i = 0; i < num; i++)
		{
			vector<int> vectorInt = GetSentence(cLine, txtFile);
			if (vectorInt.size() >0)
				ptrTokenCitation->mAbstract.push_back(vectorInt);
		}
		mTokenCitations[pmid] = ptrTokenCitation;
		buildCount++;
		if (printLog != SILENT)
		{
			if ((buildCount & 32767) == 0)
				printf("\r %d TokenCitations Build", buildCount);
		}
	}
	if (printLog != SILENT)
		printf("\n Total %d Build\n", buildCount);
	fclose(txtFile);
	return 0;
}

int TokenCitationSet::BuildIn(const char* const fileName, int printLog)
{
	if (mDictionary == NULL)
		return 1;
	FILE* file = fopen(fileName, "r");
	int rtn = 0;
	if (printLog != SILENT)
		printf("\nTokenCitationSet BuildIn from %s\n", fileName);
	rtn = BuildIn(file, printLog);
	fclose(file);
	CHECK_RTN(rtn);
	return 0;
}

int TokenCitationSet::Build(const char* const filePathHead, const char* filePathTail, const int idStart, const int idEnd, int printLog)
{
	if (mDictionary == NULL)
		return 1;
	Destroy();
	for (int i = idStart; i <= idEnd; i++)
	{
		int rtn = 0;
		rtn = BuildIn(((std::string)filePathHead + intToString(i) + (std::string)filePathTail).c_str(), printLog);
		CHECK_RTN(rtn);
	}
	return 0;
}

int TokenCitationSet::Size()
{
	return (int)mTokenCitations.size();
}

TokenCitation* TokenCitationSet::operator[](int index)
{
	if (mTokenCitations.count(index))
		return mTokenCitations[index];
	else
		return NULL;
}

int TokenCitationSet::PrintTokens(FILE* file, Tokens &tokens)
{
	int n = (int)tokens.size();
	fprintf(file, "%d\n", n);
	for (int i = 0; i < n; i++)
	{
		int m = (int)tokens[i].size();
		for (int j = 0; j < m; j++)
		{
			//fprintf(file, "%s ", (mDictionary->mIdToWords[tokens[i][j]]->mStr).c_str());
			if ( mDictionary->mIdToWords.count(tokens[i][j]) > 0 )
				fprintf(file, "%s ", (mDictionary->mIdToWords[tokens[i][j]]->mStr).c_str());
			else fprintf(file, "UNRECOGNIZED_WORD "), clog << "A unrecognized word shown." << endl;
		}
		fprintf(file, "\n");
	}
	return 0;
}
int TokenCitationSet::PrintText(const char* fileName)
{
	FILE* file = fopen(fileName, "w");
	for (map<int, TokenCitation*>::iterator ptr = mTokenCitations.begin(); ptr != mTokenCitations.end(); ptr++)
	{
		fprintf(file, "%d\n", ptr->second->mPmid);
		PrintTokens(file, ptr->second->mArticleTitle);
		PrintTokens(file, ptr->second->mJournalTitle);
		PrintTokens(file, ptr->second->mAbstract);
		fprintf(file, "\n");
	}
	fclose(file);
	return 0;
}

