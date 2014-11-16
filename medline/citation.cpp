#include "basic.h"
#include "citation.h"
#include "jsoncpp/json.h"
#include <algorithm>
using namespace std;

Citation::Citation(int pmid)
{
	memset(this, 0, sizeof(Citation));
	mPmid = pmid;
}

Citation::~Citation()
{
	DestroyCitation();
}

int Citation::SkipCitation(FileBuffer& buffer, bool bPmid)
{
	int rtn = 0;
	if (bPmid)
	{
		rtn = buffer.SkipNextInt(); //Skip pmid
		CHECK_RTN(rtn);
	}

	rtn = buffer.SkipNextInt();  //mDataCreated
	CHECK_RTN(rtn);
	rtn = buffer.SkipNextInt();  //mPubYear
	CHECK_RTN(rtn);
	rtn = buffer.SkipNextBool(); //mIsDeleted
	CHECK_RTN(rtn);
	rtn = buffer.SkipNextString(); //mJournalTitle
	CHECK_RTN(rtn);
	rtn = buffer.SkipNextString(); //mArticleTitle
	CHECK_RTN(rtn);

	int numberAbstract = 0;
	rtn = buffer.GetNextData(numberAbstract);
	CHECK_RTN(rtn);
	for (int i = 0; i < numberAbstract; i++)
	{
		rtn = buffer.SkipNextString();//mAbstract[i].mLabel
		CHECK_RTN(rtn);
		rtn = buffer.SkipNextString();//mAbstract[i].mText
		CHECK_RTN(rtn);
	}

	int numberAuthor = 0;
	rtn = buffer.GetNextData(numberAuthor);
	CHECK_RTN(rtn);
	for (int i = 0; i < numberAuthor; i++)
	{
		rtn = buffer.SkipNextString();//mAuthorList[i].mLastName
		CHECK_RTN(rtn);
		rtn = buffer.SkipNextString();//mAuthorList[i].mForeName
		CHECK_RTN(rtn);
		rtn = buffer.SkipNextString();//mAuthorList[i].mInitials
		CHECK_RTN(rtn);
		rtn = buffer.SkipNextString();//mAuthorList[i].mSuffix
		CHECK_RTN(rtn);
		rtn = buffer.SkipNextString();//mAuthorList[i].mCollectiveName
		CHECK_RTN(rtn);
	}

	int numberMesh = 0;
	rtn = buffer.GetNextData(numberMesh);
	CHECK_RTN(rtn);
	for (int i = 0; i < numberMesh; i++)
	{
		rtn = buffer.SkipNextBool();//mMeshHeadingList[i].mDescriptorName.mIsMajor
		CHECK_RTN(rtn);
		rtn = buffer.SkipNextString();//mMeshHeadingList[i].mDescriptorName.mText
		CHECK_RTN(rtn);
		int numberQual = 0;
		rtn = buffer.GetNextData(numberQual);//mMeshHeadingList[i].mNumQualifierName
		CHECK_RTN(rtn);
		for (int j = 0; j < numberQual; j++)
		{
			rtn = buffer.SkipNextBool();//mMeshHeadingList[i].mQualifierName[j].mIsMajor
			CHECK_RTN(rtn);
			rtn = buffer.SkipNextString();//mMeshHeadingList[i].mQualifierName[j].mText
			CHECK_RTN(rtn);
		}
	}

	int numberComment = 0;
	rtn = buffer.GetNextData(numberComment);
	CHECK_RTN(rtn);
	for (int i = 0; i < numberComment; i++)
	{
		rtn = buffer.SkipNextInt();//article.mCommentsCorrectionsList[i].mPmid
		CHECK_RTN(rtn);
		rtn = buffer.SkipNextString();//article.mCommentsCorrectionsList[i].mRefSource
		CHECK_RTN(rtn);
		rtn = buffer.SkipNextString();//article.mCommentsCorrectionsList[i].mRefType
		CHECK_RTN(rtn);
	}
	return 0;
}

int Citation::DestroyCitation()
{
	int rtn = 0;
	rtn = SmartFree(mJournalTitle);
	CHECK_RTN(rtn);
	rtn = SmartFree(mArticleTitle);
	CHECK_RTN(rtn);
	for (int i = 0; i < mNumberAbstract; i++)
	{
		rtn = SmartFree(mAbstract[i].mLabel);
		CHECK_RTN(rtn);
		rtn = SmartFree(mAbstract[i].mText);
		CHECK_RTN(rtn);
	}
	rtn = SmartFree(mAbstract);
	CHECK_RTN(rtn);
	for (int i = 0; i < mNumberAuthor; i++)
	{
		rtn = SmartFree(mAuthorList[i].mLastName);
		CHECK_RTN(rtn);
		rtn = SmartFree(mAuthorList[i].mForeName);
		CHECK_RTN(rtn);
		rtn = SmartFree(mAuthorList[i].mInitials);
		CHECK_RTN(rtn);
		rtn = SmartFree(mAuthorList[i].mSuffix);
		CHECK_RTN(rtn);
		rtn = SmartFree(mAuthorList[i].mCollectiveName);
		CHECK_RTN(rtn);
	}
	rtn = SmartFree(mAuthorList);
	CHECK_RTN(rtn);
	for (int i = 0; i < mNumberMesh; i++)
	{
		rtn = SmartFree(mMeshHeadingList[i].mDescriptorName.mText);
		CHECK_RTN(rtn);
		for (int j = 0; j < mMeshHeadingList[i].mNumQualifierName; j++)
		{
			rtn = SmartFree(mMeshHeadingList[i].mQualifierName[j].mText);
			CHECK_RTN(rtn);
		}
		rtn = SmartFree(mMeshHeadingList[i].mQualifierName);
		CHECK_RTN(rtn);
	}
	rtn = SmartFree(mMeshHeadingList);
	CHECK_RTN(rtn);
	for (int i = 0; i < mNumberComment; i++)
	{
		rtn = SmartFree(mCommentsCorrectionsList[i].mRefSource);
		CHECK_RTN(rtn);
		rtn = SmartFree(mCommentsCorrectionsList[i].mRefType);
		CHECK_RTN(rtn);
	}
	rtn = SmartFree(mCommentsCorrectionsList);
	CHECK_RTN(rtn);
	return 0;
}

int Citation::Save(FILE* binFile, int printLog) const
{
	int rtn = 0;
	rtn = Write(binFile, mPmid);
	CHECK_RTN(rtn);
	if (printLog == FULL_LOG)
		std::clog << "To Binary PMID" << mPmid << std::endl;

	rtn = Write(binFile, mDateCreated);
	CHECK_RTN(rtn);
	rtn = Write(binFile, mPubYear);
	CHECK_RTN(rtn);
	rtn = Write(binFile, mIsDeleted);
	CHECK_RTN(rtn);
	rtn = Write(binFile, mJournalTitle);
	CHECK_RTN(rtn);
	rtn = Write(binFile, mArticleTitle);
	CHECK_RTN(rtn);

	rtn = Write(binFile, mNumberAbstract);
	CHECK_RTN(rtn);
	for (int i = 0; i < mNumberAbstract; i++)
	{
		rtn = Write(binFile, mAbstract[i].mLabel);
		CHECK_RTN(rtn);
		rtn = Write(binFile, mAbstract[i].mText);
		CHECK_RTN(rtn);
	}

	rtn = Write(binFile, mNumberAuthor);
	CHECK_RTN(rtn);
	for (int i = 0; i < mNumberAuthor; i++)
	{
		rtn = Write(binFile, mAuthorList[i].mLastName);
		CHECK_RTN(rtn);
		rtn = Write(binFile, mAuthorList[i].mForeName);
		CHECK_RTN(rtn);
		rtn = Write(binFile, mAuthorList[i].mInitials);
		CHECK_RTN(rtn);
		rtn = Write(binFile, mAuthorList[i].mSuffix);
		CHECK_RTN(rtn);
		rtn = Write(binFile, mAuthorList[i].mCollectiveName);
		CHECK_RTN(rtn);
	}

	rtn = Write(binFile, mNumberMesh);
	CHECK_RTN(rtn);
	for (int i = 0; i < mNumberMesh; i++)
	{
		rtn = Write(binFile, mMeshHeadingList[i].mDescriptorName.mIsMajor);
		CHECK_RTN(rtn);
		rtn = Write(binFile, mMeshHeadingList[i].mDescriptorName.mText);
		CHECK_RTN(rtn);
		rtn = Write(binFile, mMeshHeadingList[i].mNumQualifierName);
		CHECK_RTN(rtn);
		for (int j = 0; j < mMeshHeadingList[i].mNumQualifierName; j++)
		{
			rtn = Write(binFile, mMeshHeadingList[i].mQualifierName[j].mIsMajor);
			CHECK_RTN(rtn);
			rtn = Write(binFile, mMeshHeadingList[i].mQualifierName[j].mText);
			CHECK_RTN(rtn);
		}
	}

	rtn = Write(binFile, mNumberComment);
	CHECK_RTN(rtn);
	for (int i = 0; i < mNumberComment; i++)
	{
		rtn = Write(binFile, mCommentsCorrectionsList[i].mPmid);
		CHECK_RTN(rtn);
		rtn = Write(binFile, mCommentsCorrectionsList[i].mRefSource);
		CHECK_RTN(rtn);
		rtn = Write(binFile, mCommentsCorrectionsList[i].mRefType);
		CHECK_RTN(rtn);
	}

	return 0;
}

int Citation::Load(FILE* binFile, int pmid, int printLog)
{
	int rtn = 0;
	if (pmid == 0)
	{
		rtn = Read(binFile, pmid);
		CHECK_RTN(rtn);
	}
	mPmid = pmid;
	if (printLog == FULL_LOG)
		std::clog << "Load Binary PMID" << mPmid << std::endl;

	rtn = Read(binFile, mDateCreated);
	CHECK_RTN(rtn);
	rtn = Read(binFile, mPubYear);
	CHECK_RTN(rtn);
	rtn = Read(binFile, mIsDeleted);
	CHECK_RTN(rtn);
	rtn = Read(binFile, mJournalTitle);
	CHECK_RTN(rtn);
	rtn = Read(binFile, mArticleTitle);
	CHECK_RTN(rtn);

	rtn = Read(binFile, mNumberAbstract);
	CHECK_RTN(rtn);
	mAbstract = Malloc(Abstract, mNumberAbstract);
	for (int i = 0; i < mNumberAbstract; i++)
	{
		rtn = Read(binFile, mAbstract[i].mLabel);
		CHECK_RTN(rtn);
		rtn = Read(binFile, mAbstract[i].mText);
		CHECK_RTN(rtn);
	}

	rtn = Read(binFile, mNumberAuthor);
	CHECK_RTN(rtn);
	mAuthorList = Malloc(Author, mNumberAuthor);
	for (int i = 0; i < mNumberAuthor; i++)
	{
		rtn = Read(binFile, mAuthorList[i].mLastName);
		CHECK_RTN(rtn);
		rtn = Read(binFile, mAuthorList[i].mForeName);
		CHECK_RTN(rtn);
		rtn = Read(binFile, mAuthorList[i].mInitials);
		CHECK_RTN(rtn);
		rtn = Read(binFile, mAuthorList[i].mSuffix);
		CHECK_RTN(rtn);
		rtn = Read(binFile, mAuthorList[i].mCollectiveName);
		CHECK_RTN(rtn);
	}

	rtn = Read(binFile, mNumberMesh);
	CHECK_RTN(rtn);
	mMeshHeadingList = Malloc(Mesh, mNumberMesh);
	for (int i = 0; i < mNumberMesh; i++)
	{
		rtn = Read(binFile, mMeshHeadingList[i].mDescriptorName.mIsMajor);
		CHECK_RTN(rtn);
		rtn = Read(binFile, mMeshHeadingList[i].mDescriptorName.mText);
		CHECK_RTN(rtn);
		rtn = Read(binFile, mMeshHeadingList[i].mNumQualifierName);
		CHECK_RTN(rtn);
		mMeshHeadingList[i].mQualifierName = Malloc(MeshTerm, mMeshHeadingList[i].mNumQualifierName);
		for (int j = 0; j < mMeshHeadingList[i].mNumQualifierName; j++)
		{
			rtn = Read(binFile, mMeshHeadingList[i].mQualifierName[j].mIsMajor);
			CHECK_RTN(rtn);
			rtn = Read(binFile, mMeshHeadingList[i].mQualifierName[j].mText);
			CHECK_RTN(rtn);
		}
	}

	rtn = Read(binFile, mNumberComment);
	CHECK_RTN(rtn);
	mCommentsCorrectionsList = Malloc(CommentsCorrections, mNumberComment);
	for (int i = 0; i < mNumberComment; i++)
	{
		rtn = Read(binFile, mCommentsCorrectionsList[i].mPmid);
		CHECK_RTN(rtn);
		rtn = Read(binFile, mCommentsCorrectionsList[i].mRefSource);
		CHECK_RTN(rtn);
		rtn = Read(binFile, mCommentsCorrectionsList[i].mRefType);
		CHECK_RTN(rtn);
	}
	return 0;
}

int Citation::PrintText(FILE* binFile)
{
	fprintf(binFile, "PMID=\"%d\"\n", mPmid);
	fprintf(binFile, "DateCreated=\"%d\"\n", mDateCreated);
	fprintf(binFile, "PubYear=\"%d\"\n", mPubYear);
	fprintf(binFile, "IsDeleted=\"%d\"\n", (int)mIsDeleted);
	fprintf(binFile, "NumberAuthor=\"%d\"\n", mNumberAuthor);
	fprintf(binFile, "NumberAbstract=\"%d\"\n", mNumberAbstract);
	fprintf(binFile, "NumberMesh=\"%d\"\n", mNumberMesh);
	fprintf(binFile, "NumberComment=\"%d\"\n", mNumberComment);
	fprintf(binFile, "JournalTitle=\"%s\"\n", mJournalTitle);
	fprintf(binFile, "ArticleTitle=\"%s\"\n", mArticleTitle);

	fprintf(binFile, "Abstract\n");
	for (int i = 0; i < mNumberAbstract; ++i)
	{
		if (mAbstract[i].mLabel != NULL)
			fprintf(binFile, "%s: %s\n", mAbstract[i].mLabel, mAbstract[i].mText);
		else
			fprintf(binFile, ": %s\n", mAbstract[i].mText);
	}
	fprintf(binFile, "\n");

	fprintf(binFile, "AuthorList\n");
	for (int i = 0; i < mNumberAuthor; ++i)
	{
		Author* pa = &mAuthorList[i];
		if (pa->mLastName) fprintf(binFile, "LastName=\"%s\"\n", pa->mLastName);
		if (pa->mForeName) fprintf(binFile, "ForeName=\"%s\"\n", pa->mForeName);
		if (pa->mInitials) fprintf(binFile, "Initials=\"%s\"\n", pa->mInitials);
		if (pa->mSuffix) fprintf(binFile, "Suffix=\"%s\"\n", pa->mSuffix);
		if (pa->mCollectiveName) fprintf(binFile, "CollectiveName=\"%s\"\n", pa->mCollectiveName);
	}
	fprintf(binFile, "\n");

	fprintf(binFile, "MeshHeadingList\n");
	for (int i = 0; i < mNumberMesh; ++i)
	{
		fprintf(binFile, "MeshHeading\n");
		fprintf(binFile, "DescriptorName: %s %d\n", mMeshHeadingList[i].mDescriptorName.mText, mMeshHeadingList[i].mDescriptorName.mIsMajor);
		for (int j = 0; j < mMeshHeadingList[i].mNumQualifierName; ++j)
			fprintf(binFile, "QualifierName: %s %d\n", mMeshHeadingList[i].mQualifierName[j].mText, mMeshHeadingList[i].mQualifierName[j].mIsMajor);
	}
	fprintf(binFile, "\n");

	for (int i = 0; i < mNumberComment; ++i)
	{
		if (mCommentsCorrectionsList[i].mPmid)
			fprintf(binFile, "PMID=\"%d\"\n", mCommentsCorrectionsList[i].mPmid);
		fprintf(binFile, "RefType=\"%s\"\n", mCommentsCorrectionsList[i].mRefType);
		fprintf(binFile, "mRefSource=\"%s\"\n", mCommentsCorrectionsList[i].mRefSource);
	}
	return 0;
}

bool Citation::FilterPmid(const int mod, const std::vector<int> &pmidList) const
{
	if (mod == 0)
	{
		for (unsigned int i = 0; i < pmidList.size(); i++)
		if (pmidList[i] == mPmid)
			return true;
		return false;
	}
	set<int> allPmid;
	allPmid.clear();
	allPmid.insert(mPmid);
	for (int i = 0; i < mNumberComment; i++)
	if (mCommentsCorrectionsList[i].mPmid != 0)
		allPmid.insert(mCommentsCorrectionsList[i].mPmid);
	for (unsigned int i = 0; i < pmidList.size(); i++)
	if (allPmid.count(pmidList[i]))
		return true;
	return false;
}

bool Citation::FilterDate(const int startDate, const int endDate) const
{
	return (mDateCreated >= startDate) && (mDateCreated <= endDate);
}

bool Citation::FilterJournalTitle(const int mod, const std::vector<std::string> &wordList) const
{
	return CmpStringList(mJournalTitle, wordList, mod);
}

bool Citation::FilterArticleTitle(const int mod, const std::vector<std::string> &wordList) const
{
	return CmpStringList(mArticleTitle, wordList, mod);
}

bool Citation::FilterAbstract(const int mod, const std::vector<std::string> &wordList) const
{
	string allAbstract;
	allAbstract = "";
	for (int i = 0; i < mNumberAbstract; i++)
		allAbstract += mAbstract[i].mText;
	return CmpStringList(allAbstract, wordList, mod);
}

bool Citation::FilterDescriptorMesh(const int mod, const std::vector<std::string> &wordList) const
{
	vector<string> allDescriptor;
	allDescriptor.clear();
	for (int i = 0; i < mNumberMesh; i++)
		allDescriptor.push_back(mMeshHeadingList[i].mDescriptorName.mText);
	for (unsigned int i = 0; i < wordList.size(); i++)
	{
		if (CmpStringList(wordList[i], allDescriptor, 1))
		{
			if (mod == INCLUDE_ANY)
				return true;
		}
		else if (mod == MATCH || mod == INCLUDE_ALL)
			return false;
	}
	return true;
}

bool Citation::FilterQualifierMesh(const int mod, const std::vector<std::string> &wordList) const
{
	vector<string> allQualifier;
	allQualifier.clear();
	for (int i = 0; i < mNumberMesh; i++)
	for (int j = 0; j < mMeshHeadingList[i].mNumQualifierName; j++)
		allQualifier.push_back(mMeshHeadingList[i].mQualifierName[j].mText);
	for (unsigned int i = 0; i < wordList.size(); i++)
	{
		if (CmpStringList(wordList[i], allQualifier, 1))
		{
			if (mod == INCLUDE_ANY)
				return true;
		}
		else if (mod == MATCH || mod == INCLUDE_ALL)
			return false;
	}
	return true;
}

int Citation::GetMeshVector(std::vector<std::string> &meshVector) const
{
	meshVector.clear();
	for (int i = 0; i < mNumberMesh; i++)
	{
		meshVector.push_back(mMeshHeadingList[i].mDescriptorName.mText);
	}
	sort(meshVector.begin(), meshVector.end());
	return 0;
}

int Citation::GetMajorMeshVector(std::vector<std::string> &majorMeshVector) const
{
	majorMeshVector.clear();
	for (int i = 0; i < mNumberMesh; i++)
	{
		bool isMajor = mMeshHeadingList[i].mDescriptorName.mIsMajor;
		for (int j = 0; j < mMeshHeadingList[i].mNumQualifierName; j++)
		{
			isMajor |= mMeshHeadingList[i].mQualifierName[j].mIsMajor;
		}
		if (isMajor)
			majorMeshVector.push_back(mMeshHeadingList[i].mDescriptorName.mText);
	}
	sort(majorMeshVector.begin(), majorMeshVector.end());
	return 0;
}

int Citation::GetAllAbstract(std::string &allAbstract) const
{
	allAbstract = "";
	for (int i = 0; i < mNumberAbstract; i++)
	{
		if (mAbstract[i].mLabel != NULL)
		{
			allAbstract += mAbstract[i].mLabel;
			allAbstract += ": ";
		}
		allAbstract += mAbstract[i].mText;
		allAbstract += " ";
	}
	return 0;
}

int Citation::SaveTrainJson2(FILE* outFile) const
{
	if (outFile == NULL)
		return -1;

	int rtn = 0;
	Json::Value root;
	if (mDateCreated != 0)
	{
		char strYear[20];
		sprintf(strYear, "%d", mDateCreated / 10000);
		root["year"] = Json::Value(strYear);
	}
	if (mArticleTitle != NULL)
	{
		root["title"] = Json::Value(mArticleTitle);
	}
	if (mPmid != 0)
	{
		char strPmid[20];
		sprintf(strPmid, "%d", mPmid);
		root["pmid"] = Json::Value(strPmid);
	}
	if (mMeshHeadingList != NULL)
	{
		vector<string> meshs;
		rtn = GetMeshVector(meshs);
		CHECK_RTN(rtn);
		for (unsigned i = 0; i < meshs.size(); ++i)
		{
			root["meshMajor"].append(meshs.at(i));
		}
	}
	if (mJournalTitle != NULL)
	{
		root["journal"] = Json::Value(mJournalTitle);
	}
	if (mAbstract != NULL)
	{
		string abst;
		rtn = GetAllAbstract(abst);
		CHECK_RTN(rtn);
		root["abstractText"] = abst;
	}
	Json::FastWriter fastWriter;
	fprintf(outFile, "%s", fastWriter.write(root).c_str());

	return 0;
}

int Citation::SaveTrainJson(FILE* outFile) const
{
	if (outFile == NULL)
		return -1;

	int rtn = 0;
	fprintf(outFile, "{");
	bool bComm = false;
	if (mAbstract != NULL)
	{
		bComm = true;
		fprintf(outFile, "\"abstractText\":\"");
		string abst;
		rtn = GetAllAbstract(abst);
		CHECK_RTN(rtn);
		rtn = PutJsonEscapes(outFile, abst.c_str());
		CHECK_RTN(rtn);
		fprintf(outFile, "\"");
	}

	if (mJournalTitle)
	{
		if (bComm)
			fprintf(outFile, ",");
		bComm = true;
		fprintf(outFile, "\"journal\":\"");
		rtn = PutJsonEscapes(outFile, mJournalTitle);
		CHECK_RTN(rtn);
		fprintf(outFile, "\"");
	}

	if (mMeshHeadingList != NULL)
	{
		if (bComm)
			fprintf(outFile, ",");
		bComm = true;
		fprintf(outFile, "\"meshMajor\":[");
		vector<string> meshs;
		rtn = GetMeshVector(meshs);
		CHECK_RTN(rtn);
		for (unsigned i = 0; i < meshs.size(); ++i)
		{
			if (i>0)
				fprintf(outFile, ",");
			fprintf(outFile, "\"");
			rtn = PutJsonEscapes(outFile, meshs.at(i).c_str());
			CHECK_RTN(rtn);
			fprintf(outFile, "\"");
		}
		fprintf(outFile, "]");
	}

	if (mPmid != 0)
	{
		if (bComm)
			fprintf(outFile, ",");
		bComm = true;
		fprintf(outFile, "\"pmid\":\"%d\"", mPmid);
	}

	if (mArticleTitle != NULL)
	{
		if (bComm)
			fprintf(outFile, ",");
		bComm = true;
		fprintf(outFile, "\"title\":\"");
		rtn = PutJsonEscapes(outFile, mArticleTitle);
		CHECK_RTN(rtn);
		fprintf(outFile, "\"");
	}

	if (mDateCreated != 0)
	{
		if (bComm)
			fprintf(outFile, ",");
		bComm = true;
		fprintf(outFile, "\"year\":\"%d\"", mDateCreated / 10000);
	}
	fprintf(outFile, "}");

	return 0;
}

int Citation::SaveTestJson2(FILE* outFile) const
{
	if (outFile == NULL)
		return -1;

	int rtn = 0;
	Json::Value root;
	if (mAbstract != NULL)
	{
		string abst;
		rtn = GetAllAbstract(abst);
		CHECK_RTN(rtn);
		root["abstract"] = Json::Value(abst);
	}
	if (mArticleTitle != NULL)
	{
		root["title"] = Json::Value(mArticleTitle);
	}
	if (mPmid != 0)
	{
		root["pmid"] = Json::Value(mPmid);
	}
	Json::FastWriter fastWriter;
	fprintf(outFile, "%s", fastWriter.write(root).c_str());

	return 0;
}

int Citation::SaveTestJson(FILE* outFile) const
{
	if (outFile == NULL)
		return -1;

	int rtn = 0;
	fprintf(outFile, "{");
	bool bpmid = false;
	if (mPmid != 0)
	{
		bpmid = true;
		fprintf(outFile, "\"pmid\":%d", mPmid);
	}
	bool btitle = false;
	if (mArticleTitle != NULL)
	{
		btitle = true;
		if (bpmid)
			fprintf(outFile, ",");
		fprintf(outFile, "\"title\":\"");
		rtn = PutJsonEscapes(outFile, mArticleTitle);
		CHECK_RTN(rtn);
		fprintf(outFile, "\"");
	}
	if (mAbstract != NULL)
	{
		string abst;
		rtn = GetAllAbstract(abst);
		CHECK_RTN(rtn);

		if (bpmid || btitle)
			fprintf(outFile, ",");
		fprintf(outFile, "\"abstract\":\"");
		rtn = PutJsonEscapes(outFile, abst.c_str());
		CHECK_RTN(rtn);
		fprintf(outFile, "\"");
	}
	fprintf(outFile, "}");
	return 0;
}

CitationSet::CitationSet()
{
	mCitations.clear();
}

CitationSet::~CitationSet()
{
	for (map<int, Citation*>::iterator it = mCitations.begin(); it != mCitations.end(); it++)
	{
		delete it->second;
	}
}

bool CitationSet::CmpDateCreated(const Citation* p1, const Citation* p2)
{
	return p1->mDateCreated < p2->mDateCreated;
}

Citation* CitationSet::operator[](int index)
{
	if (mCitations.count(index) > 0)
		return mCitations[index];
	return NULL;
}

int CitationSet::Size()
{
	return (int)mCitations.size();
}

int CitationSet::Save(FILE* binFile, int printLog)
{
	int storeCount = 0;

	for (map<int, Citation*>::iterator it = mCitations.begin(); it != mCitations.end(); it++)
	if (it->second != NULL)
	if (!it->second->mIsDeleted)
	{
		it->second->Save(binFile);
		storeCount++;
		if (printLog != SILENT)
		{
			if ((storeCount & 32767) == 0)
				printf("\r %d Citations Stored", storeCount);
		}
	}
	if (printLog != SILENT)
		printf("\n Total %d Stored\n", storeCount);
	return 0;
}

int CitationSet::Save(const char* fileName, int printLog)
{
	FILE* binFile = fopen(fileName, "wb");
	Save(binFile, printLog);
	fclose(binFile);
	return 0;
}

int CitationSet::Load(FILE* binFile, int printLog)
{
	int rtn = 0;
	mCitations.clear();
	int pmid = 0;
	int loadCount = 0;
	FileBuffer buffer(binFile);
	while (!buffer.Eof())
	{
		rtn = buffer.GetNextData(pmid);
		CHECK_RTN(rtn);
		if (printLog == FULL_LOG)
			std::clog << "Load Binary PMID" << pmid << std::endl;

		Citation *article = new Citation(pmid);
		mCitations[pmid] = article;
		rtn = buffer.GetNextData(article->mDateCreated);
		CHECK_RTN(rtn);
		rtn = buffer.GetNextData(article->mPubYear);
		CHECK_RTN(rtn);
		rtn = buffer.GetNextData(article->mIsDeleted);
		CHECK_RTN(rtn);
		rtn = buffer.GetNextData(article->mJournalTitle);
		CHECK_RTN(rtn);
		rtn = buffer.GetNextData(article->mArticleTitle);
		CHECK_RTN(rtn);
		
		rtn = buffer.GetNextData(article->mNumberAbstract);
		CHECK_RTN(rtn);
		article->mAbstract = Malloc(Abstract, article->mNumberAbstract);
		for (int i = 0; i < article->mNumberAbstract; i++)
		{
			rtn = buffer.GetNextData(article->mAbstract[i].mLabel);
			CHECK_RTN(rtn);
			rtn = buffer.GetNextData(article->mAbstract[i].mText);
			CHECK_RTN(rtn);
		}

		rtn = buffer.GetNextData(article->mNumberAuthor);
		CHECK_RTN(rtn);
		article->mAuthorList = Malloc(Author, article->mNumberAuthor);
		for (int i = 0; i < article->mNumberAuthor; i++)
		{
			rtn = buffer.GetNextData(article->mAuthorList[i].mLastName);
			CHECK_RTN(rtn);
			rtn = buffer.GetNextData(article->mAuthorList[i].mForeName);
			CHECK_RTN(rtn);
			rtn = buffer.GetNextData(article->mAuthorList[i].mInitials);
			CHECK_RTN(rtn);
			rtn = buffer.GetNextData(article->mAuthorList[i].mSuffix);
			CHECK_RTN(rtn);
			rtn = buffer.GetNextData(article->mAuthorList[i].mCollectiveName);
			CHECK_RTN(rtn);
		}

		rtn = buffer.GetNextData(article->mNumberMesh);
		CHECK_RTN(rtn);
		article->mMeshHeadingList = Malloc(Mesh, article->mNumberMesh);
		for (int i = 0; i < article->mNumberMesh; i++)
		{
			rtn = buffer.GetNextData(article->mMeshHeadingList[i].mDescriptorName.mIsMajor);
			CHECK_RTN(rtn);
			rtn = buffer.GetNextData(article->mMeshHeadingList[i].mDescriptorName.mText);
			CHECK_RTN(rtn);
			rtn = buffer.GetNextData(article->mMeshHeadingList[i].mNumQualifierName);
			CHECK_RTN(rtn);
			article->mMeshHeadingList[i].mQualifierName = Malloc(MeshTerm, article->mMeshHeadingList[i].mNumQualifierName);
			for (int j = 0; j < article->mMeshHeadingList[i].mNumQualifierName; j++)
			{
				rtn = buffer.GetNextData(article->mMeshHeadingList[i].mQualifierName[j].mIsMajor);
				CHECK_RTN(rtn);
				rtn = buffer.GetNextData(article->mMeshHeadingList[i].mQualifierName[j].mText);
				CHECK_RTN(rtn);
			}
		}

		rtn = buffer.GetNextData(article->mNumberComment);
		CHECK_RTN(rtn);
		article->mCommentsCorrectionsList = Malloc(CommentsCorrections, article->mNumberComment);
		for (int i = 0; i < article->mNumberComment; i++)
		{
			rtn = buffer.GetNextData(article->mCommentsCorrectionsList[i].mPmid);
			CHECK_RTN(rtn);
			rtn = buffer.GetNextData(article->mCommentsCorrectionsList[i].mRefSource);
			CHECK_RTN(rtn);
			rtn = buffer.GetNextData(article->mCommentsCorrectionsList[i].mRefType);
			CHECK_RTN(rtn);
		}
		loadCount++;
		if (printLog != SILENT)
		{
			if ((loadCount & 32767) == 0)
				printf("\r %d Citations Load", loadCount);
		}
	}
	if (printLog != SILENT)
		printf("\n Total %d Load\n", loadCount);
	return 0;
}

int CitationSet::Load(const char* fileName, int printLog)
{
	int rtn = 0;
	FILE* binFile = fopen(fileName, "rb");
	rtn = Load(binFile, printLog);
	CHECK_RTN(rtn);
	fclose(binFile);
	return 0;
}

int CitationSet::Load(FILE* binFile, std::set<int> &pmidSet, int printLog)
{
	int rtn = 0;
	mCitations.clear();
	int pmid = 0;
	int loadCount = 0;
	FileBuffer buffer(binFile);
	while (!buffer.Eof())
	{
		rtn = buffer.GetNextData(pmid);
		CHECK_RTN(rtn);
		if (pmidSet.count(pmid) == 0)
		{
			if(printLog == FULL_LOG)
				clog << "Skip Binary PMID " << pmid << endl;
			rtn = Citation::SkipCitation(buffer,false);
			CHECK_RTN(rtn);
			if (printLog == FULL_LOG)
				clog << pmid << " Skipped" << endl;
			continue;
		}
		if (printLog == FULL_LOG)
			std::clog << "Load Binary PMID" << pmid << std::endl;

		Citation *article = new Citation(pmid);
		mCitations[pmid] = article;
		rtn = buffer.GetNextData(article->mDateCreated);
		CHECK_RTN(rtn);
		rtn = buffer.GetNextData(article->mPubYear);
		CHECK_RTN(rtn);
		rtn = buffer.GetNextData(article->mIsDeleted);
		CHECK_RTN(rtn);
		rtn = buffer.GetNextData(article->mJournalTitle);
		CHECK_RTN(rtn);
		rtn = buffer.GetNextData(article->mArticleTitle);
		CHECK_RTN(rtn);

		rtn = buffer.GetNextData(article->mNumberAbstract);
		CHECK_RTN(rtn);
		article->mAbstract = Malloc(Abstract, article->mNumberAbstract);
		for (int i = 0; i < article->mNumberAbstract; i++)
		{
			rtn = buffer.GetNextData(article->mAbstract[i].mLabel);
			CHECK_RTN(rtn);
			rtn = buffer.GetNextData(article->mAbstract[i].mText);
			CHECK_RTN(rtn);
		}

		rtn = buffer.GetNextData(article->mNumberAuthor);
		CHECK_RTN(rtn);
		article->mAuthorList = Malloc(Author, article->mNumberAuthor);
		for (int i = 0; i < article->mNumberAuthor; i++)
		{
			rtn = buffer.GetNextData(article->mAuthorList[i].mLastName);
			CHECK_RTN(rtn);
			rtn = buffer.GetNextData(article->mAuthorList[i].mForeName);
			CHECK_RTN(rtn);
			rtn = buffer.GetNextData(article->mAuthorList[i].mInitials);
			CHECK_RTN(rtn);
			rtn = buffer.GetNextData(article->mAuthorList[i].mSuffix);
			CHECK_RTN(rtn);
			rtn = buffer.GetNextData(article->mAuthorList[i].mCollectiveName);
			CHECK_RTN(rtn);
		}

		rtn = buffer.GetNextData(article->mNumberMesh);
		CHECK_RTN(rtn);
		article->mMeshHeadingList = Malloc(Mesh, article->mNumberMesh);
		for (int i = 0; i < article->mNumberMesh; i++)
		{
			rtn = buffer.GetNextData(article->mMeshHeadingList[i].mDescriptorName.mIsMajor);
			CHECK_RTN(rtn);
			rtn = buffer.GetNextData(article->mMeshHeadingList[i].mDescriptorName.mText);
			CHECK_RTN(rtn);
			rtn = buffer.GetNextData(article->mMeshHeadingList[i].mNumQualifierName);
			CHECK_RTN(rtn);
			article->mMeshHeadingList[i].mQualifierName = Malloc(MeshTerm, article->mMeshHeadingList[i].mNumQualifierName);
			for (int j = 0; j < article->mMeshHeadingList[i].mNumQualifierName; j++)
			{
				rtn = buffer.GetNextData(article->mMeshHeadingList[i].mQualifierName[j].mIsMajor);
				CHECK_RTN(rtn);
				rtn = buffer.GetNextData(article->mMeshHeadingList[i].mQualifierName[j].mText);
				CHECK_RTN(rtn);
			}
		}

		rtn = buffer.GetNextData(article->mNumberComment);
		CHECK_RTN(rtn);
		article->mCommentsCorrectionsList = Malloc(CommentsCorrections, article->mNumberComment);
		for (int i = 0; i < article->mNumberComment; i++)
		{
			rtn = buffer.GetNextData(article->mCommentsCorrectionsList[i].mPmid);
			CHECK_RTN(rtn);
			rtn = buffer.GetNextData(article->mCommentsCorrectionsList[i].mRefSource);
			CHECK_RTN(rtn);
			rtn = buffer.GetNextData(article->mCommentsCorrectionsList[i].mRefType);
			CHECK_RTN(rtn);
		}
		loadCount++;
		if (printLog != SILENT)
		{
			if ((loadCount & 32767) == 0)
				printf("\r %d Citations Load", loadCount);
		}
	}
	if (printLog != SILENT)
		printf("\n Total %d Load\n", loadCount);
	return 0;
}

int CitationSet::Load(const char* fileName, std::set<int> &pmidSet, int printLog)
{
	int rtn = 0;
	FILE* binFile = fopen(fileName, "rb");
	if (binFile == NULL)
	{
		cerr << "Can't open file " << fileName << endl;
		return -1;
	}
	rtn = Load(binFile, pmidSet, printLog);
	CHECK_RTN(rtn);
	fclose(binFile);
	return 0;
}

int CitationSet::PrintText(FILE* outFile)
{
	int rtn = 0;
	for (map<int, Citation*>::iterator it = mCitations.begin(); it != mCitations.end(); it++)
	{
		rtn = it->second->PrintText(outFile);
		CHECK_RTN(rtn);
	}
	return 0;
}

int CitationSet::PrintText(const char* const fileName)
{
	FILE* outFile = fopen(fileName, "w");
	int rtn = PrintText(outFile);
	CHECK_RTN(rtn);
	fclose(outFile);
	return 0;
}

int CitationSet::SaveTrainJson(const char* const fileName)
{
	if (fileName == NULL)
		return -1;
	FILE *outFile = fopen(fileName, "w");
	int rtn = 0;
	rtn = SaveTrainJson(outFile);
	CHECK_RTN(rtn);
	fclose(outFile);
	return 0;
}

int CitationSet::SaveTrainJson(FILE* outFile)
{
	if (outFile == NULL)
		return -1;

	fprintf(outFile, "{'articles'=[\n");
	int rtn = 0;
	map<int, Citation*>::iterator iter = mCitations.begin();
	while (iter != mCitations.end())
	{
		if (iter->second == NULL)
			return -1;
		rtn = iter->second->SaveTrainJson(outFile);
		CHECK_RTN(rtn);
		++iter;
		if (iter != mCitations.end())
			fprintf(outFile, ",\n");
		else
			break;
	}
	fprintf(outFile, "]}\n");
	return 0;
}

int CitationSet::SaveTestJson(const char* const fileName)
{
	if (fileName == NULL)
		return -1;
	FILE *outFile = fopen(fileName, "w");
	int rtn = 0;
	rtn = SaveTestJson(outFile);
	CHECK_RTN(rtn);
	fclose(outFile);
	return 0;
}

int CitationSet::SaveTestJson(FILE* outFile)
{
	if (outFile == NULL)
		return -1;
	fprintf(outFile, "{\"documents\": [\n");
	int rtn = 0;
	map<int, Citation*>::iterator iter = mCitations.begin();
	while (iter != mCitations.end())
	{
		if (iter->second == NULL)
			return -1;
		rtn = iter->second->SaveTestJson(outFile);
		CHECK_RTN(rtn);
		++iter;
		if (iter != mCitations.end())
			fprintf(outFile, ",\n");
		else
			break;
	}
	fprintf(outFile, "]}\n");
	return 0;
}

int CitationSet::LoadTestJson(const std::string& fileName, int printLog)
{
	if (fileName.empty())
		return -1;

	int rtn = 0;
	mCitations.clear();
	FILE *inFile = fopen(fileName.c_str(), "rb");
	if (inFile == NULL)
	{
		cerr << "json file cannot open!" << endl;
		return -1;
	}

	rtn = LoadTestJson(inFile, printLog);
	CHECK_RTN(rtn);
	fclose(inFile);
	return 0;
}

int CitationSet::LoadTestJson(FILE* inFile, int printLog)
{
	if (inFile == NULL)
		return -1;

	mCitations.clear();
	if (inFile == NULL)
		return 0;

	int rtn = 0;
	FileBuffer buffer(inFile);
	char ch = 0;
	while (!buffer.Eof())
	{
		rtn = buffer.GetNextData(ch);
		CHECK_RTN(rtn);
		if (ch == '{') break;
	}
	if (buffer.Eof())
		return -1;

	Json::Value root;
	Json::Reader reader;
	string line;
	line.reserve(500000);
	while (NextJsonObject(buffer, line) >= 0)
	{
		if (!reader.parse(line, root))
		{
			cerr << "parse test set error!" << endl;
			return -1;
		}
		Citation* ptrCitation = new Citation;
		if (root.isMember("pmid"))
		{
			ptrCitation->mPmid = root["pmid"].asInt();
		}
		else
		{
			cerr << "Error: test json not have pmid" << endl;
			ptrCitation->mPmid = 0;
		}
		if (root.isMember("title"))
		{
			string title = root["title"].asString();
			if (title.size() > 0)
			{
				ptrCitation->mArticleTitle = Malloc(char, title.size() + 1);
				strcpy(ptrCitation->mArticleTitle, title.c_str());
			}
		}
		if (root.isMember("abstract"))
		{
			string abst = root["abstract"].asString();
			if (abst.size() > 0)
			{
				ptrCitation->mNumberAbstract = 1;
				ptrCitation->mAbstract = Malloc(Abstract, 1);
				memset(ptrCitation->mAbstract, 0, sizeof(Abstract));
				ptrCitation->mAbstract[0].mText = Malloc(char, abst.size() + 1);
				strcpy(ptrCitation->mAbstract[0].mText, abst.c_str());
			}
		}
		mCitations[ptrCitation->mPmid] = ptrCitation;
	}
	if (printLog != SILENT)
		clog << "Total load " << mCitations.size() << " test citations" << endl;
	return 0;
}

int CitationSet::AddJournalTitle(const CitationSet& citationSet, int printLog)
{
	int rtn = 0;
	int cnt = 0;
	for (map<int, Citation*>::const_iterator it = citationSet.mCitations.cbegin(); it != citationSet.mCitations.cend(); ++it)
	{
		if (mCitations.count(it->first) > 0 && it->second->mJournalTitle != NULL)
		{
			char* &ptrJournal = mCitations[it->first]->mJournalTitle;
			rtn = SmartFree(ptrJournal);
			CHECK_RTN(rtn);
			size_t len = strlen(it->second->mJournalTitle);
			ptrJournal = Malloc(char, len + 1);
			strcpy(ptrJournal, it->second->mJournalTitle);
			++cnt;
		}
	}
	if (printLog != SILENT)
		clog << cnt << " citations add journal title successful" << endl;
	return 0;
}

int CitationSet::SortByDateCreated(std::vector<Citation*>& vecCita)
{
	vecCita.clear();
	for (map<int, Citation*>::iterator it = mCitations.begin(); it != mCitations.end(); ++it)
	{
		vecCita.push_back(it->second);
	}
	sort(vecCita.begin(), vecCita.end(), CitationSet::CmpDateCreated);
	return 0;
}

Journal::Journal(int id)
{
	mJournalId = id;
	mMedAbbr.clear();
	mJournalTitle.clear();
	mIsoAbbr.clear();
}

Journal::~Journal()
{
	for (size_t i = 0; i < mMedAbbr.size(); ++i)
		SmartFree(mMedAbbr[i]);
	mMedAbbr.clear();
	for (size_t i = 0; i < mJournalTitle.size(); ++i)
		SmartFree(mJournalTitle[i]);
	mJournalTitle.clear();
	for (size_t i = 0; i<mJournalTitle.size(); ++i)
		SmartFree(mIsoAbbr[i]);
	mIsoAbbr.clear();
}

int Journal::Initialize(int id, const string& medAbbr, const string& isoAbbr, const string& journalTitle)
{
	mJournalId = id;
	if (medAbbr.empty())
	{
		cerr << "Error: Journal.mMemAbbr can't be empty" << endl;
		return -1;
	}
	mMedAbbr.resize(1);
	mMedAbbr[0] = Malloc(char, medAbbr.size() + 1);
	strcpy(mMedAbbr[0], medAbbr.c_str());

	if (!isoAbbr.empty())
	{
		mIsoAbbr.resize(1);
		mIsoAbbr[0] = Malloc(char, isoAbbr.size() + 1);
		strcpy(mIsoAbbr[0], isoAbbr.c_str());
	}

	if (!journalTitle.empty())
	{
		mJournalTitle.resize(1);
		mJournalTitle[0] = Malloc(char, journalTitle.size() + 1);
		strcpy(mJournalTitle[0], journalTitle.c_str());
	}
	return 0;
}

int Journal::InitializeJournal(int id, const std::string& journalTitle, const std::string& isoAbbr, const std::string& medAbbr)
{
	mJournalId = id;
	if (journalTitle.empty())
	{
		cerr << "Error: Journal.mJournalTitle can't be empty" << endl;
		return -1;
	}
	mJournalTitle.resize(1);
	mJournalTitle[0] = Malloc(char, journalTitle.size() + 1);
	strcpy(mJournalTitle[0], journalTitle.c_str());

	if (!isoAbbr.empty())
	{
		mIsoAbbr.resize(1);
		mIsoAbbr[0] = Malloc(char, isoAbbr.size() + 1);
		strcpy(mIsoAbbr[0], isoAbbr.c_str());
	}

	if (!medAbbr.empty())
	{
		mMedAbbr.resize(1);
		mMedAbbr[0] = Malloc(char, medAbbr.size() + 1);
		strcpy(mMedAbbr[0], medAbbr.c_str());
	}
	return 0;
}

int Journal::addJournalTitle(const string& journalTitle)
{
	if (journalTitle.empty())
		return 0;
	char* ptr = Malloc(char, journalTitle.size() + 1);
	strcpy(ptr, journalTitle.c_str());
	mJournalTitle.push_back(ptr);
	return 0;
}

int Journal::addIsoAbbr(const string& isoAbbr)
{
	if (isoAbbr.empty())
		return 0;
	char* ptr = Malloc(char, isoAbbr.size() + 1);
	strcpy(ptr, isoAbbr.c_str());
	mIsoAbbr.push_back(ptr);
	return 0;
}

int Journal::addMedAbbr(const string& medAbbr)
{
	if (medAbbr.empty())
		return 0;
	char* ptr = Malloc(char, medAbbr.size() + 1);
	strcpy(ptr, medAbbr.c_str());
	mMedAbbr.push_back(ptr);
	return 0;
}

int Journal::PrintText(FILE* outFile)
{
	if (outFile == NULL)
	{
		cerr << "Error: PrintText() outFile is NULL" << endl;
		return -1;
	}
	fprintf(outFile, "journalId: %d\n", mJournalId);
	for (size_t i = 0; i < mMedAbbr.size(); ++i)
		fprintf(outFile, "medAbbr: %s\n", mMedAbbr[i]);
	for (size_t i = 0; i < mJournalTitle.size(); ++i)
		fprintf(outFile, "journalTitile: %s\n", mJournalTitle[i]);
	for (size_t i = 0; i<mIsoAbbr.size(); ++i)
		fprintf(outFile, "IsoAbbr: %s\n", mIsoAbbr[i]);
	return 0;
}

int Journal::LoadMedAbbr(FILE* inFile)
{
	int rtn = 0;
	rtn = Read(inFile, mJournalId);
	CHECK_RTN(rtn);
	mMedAbbr.resize(1);
	rtn = Read(inFile, mMedAbbr[0]);
	CHECK_RTN(rtn);
	int jourLen = 0;
	rtn = Read(inFile, jourLen);
	CHECK_RTN(rtn);
	mJournalTitle.resize(jourLen);
	for (int i = 0; i < jourLen; ++i)
	{
		rtn = Read(inFile, mJournalTitle[i]);
		CHECK_RTN(rtn);
	}
	int isoLen = 0;
	rtn = Read(inFile, isoLen);
	mIsoAbbr.resize(isoLen);
	for (int i = 0; i < isoLen; ++i)
	{
		rtn = Read(inFile, mIsoAbbr[i]);
		CHECK_RTN(rtn);
	}
	return 0;
}

int Journal::SaveMedAbbr(FILE* outFile)
{
	int rtn = 0;
	rtn = Write(outFile, mJournalId);
	CHECK_RTN(rtn);
	rtn = Write(outFile, mMedAbbr[0]);
	CHECK_RTN(rtn);
	Write(outFile, (int)mJournalTitle.size());
	CHECK_RTN(rtn);
	for (int i = 0; i < (int)mJournalTitle.size(); ++i)
	{
		rtn = Write(outFile, mJournalTitle[i]);
		CHECK_RTN(rtn);
	}
	Write(outFile, (int)mIsoAbbr.size());
	for (int i = 0; i < mIsoAbbr.size(); ++i)
	{
		rtn = Write(outFile, mIsoAbbr[i]);
		CHECK_RTN(rtn);
	}
	return 0;
}

int Journal::LoadJournalTitle(FILE* inFile)
{
	int rtn = 0;
	rtn = Read(inFile, mJournalId);
	CHECK_RTN(rtn);
	mJournalTitle.resize(1);
	rtn = Read(inFile, mJournalTitle[0]);
	CHECK_RTN(rtn);
	int medLen = 0;
	rtn = Read(inFile, medLen);
	CHECK_RTN(rtn);
	mMedAbbr.resize(medLen);
	for (int i = 0; i < medLen; ++i)
	{
		rtn = Read(inFile, mMedAbbr[i]);
		CHECK_RTN(rtn);
	}
	int isoLen = 0;
	rtn = Read(inFile, isoLen);
	mIsoAbbr.resize(isoLen);
	for (int i = 0; i < isoLen; ++i)
	{
		rtn = Read(inFile, mIsoAbbr[i]);
		CHECK_RTN(rtn);
	}
	return 0;
}

int Journal::SaveJournalTitle(FILE* outFile)
{
	int rtn = 0;
	rtn = Write(outFile, mJournalId);
	CHECK_RTN(rtn);
	rtn = Write(outFile, mJournalTitle[0]);
	CHECK_RTN(rtn);
	Write(outFile, (int)mMedAbbr.size());
	CHECK_RTN(rtn);
	for (int i = 0; i < (int)mMedAbbr.size(); ++i)
	{
		rtn = Write(outFile, mMedAbbr[i]);
		CHECK_RTN(rtn);
	}
	Write(outFile, (int)mIsoAbbr.size());
	for (int i = 0; i < mIsoAbbr.size(); ++i)
	{
		rtn = Write(outFile, mIsoAbbr[i]);
		CHECK_RTN(rtn);
	}
	return 0;
}

int JournalSet::SaveJournalMeshFreq(const std::map<int, map<int, double>>& freq, const string& fileName)
{
	FILE *outFile = fopen(fileName.c_str(), "wb");
	if (outFile == NULL)
		return -1;
	int rtn = Write(outFile, (int)freq.size());
	CHECK_RTN(rtn);
	for (map<int, map<int, double>>::const_iterator it = freq.begin(); it != freq.end(); ++it)
	{
		rtn = Write(outFile, it->first);
		CHECK_RTN(rtn);
		rtn = Write(outFile, (int)it->second.size());
		CHECK_RTN(rtn);
		for (map<int, double>::const_iterator it2 = it->second.begin(); it2 != it->second.end(); ++it2)
		{
			rtn = Write(outFile, it2->first);
			CHECK_RTN(rtn);
			rtn = Write(outFile, it2->second);
			CHECK_RTN(rtn);
		}
	}
	fclose(outFile);
	return 0;
}

int JournalSet::LoadJournalMeshFreq(std::map<int, map<int, double>>& freq, const string& fileName)
{
	freq.clear();
	FileBuffer buffer(fileName.c_str());
	int size = 0;
	int rtn = 0;
	rtn = buffer.GetNextData(size);
	CHECK_RTN(rtn);
	for (int i = 0; i < size; ++i)
	{
		int journalId;
		rtn = buffer.GetNextData(journalId);
		CHECK_RTN(rtn);
		map<int, double> &meshFreq = freq[journalId];
		CHECK_RTN(rtn);
		int len = 0;
		rtn = buffer.GetNextData(len);
		CHECK_RTN(rtn);
		for (int j = 0; j < len; ++j)
		{
			int meshId;
			double value;
			rtn = buffer.GetNextData(meshId);
			CHECK_RTN(rtn);
			rtn = buffer.GetNextData(value);
			CHECK_RTN(rtn);
			meshFreq[meshId] = value;
		}
	}
	return 0;
}

JournalSet::JournalSet()
{
	Clear();
}

JournalSet::~JournalSet()
{

}

int JournalSet::Clear()
{
	mJournals.clear();
	mJournalTitle.clear();
	mMedAbbr.clear();
	mIsoAbbr.clear();
	return 0;
}

int JournalSet::LoadRowData(const string medlineJournal, const string testJournal, int printLog)
{
	Clear();
	ifstream fin(medlineJournal);
	if (!fin.is_open())
	{
		cerr << "Can't open file " << medlineJournal << endl;
		return -1;
	}
	int rtn = 0;
	ifstream finTest(testJournal);
	set<string> testMedAbbr;
	string line;
	while (getline(finTest, line))
		testMedAbbr.insert(line.substr(0, line.size() - 1));
	finTest.close();
	if (printLog != SILENT)
		clog << "Load " << testMedAbbr.size() << " test journals" << endl;

	string word;
	while (fin >> word)
	{
		if (word != "JrId:")
			continue;
		int id;
		fin >> id;
		fin >> word;
		if (word != "JournalTitle:")
		{
			cerr << "Error: " << id << " can't find JournalTitle" << endl;
			return -1;
		}
		string line;
		string jourTitle;
		getline(fin, line);
		int i = 0;
		for (i = 0; i < (int)line.size(); ++i)
		if (line[i] != ' ' && line[i] != '\t')
			break;
		jourTitle = line.substr(i);

		fin >> word;
		if (word != "MedAbbr:")
		{
			cerr << "Error: " << id << " can't find MedAbbr" << endl;
			return -1;
		}
		string medAbbr;
		getline(fin, line);
		for (i = 0; i < (int)line.size(); ++i)
		if (line[i] != ' ' && line[i] != '\t')
			break;
		medAbbr = line.substr(i);

		getline(fin, word);
		getline(fin, word);

		fin >> word;
		if (word != "IsoAbbr:")
		{
			cerr << "Error: " << id << " can't find IsoAbbr" << endl;
			return -1;
		}
		string isoAbbr;
		getline(fin, line);
		for (i = 0; i < (int)line.size(); ++i)
		if (line[i] != ' ' && line[i] != '\t')
			break;
		isoAbbr = line.substr(i);

		if (testMedAbbr.count(medAbbr) > 0)
		{
			Journal* ptrJournal = NULL;
			if (mMedAbbr.count(medAbbr) == 0)
			{
				ptrJournal = &mJournals[id];
				mMedAbbr[medAbbr] = ptrJournal;
				rtn = ptrJournal->Initialize(id, medAbbr, isoAbbr, jourTitle);
				CHECK_RTN(rtn);
			}
			else
			{
				ptrJournal = mMedAbbr[medAbbr];
				ptrJournal->addIsoAbbr(isoAbbr);
				ptrJournal->addJournalTitle(jourTitle);
				if (!isoAbbr.empty() && mIsoAbbr.count(isoAbbr) == 0)
					mIsoAbbr[isoAbbr] = ptrJournal;

				if (!jourTitle.empty() && mJournalTitle.count(jourTitle) > 0)
					mJournalTitle[jourTitle] = ptrJournal;
			}
		}
	}

	if (printLog != SILENT)
		clog << "Total load " << mJournals.size() << " journal profiles" << endl;
	fin.close();
	return 0;
}

int JournalSet::LoadRowData(const string medlineJournal, int printLog)
{
	Clear();
	ifstream fin(medlineJournal);
	if (!fin.is_open())
	{
		cerr << "Can't open file " << medlineJournal << endl;
		return -1;
	}
	int rtn = 0;

	string word;
	while (fin >> word)
	{
		if (word != "JrId:")
			continue;
		int id;
		fin >> id;
		fin >> word;
		if (word != "JournalTitle:")
		{
			cerr << "Error: " << id << " can't find JournalTitle" << endl;
			return -1;
		}
		string line;
		string jourTitle;
		getline(fin, line);
		int i = 0;
		for (i = 0; i < (int)line.size(); ++i)
		if (line[i] != ' ' && line[i] != '\t')
			break;
		jourTitle = line.substr(i);

		fin >> word;
		if (word != "MedAbbr:")
		{
			cerr << "Error: " << id << " can't find MedAbbr" << endl;
			return -1;
		}
		string medAbbr;
		getline(fin, line);
		for (i = 0; i < (int)line.size(); ++i)
		if (line[i] != ' ' && line[i] != '\t')
			break;
		medAbbr = line.substr(i);

		getline(fin, word);
		getline(fin, word);

		fin >> word;
		if (word != "IsoAbbr:")
		{
			cerr << "Error: " << id << " can't find IsoAbbr" << endl;
			return -1;
		}
		string isoAbbr;
		getline(fin, line);
		for (i = 0; i < (int)line.size(); ++i)
		if (line[i] != ' ' && line[i] != '\t')
			break;
		isoAbbr = line.substr(i);

		Journal* ptrJournal = NULL;
		if (mJournalTitle.count(jourTitle) == 0)
		{
			ptrJournal = &mJournals[id];
			mJournalTitle[jourTitle] = ptrJournal;
			rtn = ptrJournal->InitializeJournal(id, jourTitle, isoAbbr, medAbbr);
			CHECK_RTN(rtn);
		}
		else
		{
			ptrJournal = mJournalTitle[jourTitle];
			ptrJournal->addIsoAbbr(isoAbbr);
			ptrJournal->addMedAbbr(medAbbr);
			if (!isoAbbr.empty() && mIsoAbbr.count(isoAbbr) == 0)
				mIsoAbbr[isoAbbr] = ptrJournal;

			if (!medAbbr.empty() && mMedAbbr.count(medAbbr) > 0)
				mMedAbbr[medAbbr] = ptrJournal;
		}
	}

	if (printLog != SILENT)
		clog << "Total load " << mJournals.size() << " journal profiles" << endl;
	fin.close();
	return 0;
}

int JournalSet::Load(FILE* inFile, int printLog)
{
	Clear();
	if (inFile == NULL)
	{
		cerr << "Error: inFile is NULL " << endl;
		return -1;
	}
	int rtn = 0;
	int cnt = 0;
	FileBuffer buffer(inFile);
	while (!buffer.Eof())
	{
		int id;
		rtn = buffer.GetNextData(id);
		CHECK_RTN(rtn);
		Journal* ptrJour = &mJournals[id];
		ptrJour->mJournalId = id;
		ptrJour->mMedAbbr.resize(1);
		rtn = buffer.GetNextData(ptrJour->mMedAbbr[0]);
		CHECK_RTN(rtn);

		int jourLen = 0;
		rtn = buffer.GetNextData(jourLen);
		ptrJour->mJournalTitle.resize(jourLen);
		for (int i = 0; i < jourLen; ++i)
		{
			rtn = buffer.GetNextData(ptrJour->mJournalTitle[i]);
			CHECK_RTN(rtn);
		}
		int isoLen = 0;
		rtn = buffer.GetNextData(isoLen);
		ptrJour->mIsoAbbr.resize(isoLen);
		for (int i = 0; i < isoLen; ++i)
		{
			rtn = buffer.GetNextData(ptrJour->mIsoAbbr[i]);
			CHECK_RTN(rtn);
		}

		if (ptrJour->mMedAbbr[0] == NULL)
		{
			cerr << "Error: mMedAbbr can't be empty" << endl;
			return -1;
		}
		mMedAbbr[ptrJour->mMedAbbr[0]] = ptrJour;

		for (int i = 0; i < jourLen; ++i)
		if (mJournalTitle.count(ptrJour->mJournalTitle[i]) == 0)
			mJournalTitle[ptrJour->mJournalTitle[i]] = ptrJour;
		for (int i = 0; i < isoLen; ++i)
		if (mIsoAbbr.count(ptrJour->mIsoAbbr[i]) == 0)
			mIsoAbbr[ptrJour->mIsoAbbr[i]] = ptrJour;
		++cnt;
	}
	if (printLog != SILENT)
		clog << "Total load " << cnt << " journals" << endl;
	return 0;
}

int JournalSet::Load(const std::string fileName, int printLog)
{
	Clear();
	FILE* inFile = fopen(fileName.c_str(), "rb");
	if (inFile == NULL)
	{
		cerr << "Error: can't open file \"" << fileName << "\"" << endl;
		return -1;
	}
	int rtn = 0;
	rtn = Load(inFile, printLog);
	CHECK_RTN(rtn);
	fclose(inFile);
	return 0;
}

int JournalSet::Save(FILE* outFile, int printLog)
{
	if (outFile == NULL)
	{
		cerr << "Error: outFile is NULL" << endl;
		return -1;
	}
	int rtn = 0;
	int cnt = 0;
	for (map<int, Journal>::iterator it = mJournals.begin(); it != mJournals.end(); ++it)
	{
		it->second.SaveMedAbbr(outFile);
		++cnt;
	}
	if (printLog != SILENT)
		clog << "Total save " << cnt << " journals" << endl;
	return 0;
}

int JournalSet::Save(const std::string fileName, int printLog)
{
	FILE* outFile = fopen(fileName.c_str(), "wb");
	if (outFile == NULL)
	{
		cerr << "Error: can't open file \"" << fileName << "\"" << endl;
		return -1;
	}
	int rtn = 0;
	rtn = Save(outFile, printLog);
	CHECK_RTN(rtn);
	fclose(outFile);
	return 0;
}

int JournalSet::LoadJournalSet(FILE* inFile, int printLog)
{
	Clear();
	if (inFile == NULL)
	{
		cerr << "Error: inFile is NULL " << endl;
		return -1;
	}
	int rtn = 0;
	int cnt = 0;
	FileBuffer buffer(inFile);
	while (!buffer.Eof())
	{
		int id;
		rtn = buffer.GetNextData(id);
		CHECK_RTN(rtn);
		Journal* ptrJour = &mJournals[id];
		ptrJour->mJournalId = id;
		ptrJour->mJournalTitle.resize(1);
		rtn = buffer.GetNextData(ptrJour->mJournalTitle[0]);
		CHECK_RTN(rtn);

		int medLen = 0;
		rtn = buffer.GetNextData(medLen);
		ptrJour->mMedAbbr.resize(medLen);
		for (int i = 0; i < medLen; ++i)
		{
			rtn = buffer.GetNextData(ptrJour->mMedAbbr[i]);
			CHECK_RTN(rtn);
		}
		int isoLen = 0;
		rtn = buffer.GetNextData(isoLen);
		ptrJour->mIsoAbbr.resize(isoLen);
		for (int i = 0; i < isoLen; ++i)
		{
			rtn = buffer.GetNextData(ptrJour->mIsoAbbr[i]);
			CHECK_RTN(rtn);
		}

		if (ptrJour->mJournalTitle[0] == NULL)
		{
			cerr << "Error: mJournalTitle can't be empty" << endl;
			return -1;
		}
		mJournalTitle[ptrJour->mJournalTitle[0]] = ptrJour;

		for (int i = 0; i < medLen; ++i)
		if (mMedAbbr.count(ptrJour->mMedAbbr[i]) == 0)
			mMedAbbr[ptrJour->mMedAbbr[i]] = ptrJour;
		for (int i = 0; i < isoLen; ++i)
		if (mIsoAbbr.count(ptrJour->mIsoAbbr[i]) == 0)
			mIsoAbbr[ptrJour->mIsoAbbr[i]] = ptrJour;
		++cnt;
	}
	if (printLog != SILENT)
		clog << "Total load " << cnt << " journals" << endl;
	return 0;
}

int JournalSet::LoadJournalSet(const std::string fileName, int printLog)
{
	Clear();
	FILE* inFile = fopen(fileName.c_str(), "rb");
	if (inFile == NULL)
	{
		cerr << "Error: can't open file \"" << fileName << "\"" << endl;
		return -1;
	}
	int rtn = 0;
	rtn = LoadJournalSet(inFile, printLog);
	CHECK_RTN(rtn);
	fclose(inFile);
	return 0;
}

int JournalSet::SaveJournalSet(FILE* outFile, int printLog)
{
	if (outFile == NULL)
	{
		cerr << "Error: outFile is NULL" << endl;
		return -1;
	}
	int rtn = 0;
	int cnt = 0;
	for (map<int, Journal>::iterator it = mJournals.begin(); it != mJournals.end(); ++it)
	{
		it->second.SaveJournalTitle(outFile);
		++cnt;
	}
	if (printLog != SILENT)
		clog << "Total save " << cnt << " journals" << endl;
	return 0;
}

int JournalSet::SaveJournalSet(const std::string fileName, int printLog)
{
	FILE* outFile = fopen(fileName.c_str(), "wb");
	if (outFile == NULL)
	{
		cerr << "Error: can't open file \"" << fileName << "\"" << endl;
		return -1;
	}
	int rtn = 0;
	rtn = SaveJournalSet(outFile, printLog);
	CHECK_RTN(rtn);
	fclose(outFile);
	return 0;
}

Journal* JournalSet::SearchMedAbbr(const std::string medAbbr)
{
	if (mMedAbbr.count(medAbbr) > 0)
		return mMedAbbr[medAbbr];
	return NULL;
}

Journal* JournalSet::SearchIsoAbbr(const std::string isoAbbr)
{
	if (mIsoAbbr.count(isoAbbr) > 0)
		return mIsoAbbr[isoAbbr];
	return NULL;
}

Journal* JournalSet::SearchJournalTitle(const std::string journalTitle)
{
	if (mJournalTitle.count(journalTitle) > 0)
		return mJournalTitle[journalTitle];
	return NULL;
}

string JournalSet::GetFirstJournalTitle(const std::string& medAbbr)
{
	if (mMedAbbr.count(medAbbr) == 0)
		return string();
	else
		return mMedAbbr[medAbbr]->mJournalTitle[0];
}

Journal* JournalSet::operator[](int index)
{
	if (mJournals.count(index) > 0)
		return &mJournals[index];
	return NULL;
}

Journal* JournalSet::operator[](const std::string& journalTitle)
{
	if (mJournalTitle.count(journalTitle) > 0)
		return mJournalTitle[journalTitle];
	return NULL;
}

int JournalSet::PrintText(const string fileName)
{
	FILE *outFile = fopen(fileName.c_str(), "w");
	for (map<int, Journal>::iterator it = mJournals.begin(); it != mJournals.end(); ++it)
	{
		it->second.PrintText(outFile);
		fprintf(outFile, "\n");
	}
	fclose(outFile);
	return 0;
}

JournalLabelNum::JournalLabelNum()
{
	mJournalTotalLabelNum.clear();
}

JournalLabelNum::~JournalLabelNum()
{

}

int JournalLabelNum::Load(const std::string& binaryFileName, int printLog)
{
	mJournalTotalLabelNum.clear();
	if (binaryFileName.empty())
		return -1;
	int rtn = 0;
	FileBuffer buffer(binaryFileName.c_str());
	int len = 0;
	rtn = buffer.GetNextData(len);
	CHECK_RTN(rtn);
	for (int i = 0; i < len; ++i)
	{
		int journalId;
		rtn = buffer.GetNextData(journalId);
		CHECK_RTN(rtn);
		vector<pair<int, pair<int, int>>>& vec = mJournalTotalLabelNum[journalId];

		int size;
		rtn = buffer.GetNextData(size);
		CHECK_RTN(rtn);
		for (int j = 0; j < size; ++j)
		{
			int index;
			int date, meshNum;
			rtn = buffer.GetNextData(index);
			CHECK_RTN(rtn);
			rtn = buffer.GetNextData(date);
			CHECK_RTN(rtn);
			rtn = buffer.GetNextData(meshNum);
			CHECK_RTN(rtn);
			vec.push_back(make_pair(index, make_pair(date, meshNum)));
		}
	}
	if (printLog != SILENT)
		clog << "total load " << mJournalTotalLabelNum.size() << " journal label num profiles" << endl;
	return 0;
}

int JournalLabelNum::Save(const std::string& binaryFileName, int printLog)
{
	FILE* outFile = fopen(binaryFileName.c_str(), "wb");
	if (outFile == NULL)
		return -1;
	int rtn = 0;
	rtn = Write(outFile, (int)mJournalTotalLabelNum.size());
	CHECK_RTN(rtn);
	for (map<int, vector<pair<int, pair<int, int>>>>::iterator it = mJournalTotalLabelNum.begin(); it != mJournalTotalLabelNum.end(); ++it)
	{
		rtn = Write(outFile, it->first);
		CHECK_RTN(rtn);

		vector<pair<int, pair<int, int>>>& vec = it->second;
		rtn = Write(outFile, (int)vec.size());
		CHECK_RTN(rtn);
		for (int i = 0; i < (int)vec.size(); ++i)
		{
			rtn = Write(outFile, vec[i].first);
			CHECK_RTN(rtn);
			rtn = Write(outFile, vec[i].second.first);
			CHECK_RTN(rtn);
			rtn = Write(outFile, vec[i].second.second);
			CHECK_RTN(rtn);
		}
	}
	fclose(outFile);
	if (printLog != SILENT)
		clog << "total save " << mJournalTotalLabelNum.size() << " journal label num profiles" << endl;
	return 0;
}

int JournalLabelNum::Initialize(const std::string& citationFileName, const std::string& journalSetFileName, int printLog)
{
	const int LOC_BUFFER_SIZE = 5000;
	char buffer[LOC_BUFFER_SIZE];
	int rtn = 0;

	CitationSet citationSet;
	rtn = citationSet.Load(citationFileName.c_str(), printLog);
	CHECK_RTN(rtn);

	JournalSet journalSet;
	rtn = journalSet.LoadJournalSet(journalSetFileName.c_str(), printLog);
	CHECK_RTN(rtn);

	int citationCnt = 0;
	set<string> cantfindJournal;
	for (map<int, Citation*>::iterator it = citationSet.mCitations.begin(); it != citationSet.mCitations.end(); ++it)
	{
		EscapingStrCpy(buffer, it->second->mJournalTitle);
		if (it->second->mJournalTitle == NULL || journalSet.SearchJournalTitle(buffer) == NULL)
		{
			if (buffer)
				cantfindJournal.insert(buffer);
			continue;
		}

		++citationCnt;
		int journalId = journalSet.SearchJournalTitle(buffer)->mJournalId;
		pair<int, int> pil = make_pair(it->second->mDateCreated, it->second->mNumberMesh);
		mJournalTotalLabelNum[journalId].push_back(make_pair(it->first, pil));
	}
	if (printLog != SILENT)
		clog << "Get " << citationCnt << " valid citation" << endl;

	for (map<int, vector<pair<int, std::pair<int, int>>>>::iterator it = mJournalTotalLabelNum.begin(); it != mJournalTotalLabelNum.end(); ++it)
	{
		it->second.push_back(make_pair(0, make_pair(-1, 0)));
		sort(it->second.begin(), it->second.end(), CmpPairBySmallerSecond<int, pair<int, int>>);
		for (int i = 1; i < (int)it->second.size(); ++i)
		{
			it->second[i].second.second += it->second[i - 1].second.second;
		}
	}
	/*FILE* outFile = fopen("cantfindjournal.txt", "w");
	for (set<string>::iterator it = cantfindJournal.begin(); it != cantfindJournal.end(); ++it)
	fprintf(outFile, "%s\n", it->c_str());
	fclose(outFile);*/
	if (printLog != SILENT)
		clog << "total get " << mJournalTotalLabelNum.size() << " journal label num profiles" << endl;

	return 0;
}

int JournalLabelNum::GetAverageLabelNum(int journalId, int dateCreated, double& average)
{
	average = 0.0;
	if (mJournalTotalLabelNum.count(journalId) == 0)
	{
		//clog << "Warning: Can't find journalId " << journalId << " in JournalLabelNum" << endl;
		return 0;
	}
	int year = dateCreated / 10000;
	int upper = year * 10000 + 1300;
	int lower = year * 10000;

	int rtn = 0;
	int totalLabelNum1 = 0, totalLabelNum2 = 0;
	int index1 = 0, index2 = 0;
	rtn = SearchByTime(journalId, lower, totalLabelNum1, index1);
	CHECK_RTN(rtn);
	rtn = SearchByTime(journalId, upper, totalLabelNum2, index2);
	CHECK_RTN(rtn);
	if (index2 - index1 != 0)
		average = (double)(totalLabelNum2 - totalLabelNum1) / (double)(index2 - index1);
	else
		average = 0.0;
	return 0;
}

int JournalLabelNum::GetAverageLabelNum(int journalId, int dateCreated, int latestCitationNum, double& average)
{
	average = 0.0;
	if (mJournalTotalLabelNum.count(journalId) == 0)
	{
		//clog << "Warning: Can't find journalId " << journalId << " in JournalLabelNum" << endl;
		return 0;
	}
	int rtn = 0;
	int totalLabelNum1 = 0, totalLabelNum2 = 0;
	int index1 = 0, index2 = 0;
	rtn = SearchByTime(journalId, dateCreated, totalLabelNum2, index2);
	CHECK_RTN(rtn);

	index1 = index2 - latestCitationNum;
	if (index1 > 0)
		totalLabelNum1 = mJournalTotalLabelNum[journalId][index1].second.second;
	else
	{
		index1 = 0;
		totalLabelNum1 = 0;
	}

	if (index2 - index1 != 0)
		average = (double)(totalLabelNum2 - totalLabelNum1) / (double)(index2 - index1);
	else
		average = 0.0;

	return 0;
}

int JournalLabelNum::GetAverageLabelNum(int journalId, double& average)
{
	average = 0.0;
	if (mJournalTotalLabelNum.count(journalId) == 0)
	{
		//clog << "Warning: Can't find journalId " << journalId << " in JournalLabelNum" << endl;
		return 0;
	}

	int index2 = (int)mJournalTotalLabelNum[journalId].size() - 1;
	int totalLabelNum2 = mJournalTotalLabelNum[journalId][index2].second.second;
	if (index2 > 0)
		average = (double)totalLabelNum2 / (double)index2;
	else
		average = 0.0;
	return 0;
}

int JournalLabelNum::GetStandardDeviation(int journalId, int dateCreated, double& stdDev)
{
	stdDev = 0.0;
	if (mJournalTotalLabelNum.count(journalId) == 0)
	{
		//clog << "Warning: Can't find journalId " << journalId << " in JournalLabelNum" << endl;
		return 0;
	}
	int rtn = 0;
	int year = dateCreated / 10000;
	int upper = year * 10000 + 1300;
	int lower = year * 10000;
	rtn = GetStdDevByTime(journalId, lower, upper, stdDev);
	CHECK_RTN(rtn);
	return 0;
}

int JournalLabelNum::GetStandardDeviation(int journalId, int dateCreated, int latestCitationNum, double& stdDev)
{
	stdDev = 0.0;
	if (mJournalTotalLabelNum.count(journalId) == 0)
	{
		//clog << "Warning: Can't find journalId " << journalId << " in JournalLabelNum" << endl;
		return 0;
	}
	int rtn = 0;
	int endIndex;
	int totalLabelNum = 0;
	rtn = SearchByTime(journalId, dateCreated, totalLabelNum, endIndex);
	CHECK_RTN(rtn);
	int startIndex = endIndex - latestCitationNum + 1;
	if (startIndex < 1)
		startIndex = 1;
	if (startIndex <= endIndex)
	{
		rtn = GetStdDevByIndex(journalId, startIndex, endIndex, stdDev);
		CHECK_RTN(rtn);
	}
	return 0;
}

int JournalLabelNum::GetStandardDeviation(int journalId, double& stdDev)
{
	stdDev = 0.0;
	if (mJournalTotalLabelNum.count(journalId) == 0)
	{
		//clog << "Warning: Can't find journalId " << journalId << " in JournalLabelNum" << endl;
		return 0;
	}
	int rtn = 0;
	rtn = GetStdDevByIndex(journalId, 1, (int)mJournalTotalLabelNum[journalId].size() - 1, stdDev);
	CHECK_RTN(rtn);
	return 0;
}

int JournalLabelNum::GetLatestPmid(int journalId, int dateCreated, int latestCitationNum, std::vector<int>& latestPmids)
{
	latestPmids.clear();
	if (mJournalTotalLabelNum.count(journalId) == 0)
	{
		//clog << "Warning: Can't find journalId " << journalId << " in JournalLabelNum" << endl;
		return 0;
	}
	int rtn = 0;
	int totalLabelNum = 0;
	int index1 = 0, index2 = 0;
	rtn = SearchByTime(journalId, dateCreated, totalLabelNum, index2);
	CHECK_RTN(rtn);

	index1 = index2 - latestCitationNum + 1;
	if (index1 < 1)
		index1 = 1;
	vector<pair<int, pair<int, int>>>& vec = mJournalTotalLabelNum[journalId];
	for (int i = index1; i <= index2; ++i)
		latestPmids.push_back(vec[i].first);
	return 0;
}

int JournalLabelNum::GetStdDevByTime(int journalId, int startTime, int endTime, double& stdDev)
{
	stdDev = 0.0;
	if (mJournalTotalLabelNum.count(journalId) == 0)
	{
		//clog << "Warning: Can't find journalId " << journalId << " in JournalLabelNum" << endl;
		return 0;
	}

	int rtn = 0;
	int index1 = 0, index2 = 0;
	int totalLabelNum1 = 0, totalLabelNum2 = 0;

	rtn = SearchByTime(journalId, startTime - 1, totalLabelNum1, index1);
	CHECK_RTN(rtn);
	rtn = SearchByTime(journalId, endTime, totalLabelNum2, index2);
	CHECK_RTN(rtn);

	rtn = GetStdDevByIndex(journalId, index1 + 1, index2, stdDev);
	CHECK_RTN(rtn);
	return 0;
}

int JournalLabelNum::GetStdDevByIndex(int journalId, int startIndex, int endIndex, double& stdDev)
{
	stdDev = 0.0;
	if (startIndex > endIndex)
		return -1;
	if (mJournalTotalLabelNum.count(journalId) == 0)
	{
		//clog << "Warning: Can't find journalId " << journalId << " in JournalLabelNum" << endl;
		return 0;
	}

	vector<pair<int, pair<int, int>>>& vec = mJournalTotalLabelNum[journalId];

	int totalLabelNum1 = 0, totalLabelNum2 = 0;
	totalLabelNum1 = vec[startIndex - 1].second.second;
	totalLabelNum2 = vec[endIndex].second.second;
	double average;
	int elementNum = endIndex - startIndex + 1;
	if (elementNum != 0)
		average = double(totalLabelNum2 - totalLabelNum1) / double(elementNum);
	else
		average = 0.0;

	double sum = 0.0;
	for (int i = startIndex; i <= endIndex; ++i)
	{
		double labelNum = vec[i].second.second - vec[i - 1].second.second;
		sum += (labelNum - average) * (labelNum - average);
	}
	if (elementNum != 0)
		stdDev = sqrt(sum / double(elementNum));
	else
		stdDev = 0.0;
	return 0;
}

int JournalLabelNum::SearchByTime(int journalId, int dateCreated, int& totalLabelNum, int& index)
{
	totalLabelNum = 0;
	index = 0;
	if (mJournalTotalLabelNum.count(journalId) == 0)
	{
		//clog << "Warning: Can't find journalId " << journalId << " in JournalLabelNum" << endl;
		return 0;
	}

	vector<pair<int, pair<int, int>>>& vec = mJournalTotalLabelNum[journalId];
	int left = 0;
	int right = (int)vec.size() - 1;
	int mid;
	while ((int)vec.size() > right && left <= right)
	{
		mid = (left + right) >> 1;
		if (vec[mid].second.first <= dateCreated)
			left = mid + 1;
		else
			right = mid - 1;
	}
	if (right >= 0 && right < (int)vec.size())
	{
		totalLabelNum = vec[right].second.second;
		index = right;
	}
	return 0;
}

int TrainCitationSet::LoadTrainSet(FILE *pFile)
{
	mTrainSet.clear();
	if (pFile == NULL)
		return 0;

	int rtn = 0;
	char ch = 0;
	FileBuffer buffer(pFile);
	while (!buffer.Eof())
	{
		rtn = buffer.GetNextData(ch);
		CHECK_RTN(rtn);
		if (ch == '{') break;
	}
	if (buffer.Eof())
		return -1;

	Json::Reader reader;
	string line;
	line.reserve(500000);
	while (NextJsonObject(buffer, line) >= 0)
	{
		Json::Value root;
		if (!reader.parse(line, root))
		{
			cerr << "parse train set error!" << endl;
			return -1;
		}

		mTrainSet.push_back(TrainCitation());
		TrainCitation &article = mTrainSet[mTrainSet.size() - 1];
		if (root.isMember("pmid"))
		{
			sscanf(root["pmid"].asString().c_str(), "%d", &article.mPmid);
		}
		else
			article.mPmid = 0;
		if (root.isMember("year"))
		{
			sscanf(root["year"].asString().c_str(), "%d", &article.year);
		}
		else
			article.year = 0;
		if (root.isMember("title"))
		{
			article.mArticle = root["title"].asString();
		}
		if (root.isMember("meshMajor"))
		{
			Json::Value &meshs = root["meshMajor"];
			for (unsigned i = 0; i < meshs.size(); ++i)
			{
				article.mMeshs.push_back(meshs[i].asString());
			}
		}
		if (root.isMember("journal"))
		{
			article.mJournal = root["journal"].asString();
		}
		if (root.isMember("abstractText"))
		{
			article.mAbstract = root["abstractText"].asString();
		}
	}
	return 0;
}

int TrainCitationSet::LoadTrainSet(const char* const fileName)
{
	if (fileName == NULL)
		return -1;

	int rtn = 0;
	mTrainSet.clear();
	FILE *pFile = fopen(fileName, "rb");
	if (pFile == NULL)
	{
		cerr << "json file cannot open!" << endl;
		return -1;
	}

	rtn = LoadTrainSet(pFile);
	CHECK_RTN(rtn);
	fclose(pFile);
	return 0;
}

int TestCitationSet::LoadTestSet(FILE *pFile)
{
	if (pFile == NULL)
		return -1;

	mTestSet.clear();
	if (pFile == NULL)
		return 0;

	int rtn = 0;
	FileBuffer buffer(pFile);
	char ch = 0;
	while (!buffer.Eof())
	{
		rtn = buffer.GetNextData(ch);
		CHECK_RTN(rtn);
		if (ch == '{') break;
	}
	if (buffer.Eof())
		return -1;

	Json::Value root;
	Json::Reader reader;
	string line;
	line.reserve(500000);
	while (NextJsonObject(buffer, line) >= 0)
	{
		if (!reader.parse(line, root))
		{
			cerr << "parse test set error!" << endl;
			return -1;
		}
		mTestSet.push_back(TestCitation());
		TestCitation &article = mTestSet[mTestSet.size() - 1];
		if (root.isMember("pmid"))
		{
			article.mPmid = root["pmid"].asInt();
		}
		else
			article.mPmid = 0;
		if (root.isMember("title"))
		{
			article.mArticle = root["title"].asString();
		}
		if (root.isMember("abstract"))
		{
			article.mAbstract = root["abstract"].asString();
		}
	}
	return 0;
}

int TestCitationSet::LoadTestSet(const char* const fileName)
{
	if (fileName == NULL)
		return -1;

	int rtn = 0;
	mTestSet.clear();
	FILE *pFile = fopen(fileName, "rb");
	if (pFile == NULL)
	{
		cerr << "json file cannot open!" << endl;
		return -1;
	}

	rtn = LoadTestSet(pFile);
	CHECK_RTN(rtn);
	fclose(pFile);
	return 0;
}
